#include <nuttx/config.h>
#include <nuttx/fs/fs.h>

#ifdef CONFIG_DISABLE_POLL
# error Poll operation required.
#endif

#include <cerrno>
#include <cstdint>

#include "bt_types.hpp"
#include "chardev.hpp"
#include "chardev_poll.hpp"
#include "debug.hpp"
#include "device_connection_map.hpp"
#include "io_multiplexer.hpp"
#include "io_multiplexer_flags.hpp"
#include "io_multiplexer_global.hpp"
#include "io_multiplexer_rw.hpp"

/*
 * Bluetooth character devices implementation uses struct file .priv
 * not as an address, but as a bluetooth channel number.
 */

namespace BT
{
namespace CharacterDevices
{
namespace ForeignAddressSpace
{

constexpr void *
dev_index_to_priv(device_index_t di) { return (void *)(uintptr_t)di; }

inline device_index_t
priv_to_dev_index(FAR struct file * filp)
{
	D_ASSERT(filp);
	D_ASSERT(filp->f_inode);
	auto value = (uintptr_t)filp->f_inode->i_private;
	return value <= 7 ? (device_index_t)value : INVALID_DEV_INDEX;
}

inline MultiPlexer &
get_multiplexer(FAR struct file * filp) { return Globals::Multiplexer::get(); }

static int
open(FAR struct file * filp)
{
	const channel_index_t di = priv_to_dev_index(filp);
	if (di == INVALID_DEV_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	bool ok = opened_acquare(mp, di);
	if (ok) { drain(mp.rx, di); }
	return ok ? 0 : -EBUSY;
}

static int
close(FAR struct file * filp)
{
	const channel_index_t di = priv_to_dev_index(filp);
	if (di == INVALID_DEV_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	bool ok = opened_release(mp, di);
	if (ok) { drain(mp.xt, di); }
	return ok ? 0 : -EBADF;
}

static ssize_t
read(FAR struct file * filp, FAR char * buffer, size_t buflen)
{
	const channel_index_t di = priv_to_dev_index(filp);
	if (di == INVALID_DEV_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	if (not is_healthy(mp)) { return -EIO; }

	ssize_t r = 0;
	if (di == 0)
	{
		r = read_service_channel(mp, buffer, buflen);
		dbg("chardev read(0/service) returns %i.\n", r);
	}
	else if (is_connection_established(mp.connection_slots, di))
	{
		channel_index_t ch = channel_index(mp.connection_slots, di);
		r = read_channel_raw(mp, ch, buffer, buflen);
		dbg("chardev read(%u/%u) returns %i.\n", di, ch, r);
	}
	else { dbg("chardev read(%u/-) unconnected.\n", di); }

	if (r == 0) { r = -EAGAIN; }

	dbg_dump("chardev read", mp.rx);
	return r;
}

static ssize_t
write(FAR struct file * filp, FAR const char * buffer, size_t buflen)
{
	const channel_index_t di = priv_to_dev_index(filp);
	if (di == INVALID_DEV_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	if (not is_healthy(mp)) { return -EIO; }

	ssize_t r = 0;
	if (di == 0)
	{
		// FIXME write_service_channel
		r = write_channel_packet(mp, 0, buffer, buflen);
		dbg("chardev write(0/service) returns %i.\n", r);
	}
	else if (is_connection_established(mp.connection_slots, di))
	{
		channel_index_t ch = channel_index(mp.connection_slots, di);
		if (is_channel_xt_ready(mp, ch))
			r = write_channel_packet(mp, ch, buffer, buflen);
		dbg("chardev write(%u/%u) returns %i.\n", di, ch, r);
	}
	else { dbg("chardev write(%u/-) unconnected.\n", di); }

	if (r == 0) { r = -EAGAIN; }

	dbg_dump("chardev write", mp.xt);
	return r;
}

static int
poll(FAR struct file * filp, FAR struct pollfd * p_fd, bool setup_phase)
{
	const channel_index_t di = priv_to_dev_index(filp);
	if (di == INVALID_DEV_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);

	channel_index_t ch;
       	if (di == 0)
		ch = 0;
	else if (is_connection_established(mp.connection_slots, di))
		ch = channel_index(mp.connection_slots, (connection_index_t)di);
	else
	{
		/*
		 * If there is no connection:
		 *
		 * + at set-up phase, it is not an error,
		 *   just let it sleep and receive timeout.
		 *
		 * + at clean-up phase, do nothing and let open(),
		 *   connect and disconnect handlers remove useless
		 *   referencies.
		 */
		return 0;
	}

	int r = 0;
	if (setup_phase)
	{
		if (full(mp.poll_waiters[ch]))
		{
			/*
			 * This should never happen while first open() only
			 * succeeds.
			 */
			r = -ENOMEM;
		}
		else if (is_healthy(mp))
		{
			add(mp.poll_waiters[ch], p_fd);
			poll_notify_channel_unsafe(mp.poll_waiters, ch,
				not empty(mp.rx.channel_buffer[ch]),
				not full(mp.xt.channel_buffer[ch])
			);
		}
		/*
		 * If multiplexer is not healthy let caller get a timeout
		 * to preserve mavlink receiver from infinite active loop.
		 */
	}
	else
	{
		clear(mp.poll_waiters[ch]);
	}
	//dbg("chardev poll %u returns %i.\n", ch, r);
	return r;
}

}
// end of namespace ForeignAddressSpace

static const struct file_operations g_fileops =
{
	.open  = ForeignAddressSpace::open,
	.close = ForeignAddressSpace::close,
	.read  = ForeignAddressSpace::read,
	.write = ForeignAddressSpace::write,
	.seek  = 0,
	.ioctl = 0,
	.poll  = ForeignAddressSpace::poll,
};

static const char * const devname[8] = {
	"/dev/btcmd", "/dev/bt1", "/dev/bt2", "/dev/bt3",
	"/dev/bt4", "/dev/bt5", "/dev/bt6", "/dev/bt7"
};

bool
register_all_devices()
{
	using ForeignAddressSpace::dev_index_to_priv;

	int err;
	device_index_t di = 0;
	do
	{
		err = register_driver(
			devname[di],
			&g_fileops,
			0666,
			dev_index_to_priv(di)
		);
		++di;
	}
	while (err == 0 and di < 7);

	if (err != 0)
	{
		errno = -err;
		perror("register_all_devices");
	}
	return err == 0;;
}

void
unregister_all_devices()
{
	for (device_index_t di = 0; di <= 7; ++di)
		unregister_driver(devname[di]);
}

}
// end of namespace CharacterDevices
}
// end of namespace BT

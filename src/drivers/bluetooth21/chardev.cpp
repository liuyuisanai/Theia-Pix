#include <nuttx/config.h>
#include <nuttx/fs/fs.h>

#ifdef CONFIG_DISABLE_POLL
# error Poll operation required.
#endif

#include <cerrno>
#include <cstdint>

#include "bt_types.hpp"
#include "debug.hpp"
#include "chardev.hpp"
#include "chardev_poll.hpp"
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

constexpr channel_index_t INVALID_CHANNEL_INDEX = 255;

inline void *
channel_index_to_priv(channel_index_t ch) { return (void *)(uintptr_t)ch; }

inline channel_index_t
priv_to_channel_index(FAR struct file * filp)
{
	D_ASSERT(filp);
	D_ASSERT(filp->f_inode);
	auto value = (uintptr_t)filp->f_inode->i_private;
	return value <= 7 ? (channel_index_t)value : INVALID_CHANNEL_INDEX;
}

inline MultiPlexer &
get_multiplexer(FAR struct file * filp) { return Globals::Multiplexer::get(); }

static int
open(FAR struct file * filp)
{
	channel_index_t ch = priv_to_channel_index(filp);
	if (ch == INVALID_CHANNEL_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	bool ok = opened_acquare(mp, ch);
	if (ok) { drain(mp.rx, ch); }
	return ok ? 0 : -EBUSY;
}

static int
close(FAR struct file * filp)
{
	channel_index_t ch = priv_to_channel_index(filp);
	if (ch == INVALID_CHANNEL_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);
	bool ok = opened_release(mp, ch);
	if (ok) { drain(mp.xt, ch); }
	return ok ? 0 : -EBADF;
}

static ssize_t
read(FAR struct file * filp, FAR char * buffer, size_t buflen)
{
	channel_index_t ch = priv_to_channel_index(filp);
	if (ch == INVALID_CHANNEL_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);

	ssize_t r;
	if (ch == 0)
		r = read_service_channel(mp, buffer, buflen);
	else
		r = read_channel_raw(mp, ch, buffer, buflen);

	if (r == 0) { r = -EAGAIN; }

	dbg_dump("chardev read", mp.rx);
	//dbg("chardev read(%u) returns %i.\n", ch, r);
	return r;
}

static ssize_t
write(FAR struct file * filp, FAR const char * buffer, size_t buflen)
{
	channel_index_t ch = priv_to_channel_index(filp);
	if (ch == INVALID_CHANNEL_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);

	// FIXME write_service_channel when ch == 0
	ssize_t r = 0;
	if (is_channel_xt_ready(mp, ch))
		r = write_channel_packet(mp, ch, buffer, buflen);

	if (r == 0) { r = -EAGAIN; }

	dbg_dump("chardev write", mp.xt);
	//dbg("chardev write(%u) returns %i.\n", ch, r);
	return r;
}

static int
poll(FAR struct file * filp, FAR struct pollfd * p_fd, bool setup_phase)
{
	channel_index_t ch = priv_to_channel_index(filp);
	if (ch == INVALID_CHANNEL_INDEX) { return -ENXIO; }

	auto & mp = get_multiplexer(filp);

	int r = 0;
	if (setup_phase)
	{
		if (full(mp.poll_waiters[ch]))
		{
			r = -ENOMEM;
		}
		else
		{
			add(mp.poll_waiters[ch], p_fd);
			poll_notify_channel_unsafe(mp.poll_waiters, ch,
				not empty(mp.rx.channel_buffer[ch]),
				not full(mp.xt.channel_buffer[ch])
			);
		}
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

static const char * const devname[7] = {
	"/dev/bt1", "/dev/bt2", "/dev/bt3", "/dev/bt4",
	"/dev/bt5", "/dev/bt6", "/dev/bt7"
};

bool
register_all_devices()
{
	using ForeignAddressSpace::channel_index_to_priv;

	auto err = register_driver("/dev/btcmd", &g_fileops, 0666,
					channel_index_to_priv(0));

	for (channel_index_t ch = 1; err == 0 and ch <= 7; ++ch)
	{
		err = register_driver(
			devname[ch - 1],
			&g_fileops,
			0666,
			channel_index_to_priv(ch)
		);
	}

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
	for (channel_index_t ch = 1; ch <= 7; ++ch)
		unregister_driver(devname[ch - 1]);

	unregister_driver("/dev/btcmd");
}

}
// end of namespace CharacterDevices
}
// end of namespace BT

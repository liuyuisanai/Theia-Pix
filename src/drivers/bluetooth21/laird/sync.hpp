#pragma once

#include "commands.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

struct SyncState
{
	enum class Level : uint8_t
	{
		UNKNOWN,
		REQUESTED,
		IN_SYNC,
		LOST,
		MODULE_REBOOTED,
	};

	Level level;

	SyncState() : level(Level::UNKNOWN) {}
};

inline void
set_in_sync(SyncState & self) { self.level = SyncState::Level::IN_SYNC; }

inline bool
in_sync(const SyncState & self)
{ return self.level == SyncState::Level::IN_SYNC; }

inline bool
module_rebooted(const SyncState & self)
{ return self.level == SyncState::Level::MODULE_REBOOTED; }

template <typename ServiceIO>
bool
sync_soft_reset(ServiceIO & io, SyncState & self)
{
	self.level = SyncState::Level::REQUESTED;
	log_info("Sync: waiting restart.\n");
	bool ok = soft_reset(io);
	if (ok)
	{
		wait_process_event(io);
		ok = self.level == SyncState::Level::IN_SYNC;
	}
	if (not ok) { self.level = SyncState::Level::LOST; }
	log_info("Sync %s.\n", ok ? "ok" : "failed");
	return ok;
}

bool
handle(SyncState & self, const RESPONSE_EVENT_UNION & p)
{
	switch (get_event_id(p))
	{
	case EVT_STATUS:
		log_info("-> EVT_STATUS: %d disco %d conn %d sec %d.\n"
			, p.evtStatus.status
			, p.evtStatus.discoverable_mode
			, p.evtStatus.connectable_mode
			, p.evtStatus.security_mode
		);

		if (self.level == SyncState::Level::REQUESTED)
			self.level = SyncState::Level::IN_SYNC;
		else if (p.evtStatus.discoverable_mode == 0 and p.evtStatus.connectable_mode == 0)
			self.level = SyncState::Level::MODULE_REBOOTED;
	break;

	case EVT_UNKNOWN_COMMAND:
		log_err("-> EVT_UNKNOWN_COMMAND: command id 0x%02x.\n",
			p.evtUnknownCmd.command);
		log_err("Sync LOST.\n");
		self.level = SyncState::Level::LOST;
	break;

	case EVT_INVALID_PKTSIZE:
		log_err("-> EVT_INVALID_PKTSIZE:"
			" command id 0x%02x actual %u desired %u.\n"
			, p.evtInvPktSize.command
			, p.evtInvPktSize.actualSize
			, p.evtInvPktSize.requiredSize
		);
		log_err("Sync LOST.\n");
		self.level = SyncState::Level::LOST;
	break;

	default: return false;
	}

	return true;
}

template <typename It, typename Size>
bool
handle_unknown_packet(SyncState & self, It first, Size n)
{
	log_err("Desync by unknown packet.\n");
	self.level = SyncState::Level::LOST;
	return true;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

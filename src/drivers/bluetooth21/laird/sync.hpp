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

template <typename ServiceIO>
void
mark_sync(SyncState & self) { self.level = SyncState::Level::IN_SYNC; }

template <typename ServiceIO>
void
in(SyncState & self) { self.level = SyncState::Level::IN_SYNC; }

template <typename ServiceIO>
bool
sync_soft_reset(ServiceIO & io, SyncState & self)
{
	self.level = SyncState::Level::REQUESTED;
	dbg("Sync: waiting restart.\n");
	bool ok = soft_reset(io);
	if (ok)
	{
		wait_process_event(io);
		ok = self.level == SyncState::Level::IN_SYNC;
	}
	if (not ok) { self.level = SyncState::Level::LOST; }
	dbg("Sync %s.\n", ok ? "ok" : "failed");
	return ok;
}

bool
handle(SyncState & self, const RESPONSE_EVENT_UNION & p)
{
	switch (get_event_id(p))
	{
	case EVT_STATUS:
		dbg("-> EVT_STATUS: %d disco %d conn %d sec %d.\n"
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
		dbg("-> EVT_UNKNOWN_COMMAND: command id 0x%02x.\n",
			p.evtUnknownCmd.command);
		dbg("Sync LOST.\n");
		self.level = SyncState::Level::LOST;
	break;

	case EVT_INVALID_PKTSIZE:
		dbg("-> EVT_INVALID_PKTSIZE:"
			" command id 0x%02x actual %u desired %u.\n"
			, p.evtInvPktSize.command
			, p.evtInvPktSize.actualSize
			, p.evtInvPktSize.requiredSize
		);
		dbg("Sync LOST.\n");
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
	dbg("Desync by unknown packet.\n");
	self.level = SyncState::Level::LOST;
	return true;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

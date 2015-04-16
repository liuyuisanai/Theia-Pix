#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../debug.hpp"

#include "../std_array.hpp"

#include "service_defs.hpp"
#include "service_io.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

struct InquiryResult
{
	Address6 addr;
	int8_t rssi;
};

inline void
dbg_result(const InquiryResult & r)
{
	dbg("-> INQUIRY reply " Address6_FMT " %i.\n",
		Address6_FMT_ITEMS(r.addr), r.rssi);
}

inline bool
parse_inquiry_enhanced_data(InquiryResult & r, const uint8_t buf[], size_t n)
{
	bool ok = n >= 12;
	if (ok)
	{
		copy_n(buf + 2, 6, begin(r.addr));
		r.rssi = buf[11];
	}
	return ok;
}

struct InquiryState
{
	static constexpr size_t CAPACITY = 4;
	PODArray<InquiryResult, CAPACITY> first_results;
	uint8_t n_results;

	InquiryState() : n_results(0) {}
};

void
reset(InquiryState & self) { self.n_results = 0; }

template <typename ServiceIO>
bool
inquiry(ServiceIO & io, InquiryState & inq)
{
	reset(inq);

	RESPONSE_INQUIRY_REQ rsp;
	auto cmd = prefill_packet<COMMAND_INQUIRY_REQ, CMD_INQUIRY_REQ>();
	cmd.maxResponses = inq.CAPACITY;
	cmd.timeout_sec = 9;
	cmd.flags = 1 << 7;

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK
		and inq.n_results > 0;

	dbg("<- request INQUIRY_REQ %s %u\n"
		, ok ? "ok": "failed"
		, inq.n_results
	);
	return ok;
}

bool
handle_inquiry_enhanced_data(InquiryState & self, const uint8_t buf[], size_t n)
{
	bool ok = self.n_results < self.CAPACITY;
	if (ok)
	{
		auto & r = self.first_results[self.n_results];
		ok = parse_inquiry_enhanced_data(r, buf, n);
		if (ok)
		{
			dbg_result(r);
			++self.n_results;
		}
	}
	dbg("-> EIR data %s.\n", ok ? "handled" : "dropped.");
	return false;
}

bool
handle(InquiryState & self, const RESPONSE_EVENT_UNION & p)
{
	if (get_event_id(p) == EVT_INQUIRY_RESULT)
	{
		bool ok = self.n_results < self.CAPACITY;
		if (ok)
		{
			auto & r = self.first_results[self.n_results];

			r.addr = p.evtInqResult.bdAddr;
			r.rssi = -128;
			dbg_result(r);

			++self.n_results;
		}
		dbg("-> EVT_INQUIRY_RESULT " Address6_FMT,
			Address6_FMT_ITEMS(p.evtInqResult.bdAddr));
		return ok;
	}
	return false;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

#pragma once

#include <cstdint>
#include <cstdlib>

#include "bt_types.hpp"
#include "debug.hpp"
#include "svc_settings.hpp"

#include "std_array.hpp"

namespace BT
{
namespace Service
{

struct InquiryResult
{
	Address6 addr;
	Class_of_Device cod;
	int8_t rssi;

	friend inline bool
	operator < (const InquiryResult & a, const InquiryResult & b)
	{ return abs(a.rssi) < abs(b.rssi); }
};

inline void
dbg_dump(const InquiryResult & r)
{
	dbg("-> INQUIRY Result " Address6_FMT " CoD 0x%06x RSSI %i.\n",
		Address6_FMT_ITEMS(r.addr), r.cod, r.rssi);
}

struct InquiryState
{
	static constexpr size_t CAPACITY = 4;
	static constexpr auto DRONE_COD = Class_of_Device::DRONE;
	static constexpr unsigned INQUIRY_DURATION_sec = 3;

	PODArray<InquiryResult, CAPACITY> first_results;
	uint8_t n_results;

	InquiryState() : n_results(0) {}
};

void
reset(InquiryState & self) { self.n_results = 0; }

bool
filter(const InquiryState & self, const InquiryResult & r)
{ return r.cod == self.DRONE_COD; }

}
// end of namespace Service
}
// end of namespace BT


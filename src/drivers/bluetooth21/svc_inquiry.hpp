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

bool
closest_is_obvious(const InquiryState & self)
{
	dbg("closest_is_obvious() n_results %u.\n", self.n_results);

	if (self.n_results == 0) { return false; }
	if (self.n_results == 1) { return true; }

	auto first = cbegin(self.first_results);
	auto last = next(first, self.n_results);
	auto p = two_min_elements(first, last);
	dbg("closest_is_obvious() [%u] %i [%u] %i: %i.\n"
		, p.first - first
		, p.first->rssi
		, p.second - first
		, p.second->rssi
		, *p.first < *p.second
	);
	return *p.first < *p.second;
}

Address6 // valid only if closest_is_obvious()
closest_address(const InquiryState & self)
{
	auto first = cbegin(self.first_results);
	auto last = first + self.n_results;
	return min_element(first, last)->addr;
}

}
// end of namespace Service
}
// end of namespace BT


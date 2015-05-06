#pragma once

#include "read_write_log.hpp"

namespace BT
{

template <bool OUT_STDERR, typename Device>
struct TraceHandle;

template <typename Device>
struct TraceHandle<false, Device>
{
	Device & dev;

	TraceHandle(Device & d, const char *, const char *) : dev(d) {}
};

template <typename Device>
struct TraceHandle<true, Device>
{
	DevLog dev;

	TraceHandle(Device & d, const char * prefix_r, const char * prefix_w)
	: dev(fileno(d), 2, prefix_r, prefix_w)
	{}
};

template <bool OUT_STDERR, typename Device>
TraceHandle<OUT_STDERR, Device>
make_trace_handle(Device & dev, const char * prefix_r, const char * prefix_w)
{ return TraceHandle<OUT_STDERR, Device>(dev, prefix_r, prefix_w); }

}
// end of namespace BT

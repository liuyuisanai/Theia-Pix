#pragma once

#include <fcntl.h>
#include <unistd.h>

#include "read_write_log.hpp"
#include "unique_file.hpp"

#define MULTIPLEXER_TRACE_FILE  "/fs/microsd/mp_trace.txt"
#define SERVICE_TRACE_FILE      "/fs/microsd/svc_trace.txt"

namespace BT
{

enum class DebugTraceKind
{
	NO_TRACE,
	STDERR,
	FILE
};

template <DebugTraceKind, typename Device>
struct TraceHandle;

template <typename Device>
struct TraceHandle<DebugTraceKind::NO_TRACE, Device>
{
	Device & dev;

	TraceHandle(const char *, Device & d, const char *, const char *)
	: dev(d)
	{}
};

template <typename Device>
struct TraceHandle<DebugTraceKind::STDERR, Device>
{
	DevLog dev;

	TraceHandle(const char * fn, Device & d, const char * prefix_r, const char * prefix_w)
	: dev(fileno(d), 2, prefix_r, prefix_w)
	{}
};

template <typename Device>
struct TraceHandle<DebugTraceKind::FILE, Device>
{
	unique_file log;
	DevLog dev;

	TraceHandle(const char * fn, Device & d, const char * prefix_r, const char * prefix_w)
	: log(open(fn, O_CREAT | O_WRONLY | O_NOCTTY))
	, dev(fileno(d), (fileno(log) < 0 ? 2 : fileno(log)), prefix_r, prefix_w)
	{
		if (fileno(log) < 0) { return; }

		lseek(fileno(log), 0, SEEK_END);
		const char border[] = "\n\n\n--- --- ---\n\n";
		write(fileno(log), border, strlen(border));
	}
};

template <DebugTraceKind KIND, typename Device>
TraceHandle<KIND, Device>
make_trace_handle(const char * fn, Device & dev, const char * prefix_r, const char * prefix_w)
{ return TraceHandle<KIND, Device>(fn, dev, prefix_r, prefix_w); }

}
// end of namespace BT

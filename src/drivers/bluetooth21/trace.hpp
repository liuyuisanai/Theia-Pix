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

	static inline int
	open_trace_file(const char * fn)
	{
		int fd = open(fn, O_CREAT | O_APPEND | O_WRONLY | O_NOCTTY);
		if (fd < 0)
		{
			dbg("Error opening trace '%s': %i %s.\n"
				, fn
				, errno
				, strerror(errno)
			);
			dbg("Trace goes to stderr.\n");
		}
		else
		{
			dbg("Trace '%s' is opened for append at fd %i.\n",
				fn, fd);
		}
		return fd;
	}

	TraceHandle(const char * fn, Device & d, const char * prefix_r, const char * prefix_w)
	: log(open_trace_file(fn))
	, dev(fileno(d), (fileno(log) < 0 ? 2 : fileno(log)), prefix_r, prefix_w)
	{
		dbg("TraceHandle log fd %i.\n", dev.log);

		if (fileno(log) < 0) { return; }

		const char border[] = "\n\n\n--- --- ---\n\n";
		ssize_t r = write(log, border, strlen(border));
		if (r < 0) { dbg_perror("TraceHandle<FILE> / write border"); }
	}
};

template <DebugTraceKind KIND, typename Device>
TraceHandle<KIND, Device>
make_trace_handle(const char * fn, Device & dev, const char * prefix_r, const char * prefix_w)
{ return TraceHandle<KIND, Device>(fn, dev, prefix_r, prefix_w); }

}
// end of namespace BT

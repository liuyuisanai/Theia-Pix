#pragma once

#include <cstdio>

namespace BT
{
namespace Daemon
{

namespace Multiplexer
{

bool
is_running();

void
start(const char name[]);

void
report_status(FILE *);

void
request_stop();

}
// end of namespace Multiplexer
}
// end of namespace Daemon
}
// end of namespace BT

#include <cassert>

#include "io_multiplexer.hpp"

namespace BT
{
namespace Globals
{
namespace Multiplexer
{

extern MultiPlexer * _mp;

inline MultiPlexer &
get()
{
	assert(_mp != nullptr);
	return *_mp;
}

bool
create();

void
destroy();

}
// end of namespace Multiplexer
}
// end of namespace Globals
}
// end of namespace BT

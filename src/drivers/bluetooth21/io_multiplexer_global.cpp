#include <new>

#include "io_multiplexer_global.hpp"

namespace BT
{
namespace Globals
{
namespace Multiplexer
{

MultiPlexer * _mp = nullptr;

bool
create()
{
	// _mp = new (std::nothrow) MultiPlexer;
	_mp = new MultiPlexer;
	return _mp != nullptr;
}

void
destroy() { if (_mp) delete _mp; }

}
// end of namespace Multiplexer
}
// end of namespace Globals
}
// end of namespace BT

#include "svc_globals.hpp"

namespace BT
{
namespace Globals
{
namespace Service
{

bool pairing_on = false; 

void 
turn_pairing_on(){
    pairing_on = true;
}

void
turn_pairing_off(){
    pairing_on = false;
}

void
toggle_pairing(){
    pairing_on = !pairing_on;
}

bool 
get_pairing_status(){
    return pairing_on;
}

}
// end of namespace Service
}
// end of namespace Globals
}
// end of namespace BT

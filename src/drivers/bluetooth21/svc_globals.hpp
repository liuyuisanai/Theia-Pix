#pragma once

namespace BT
{
namespace Globals
{
namespace Service
{

extern bool pairing_on;

void turn_pairing_on();
void turn_pairing_off();
void toggle_pairing();
bool get_pairing_status();

}
// end of namespace Service
}
// end of namespace Globals
}
// end of namespace BT

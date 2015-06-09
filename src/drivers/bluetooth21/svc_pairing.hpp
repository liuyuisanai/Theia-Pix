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

/* See SSP_ACTION_XXXX #defines in BmHostProtocol 
 * every PAIRING_STAGE definition from 0 to 3  
 * have it's corresponding define there according to its
 * number */
enum PAIRING_STAGE{
    STAGE_COMPLETE = 0,
    STAGE_DISPLAYED,
    STAGE_DISPLAY_YESNO,
    STAGE_ENTER_PASSCODE,
    STAGE_INITIATED,
    STAGE_NONE
};

struct PairingState
{
    PAIRING_STAGE pairing_stage = STAGE_NONE;

    Address6 addr;
    LinkKey16 link_key;

};

inline void
dbg_dump(const PairingState & p){

	dbg("-> Pairing State " Address6_FMT " " LinkKey16_FMT "\n",
		Address6_FMT_ITEMS(p.addr), LinkKey16_FMT_ITEMS(p.link_key));

}

}
// end of namespace Service
}
// end of namespace BT


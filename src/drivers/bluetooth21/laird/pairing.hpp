#pragma once

#include <cstdint>
#include <cstdlib>

#include "../bt_types.hpp"
#include "../debug.hpp"
#include "../svc_pairing.hpp"

#include "service_defs.hpp"
#include "service_io.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{


template <typename ServiceIO> bool
initiate_pairing(ServiceIO & io, const Address6 & addr);

template <typename ServiceIO> bool
send_passcode(ServiceIO & io, const Address6 & addr);


template <typename Device, typename State>
static bool
pair(ServiceBlockingIO< Device, State> & io, const Address6 & addr){

    // If receive and answer functionality is needed for yesno or
    // passcode then function which will return something after every received packet so 
    // that the packet cycle will be here and it will be possible to answer the
    // with yesno/passcode accrdingly. Packet sending should be done in this cycle. 
    // MF
    //
    
    PairingState pr = io.state.pairing;
    
    pr.pairing_stage = STAGE_NONE;

    RESPONSE_PAIR_INITIATE rsp; 
    auto cmd = prefill_packet<COMMAND_PAIR_INITIATE, CMD_PAIR_INITIATE>();
	const auto wait_for = Time::duration_sec(5);

    while (true) {

        switch(pr.pairing_stage)
        {
            case STAGE_NONE:
                if (! initiate_pairing(io, addr))
                    return false;

            break;
            case STAGE_DISPLAY_YESNO: 
                // if (! confirm_deny_pairing())
                //     return false;
            break;
            case STAGE_ENTER_PASSCODE:
                if (! send_passcode(io, addr))
                    return false;
            break;
            default:
            break;
        }

        switch(wait_any_answer(io, get_event_id(cmd), &rsp, sizeof(rsp), wait_for)){
            case NO_ANSWER:
                return false;
            break;

            case ANSWER_RESPONSE:

                if (get_response_status(rsp) == MPSTATUS_OK)
                    return true;
                else
                    return false;

            break;

            default:
            break;
        }

    }  

    return false;
}

template <typename ServiceIO>
bool
initiate_pairing(ServiceIO & io, const Address6 & addr) {

    auto cmd = prefill_packet<COMMAND_PAIR_INITIATE, CMD_PAIR_INITIATE>();
    cmd.timeoutSec = 5;

    copy(begin(addr), end(addr), cmd.bdAddr);

	if (not write_command(io.dev, &cmd, sizeof cmd))
	{
		dbg_perror("initiate_pairing / write_command");
		return false;
	}

    return true;
}


template <typename ServiceIO>
bool
send_passcode(ServiceIO & io, const Address6 & addr) {

    /*
    auto cmd = prefill_packet<COMMAND_PAIR_INITIATE, CMD_PAIR_INITIATE>();
    cmd.timeoutSec = 5;
    cmd.bdAddr = addr;

	if (not write_command(io.dev, &cmd, sizeof cmd))
	{
		dbg_perror("initiate_pairing / write_command");
		return false;
	}
    */

    return true;
}

bool handle(PairingState & self, const RESPONSE_EVENT_UNION & p)
{

    switch (get_event_id(p))
    {
        case EVT_SIMPLE_PAIRING: 

            switch (p.evtSimplePairing.action)
                case SSP_ACTION_COMPLETE:
                    self.pairing_stage = STAGE_COMPLETE;
                    return true;
                break;

                case SSP_ACTION_DISPLAY_ONLY:
                    self.pairing_stage = STAGE_DISPLAYED;
                    return true;
                break;

                case SSP_ACTION_DISPLAY_YESNO:
                    self.pairing_stage = STAGE_DISPLAY_YESNO;
                    return true;
                break;

                case SSP_ACTION_ENTER_PASSCODE:
                    self.pairing_stage = STAGE_ENTER_PASSCODE;
                    return true;
                break;

        break;

        case EVT_LINK_KEY_EX:

            if (self.pairing_stage == STAGE_COMPLETE) {

                self.link_key = p.evtLinkKeyEx.linkKey;

                return true;
            
            } else {

                return false;
            }
        break;

        case EVT_LINK_KEY:

        break;

    }

    return false;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

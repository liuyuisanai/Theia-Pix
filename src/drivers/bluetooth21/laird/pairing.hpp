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
pair(ServiceBlockingIO< Device, State> & io){

    dbg("<- pair() start \n");

    RESPONSE_PAIR_INITIATE rsp; 

    auto cmd = prefill_packet<COMMAND_PAIR_INITIATE, CMD_PAIR_INITIATE>();
    const auto wait_for = Time::duration_sec(5);

    if (io.state.pairing.pairing_initiator) {

        copy(begin(io.state.pairing.addr), end(io.state.pairing.addr), cmd.bdAddr);

        bool ok = send_receive_verbose(io, cmd, rsp, wait_for)
            and get_response_status(rsp) == MPSTATUS_OK;

        dbg("<- initiate PAIRING %s\n"
            , ok ? "ok": "failed"
        );
    }

    else {

        dbg("<- listening for PAIRING packets\n");

        // Just listening and processing packets 
        while (io.state.pairing.pairing_active){
            wait_any_answer(io, get_event_id(cmd), &rsp, sizeof(rsp), wait_for);
        }
    }


    return false;
}


bool 
get_pairing_confirmation(){
    // ToDo: Confirmation button processing here.
    dbg("Drone is confirming pairing.\n");
    return true;
}

bool
is_drone() {


    dbg("Checking if drone.\n");
    uint32_t i = Params::get("MAV_TYPE");
    return i == 2; 

}

template <typename Device>
bool
confirm_deny_pairing(Device & dev, const RESPONSE_EVENT_UNION & p) {

    // Pairing confirmation is implemented and used only on drone side.
    
    if (is_drone()){
        auto cmd = prefill_packet<CONFIRM_SIMPLE_PAIRING, CNF_SIMPLE_PAIRING>();
        cmd.action = SSP_ACTION_DISPLAY_YESNO;
        copy(begin(p.evtSimplePairing.bdAddr), end(p.evtSimplePairing.bdAddr), cmd.bdAddr);

        if (get_pairing_confirmation())
            cmd.actionValue[0] = 1;
        else
            cmd.actionValue[0] = 0;

        if (not write_command(dev, &cmd, sizeof cmd))
        {
            dbg_perror("confirm_deny_pairing / write_command failed\n");
            return false;
        }

        return true;
    } else {

        return false;
    
    }
}


template <typename Device>
bool
send_passcode(Device & dev, const RESPONSE_EVENT_UNION & p) {
    return false;
}

template <typename Device>
bool 
handle(Device & dev, PairingState & self, const RESPONSE_EVENT_UNION & p)
{

    switch (get_event_id(p))
    {
        case EVT_SIMPLE_PAIRING: 

            dbg("->EVT_SIMPLE_PAIRING\n");

            handle_evt_simple_pairing(dev, self, p);

            break;

        case EVT_LINK_KEY_EX:

            dbg("->EVT_LINK_KEY_EX\n");


            dbg("Link key received.\n");

            // self.link_key = p.evtLinkKeyEx.linkKey;
            // Link key received - add to trusted db?
            
            // dbg("Pairing done. Adding trusted key. ");
            // bool ok = add_trusted_key(service_io, pairing_address, svc.pairing.link_key);


            break;

        case EVT_LINK_KEY:

            dbg("->EVT_LINK_KEY\n");

            break;

    }

    return false;
}

template <typename Device>
bool 
handle_evt_simple_pairing(Device & dev, PairingState & self, const RESPONSE_EVENT_UNION & p) {


    switch (p.evtSimplePairing.action) {
        case SSP_ACTION_COMPLETE:
            
            dbg("Pairing complete action.\n");

            return true;

            break;

        case SSP_ACTION_DISPLAY_ONLY:

            dbg("Pairing display only action.\n");

            return true;

            break;

        case SSP_ACTION_DISPLAY_YESNO:

            dbg("Pairing yes/no action.\n");

            confirm_deny_pairing(dev, p);

            return true;

            break;

        case SSP_ACTION_ENTER_PASSCODE:
            send_passcode(dev, p);
            dbg("Enter passcode.\n");
            return true;

            break;

        default:

            dbg("No matched SSP_ACTION !\n");

            return false;
            break;
    }

}


}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

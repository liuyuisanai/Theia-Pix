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

template <typename Device, typename State>
static bool
pair(ServiceBlockingIO< Device, State> & io){

    dbg("<- pair() start \n");

    RESPONSE_PAIR_INITIATE rsp; 

    auto cmd = prefill_packet<COMMAND_PAIR_INITIATE, CMD_PAIR_INITIATE>();
    const auto wait_for = Time::duration_sec(10);

	auto time_limit = Time::now() + wait_for;

    memset(cmd.pinCode, 65, sizeof(cmd.pinCode));
    cmd.timeoutSec = 100;
    cmd.pinCode[16]='\0';

    if (io.state.pairing.pairing_initiator) {

        copy(begin(io.state.pairing.addr), end(io.state.pairing.addr), cmd.bdAddr);

        bool ok = send_receive_verbose(io, cmd, rsp, wait_for)
            and get_response_status(rsp) == MPSTATUS_OK;

        dbg("<- initiate PAIRING %s\n"
            , ok ? "ok": "failed"
        );

        return ok;

    }

    else {

        dbg("<- listening for PAIRING packets\n");

        // Listening and processing packets for wait_for
        while ( time_limit > Time::now() ){
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

    auto cmd = prefill_packet<CONFIRM_SIMPLE_PAIRING, CNF_SIMPLE_PAIRING>();
    cmd.action = SSP_ACTION_ENTER_PASSCODE;
    copy(begin(p.evtSimplePairing.bdAddr), end(p.evtSimplePairing.bdAddr), cmd.bdAddr);


    // The actionValue should be in form 00 0n nn nn - first 3 parts of actionValue must be zeros.
    memset(cmd.actionValue, 63, sizeof(cmd.actionValue));
    cmd.actionValue[0] = 0;
    cmd.actionValue[1] = 0;
    cmd.actionValue[2] = 114;
    cmd.actionValue[3] = 255; 

    if (not write_command(dev, &cmd, sizeof cmd))
    {
        dbg_perror("confirm_deny_pairing / write_command failed\n");
        return false;
    }

    return true;

}

template <typename Device, typename State>
bool 
handle(ServiceBlockingIO< Device, State > & service_io, PairingState & self, const RESPONSE_EVENT_UNION & p)
{

    switch (get_event_id(p))
    {
        case EVT_SIMPLE_PAIRING: 

            dbg("->EVT_SIMPLE_PAIRING\n");

            handle_evt_simple_pairing(service_io, p);

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

            // If EVT_LINK_KEY is received then this device is added to ROLLING trust db
            // Let's move it to PERSISTANT trust db
            
            move_rolling_to_persistant(service_io, p.evtLinkKey.bdAddr);
            service_io.state.pairing.paired_devices++;

            break;

    }

    return false;
}

template <typename Device, typename State>
bool 
handle_evt_simple_pairing(ServiceBlockingIO< Device, State > & service_io,const RESPONSE_EVENT_UNION & p) {

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

            confirm_deny_pairing(service_io.dev, p);

            return true;

            break;

        case SSP_ACTION_ENTER_PASSCODE:


            send_passcode(service_io.dev, p);

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

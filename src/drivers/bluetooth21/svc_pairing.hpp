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

struct PairingState
{

    bool pairing_active;
    bool pairing_initiator;

    int paired_devices;

    // If pairing_initiator is true then addr contains target device address 
	Address6 addr;

};

inline void
dbg_dump(const PairingState & p){


}

}
// end of namespace Service
}
// end of namespace BT


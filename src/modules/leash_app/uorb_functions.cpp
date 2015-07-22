#include "uorb_functions.h"

#include <stdio.h>

void sendAirDogCommnad(enum VEHICLE_CMD command,
                      float param1,
                      float param2,
                      float param3,
                      float param4,
                      double param5,
                      double param6,
                      float param7
)
{
    struct vehicle_command_s vehicle_command;
    static orb_advert_t to_vehicle_command = 0;


    printf("sendAirDogCommnad cmd %d: %.3f %.3f %.3f %.3f %.3f\n", (int)command,
           (double)param1, (double)param2, (double)param3,
           (double)param4, (double)param5);

    vehicle_command.command = command;
    vehicle_command.param1 = param1;
    vehicle_command.param2 = param2;
    vehicle_command.param3 = param3;
    vehicle_command.param4 = param4;
    vehicle_command.param5 = param5;
    vehicle_command.param6 = param6;
    vehicle_command.param7 = param7;

    vehicle_command.target_system = 1;
    vehicle_command.target_component = 50;

    if (to_vehicle_command > 0)
    {
        orb_publish(ORB_ID(vehicle_command), to_vehicle_command, &vehicle_command);
    }
    else
    {
        to_vehicle_command = orb_advertise(ORB_ID(vehicle_command), &vehicle_command);
    }
}

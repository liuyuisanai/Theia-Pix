#pragma once

#include <cstdint>

namespace BT
{
namespace Service
{

/*
 * Generated using http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
 */
enum class Class_of_Device : uint32_t
{
	DEFAULT = 0x001f00,
	DRONE   = 0x08080c, // Capturing, Toy, Robot
	LEASH   = 0x080810, // Capturing, Toy, Controller
};

}
// end of namespace Service
}
// end of namespace BT

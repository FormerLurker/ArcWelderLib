////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Inverse Processor (firmware simulator).  
// Please see the copyright notices in the function definitions
//
// Converts G2/G3(arc) commands back to G0/G1 commands.  Intended to test firmware changes to improve arc support.
// This reduces file size and the number of gcodes per second.
// 
// Based on arc interpolation implementations from:
//    Marlin 1.x (see https://github.com/MarlinFirmware/Marlin/blob/1.0.x/LICENSE for the current license)
//    Marlin 2.x (see https://github.com/MarlinFirmware/Marlin/blob/2.0.x/LICENSE for the current license)
//    Prusa-Firmware (see https://github.com/prusa3d/Prusa-Firmware/blob/MK3/LICENSE for the current license)
//    Smoothieware (see https://github.com/Smoothieware/Smoothieware for the current license)
//    Repetier (see https://github.com/repetier/Repetier-Firmware for the current license)
// 
// Built using the 'Arc Welder: Anti Stutter' library
//
// Copyright(C) 2021 - Brad Hochgesang
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
//
// You can contact the author at the following email address: 
// FormerLurker@pm.me
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>

enum firmware_types { MARLIN_1=0, MARLIN_2=1, REPETIER=2, PRUSA=3, SMOOTHIEWARE=4};
#define NUM_FIRMWARE_TYPES 5
static const std::string firmware_type_names[NUM_FIRMWARE_TYPES] = {
		 "MARLIN_1", "MARLIN_2", "REPETIER", "PRUSA", "SMOOTHIEWARE"
};





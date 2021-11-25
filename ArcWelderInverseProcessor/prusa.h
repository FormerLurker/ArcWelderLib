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
#include "gcode_position.h"
#include "firmware.h"
#include "arc_interpolation_structs.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

class prusa :
  public firmware
{
public:
  enum class prusa_firmware_versions { V3_10_0 = 0, V3_11_0 = 1 };
  typedef unsigned char uint8_t;
  typedef unsigned short uint16_t;
  typedef signed char int8_t;
  enum AxisEnum { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3, X_HEAD = 4, Y_HEAD = 5 };
  prusa(firmware_arguments args);
  virtual std::string interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise) override;
  virtual firmware_arguments get_default_arguments_for_current_version() const override;
  virtual void apply_arguments() override;
private:
  std::string gcodes_;
  /// <summary>
  /// A struct representing the prusa configuration store.  Note:  I didn't add the trailing underscore so this variable name will match the original source algorithm name.
  /// </summary>
  typedef void(prusa::*mc_arc_func)(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder);
  void mc_arc_3_10_0(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder);
  void mc_arc_3_11_0(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder);
  void clamp_to_software_endstops(float* target);
  void plan_buffer_line(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target);
  mc_arc_func mc_arc_;
  prusa::prusa_firmware_versions prusa_version_;
};






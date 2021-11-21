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
#include "firmware.h"

#define NUM_MARLIN_1_FIRMWARE_VERSIONS 1


#define MARLIN_XYZE 5

class marlin_1 :
  public firmware
{
public:
  enum class marlin_1_firmware_versions { V1_1_9_1 = 0 };
  /// <summary>
  /// Types and enums taken from https://github.com/MarlinFirmware/Marlin/blob/1314b31d97bba8cd74c6625c47176d4692f57790/Marlin/enum.h
  /// Note that HANGPRINTER has been disabled to reduce impementation complexity
  /// </summary>
  enum AxisEnum : unsigned char {
    X_AXIS = 0,
    A_AXIS = 0,
    Y_AXIS = 1,
    B_AXIS = 1,
    Z_AXIS = 2,
    C_AXIS = 2,
    E_CART = 3,
//#if ENABLED(HANGPRINTER) // Hangprinter order: A_AXIS, B_AXIS, C_AXIS, D_AXIS, E_AXIS
//    D_AXIS = 3,
//    E_AXIS = 4,
//#else
    E_AXIS = 3,
//#endif
    X_HEAD, Y_HEAD, Z_HEAD,
    ALL_AXES = 0xFE,
    NO_AXIS = 0xFF
  };
  
  marlin_1(firmware_arguments args);
  virtual ~marlin_1();
  virtual std::string interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise) override;
  virtual firmware_arguments get_default_arguments_for_current_version() const override;
  virtual void apply_arguments() override;
private:
  marlin_1_firmware_versions marlin_1_version_;
  std::string gcodes_;
  float* current_position;
  float feedrate_mm_s;
  
  /// <summary>
  /// A struct representing the prusa configuration store.  Note:  I didn't add the trailing underscore so this variable name will match the original source algorithm name.
  /// </summary>
  typedef void(marlin_1::* plan_arc_func)(const float(&cart)[MARLIN_XYZE], // Destination position
    const float(&offset)[2], // Center of rotation relative to current_position
    const bool clockwise      // Clockwise?
  );
  
  void plan_arc_1_1_9_1(const float(&cart)[MARLIN_XYZE], // Destination position
    const float(&offset)[2], // Center of rotation relative to current_position
    const bool clockwise      // Clockwise?
  );
  
  plan_arc_func plan_arc_;
  // Marlin Function Defs
  void NOLESS(uint16_t& x, uint16_t y);
  float MMS_SCALED(float x);
  void COPY(float target[MARLIN_XYZE], const float (&source)[MARLIN_XYZE]);
  bool buffer_line_kinematic(const float (&cart)[MARLIN_XYZE], double fr_mm_s, int active_extruder);
  void clamp_to_software_endstops(const float (&raw)[MARLIN_XYZE]);
};


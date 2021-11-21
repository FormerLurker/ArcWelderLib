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

#define SMOOTHIEWARE_MAX_ROBOT_ACTUATORS 4
struct SmoothiewareGcode {
  SmoothiewareGcode() {
    is_error = false;
    txt_after_ok = "";
  }
  bool is_error;
  std::string txt_after_ok;
  
};
struct SmoothiewareKernel
{
  bool is_halted() {return false;}
};
class smoothieware :
    public firmware
{
public:
  enum class smoothieware_firmware_versions { V2021_06_19 = 0 };
  smoothieware(firmware_arguments args);
  virtual ~smoothieware();
  virtual std::string interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise) override;
  virtual firmware_arguments get_default_arguments_for_current_version() const override;
  virtual void apply_arguments() override;
private:
  smoothieware::smoothieware_firmware_versions smoothieware_version_;
  enum MOTION_MODE_T {
    NONE,
    SEEK, // G0
    LINEAR, // G1
    CW_ARC, // G2
    CCW_ARC // G3
  };
  std::string gcodes_;
  const static int REPETIER_XYZE = 4;
  enum AxisEnum { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3, A_AXIS = 3 };   // A axis is the same as the E axis.
  /// <summary>
  /// A struct representing the prusa configuration store.  Note:  I didn't add the trailing underscore so this variable name will match the original source algorithm name.
  /// </summary>
  typedef bool(smoothieware::* append_arc_func)(SmoothiewareGcode* gcode, const float target[], const float offset[], float radius, bool is_clockwise);

  bool append_arc_2021_06_19(SmoothiewareGcode* gcode, const float target[], const float offset[], float radius, bool is_clockwise);

  append_arc_func append_arc_;

  // Note that trailing underscore are sometimes dropped to keep the ported function as close as possible to the original
  // Repetier Function Defs
  bool append_milestone(const float target[], double rate_mm_s);
  static const int seconds_per_minute = 60;
  static const int k_max_actuators = SMOOTHIEWARE_MAX_ROBOT_ACTUATORS;
  static const int n_motors = SMOOTHIEWARE_MAX_ROBOT_ACTUATORS;
  float machine_position[k_max_actuators];
  static const int plane_axis_0 = AxisEnum::X_AXIS;
  static const int plane_axis_1 = AxisEnum::Y_AXIS;
  static const int plane_axis_2 = AxisEnum::Z_AXIS;
  static const int plane_axis_3 = AxisEnum::E_AXIS;
  SmoothiewareGcode gcode_;
  SmoothiewareKernel *THEKERNEL;
  float feed_rate;
};


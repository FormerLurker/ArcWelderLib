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

#include "repetier.h"
#include "utilities.h"
repetier::repetier(firmware_arguments args) : firmware(args) {

  feedrate = 0;
  apply_arguments();
};

void repetier::apply_arguments()
{
  static const std::vector<std::string> repetier_firmware_version_names{ "1.0.4", "1.0.5" };
  set_versions(repetier_firmware_version_names, "1.0.4");
  repetier_version_ = (repetier::repetier_firmware_versions)version_index_;
  std::vector<std::string> used_arguments;
  switch (repetier_version_)
  {
  case repetier::repetier_firmware_versions::V1_0_5:
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    arc_ = &repetier::arc_1_0_5;
    break;
  default:
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    arc_ = &repetier::arc_1_0_4;
    break;
  }
  
  args_.set_used_arguments(used_arguments);
  
}
firmware_arguments repetier::get_default_arguments_for_current_version() const
{
  // Start off with the current args so they are set up correctly for this firmware type and version
    firmware_arguments default_args = args_;


  // firmware defaults
  default_args.g90_g91_influences_extruder = false;
  // Leave the switch in here in case we want to add more versions.
  switch (repetier_version_)
  {
  case repetier::repetier_firmware_versions::V1_0_5:
    // Active Settings
    default_args.mm_per_arc_segment = 1.0f;
    default_args.n_arc_correction = 25;
    default_args.min_arc_segments = 24;
    break;
  default:
    // Active Settings
    default_args.mm_per_arc_segment = 1.0f;
    default_args.n_arc_correction = 25;
    break;
  }
  return default_args;
}

repetier::~repetier()
{
}

std::string repetier::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
  // Clear the current list of gcodes
  gcodes_.clear();
  // Set up the necessary values to call mc_arc
  float repetier_position[4];
  repetier_position[X_AXIS] = static_cast<float>(position_.x);
  repetier_position[Y_AXIS] = static_cast<float>(position_.y);
  repetier_position[Z_AXIS] = static_cast<float>(position_.z);
  repetier_position[E_AXIS] = static_cast<float>(position_.e);
  float repetier_target[4];
  repetier_target[X_AXIS] = static_cast<float>(target.x);
  repetier_target[Y_AXIS] = static_cast<float>(target.y);
  repetier_target[Z_AXIS] = static_cast<float>(target.z);
  repetier_target[E_AXIS] = static_cast<float>(target.e);
  float repetier_offset[2];
  repetier_offset[0] = static_cast<float>(i);
  repetier_offset[1] = static_cast<float>(j);
  float repetier_radius = static_cast<float>(r);
  if (repetier_radius != 0)
  {
    repetier_radius = utilities::hypotf(repetier_offset[X_AXIS], repetier_offset[Y_AXIS]); // Compute arc radius for mc_arc
  }
  float repetier_f = static_cast<float>(target.f);

  uint8_t repetier_isclockwise = is_clockwise ? 1 : 0;

  feedrate = repetier_f;
  (this->*arc_)(repetier_position, repetier_target, repetier_offset, repetier_radius, repetier_isclockwise);

  return gcodes_;
}

/// <summary>
/// This is intended as a possible replacement for the arc function in 1.0.4 (1.0.5?), which can be found at the following link:
/// https://github.com/repetier/Repetier-Firmware/blob/2bbda51eb6407faf29a09987fd635c86818d32db/src/ArduinoAVR/Repetier/motion.cpp
/// This copyright notice was taken from the link above:
/// 
/// This file is part of Repetier-Firmware.
/// Repetier-Firmware is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// Repetier-Firmware is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// You should have received a copy of the GNU General Public License
/// along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.
/// This firmware is a nearly complete rewrite of the sprinter firmware
/// by kliment (https://github.com/kliment/Sprinter)
/// which based on Tonokip RepRap firmware rewrite based off of Hydra-mmm firmware.
/// Functions in this file are used to communicate using ascii or repetier protocol.
/// </summary>
/// <param name="cart">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="clockwise">Is the motion clockwise or counterclockwise</param>
void repetier::arc_1_0_5(float* position, float* target, float* offset, float radius, uint8_t isclockwise)
{
  //   int acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
    //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
  float center_axis0 = position[X_AXIS] + offset[X_AXIS];
  float center_axis1 = position[Y_AXIS] + offset[Y_AXIS];
  float linear_travel = target[Z_AXIS] - position[Z_AXIS];
  float extruder_travel = (target[E_AXIS] - position[E_AXIS]); // * Printer::invAxisStepsPerMM[E_AXIS];  -- Not sure what this does...
  float r_axis0 = -offset[0]; // Radius vector from center to current location
  float r_axis1 = -offset[1];
  float rt_axis0 = target[0] - center_axis0;
  float rt_axis1 = target[1] - center_axis1;

  float angular_travel;
  // First determine if we have a full circle by checking to see if the start and ending
  // XY position is the same before and after the arc is drawn
    //if (position[X_AXIS] == target[X_AXIS] && position[Y_AXIS] == target[Y_AXIS])
  if (repetier_is_close(position[X_AXIS], target[X_AXIS]) && repetier_is_close(position[Y_AXIS], target[Y_AXIS]))
  {
    // Preserve direction for circles
    angular_travel = isclockwise ? -2.0f * PI_FLOAT : 2.0f * PI_FLOAT;
  }
  else 
  {
    // CCW angle between position and target from circle center. Only one atan2() trig computation required.
    angular_travel = (float)utilities::atan2((double)r_axis0 * rt_axis1 - (double)r_axis1 * rt_axis0, (double)r_axis0 * rt_axis0 + (double)r_axis1 * rt_axis1);
    
    // No need to draw an arc if there is no angular travel
    if (!angular_travel) return;
    
    // Make sure angular travel over 180 degrees goes the other way around.
    if (angular_travel > 0)
    {
      if (isclockwise)
      {
        angular_travel -= 2.0f * PI_FLOAT;
      }
    }
    else if (!isclockwise)
    {
      angular_travel += 2.0f * PI_FLOAT;
    }
  }

  // Determine the number of mm of total travel
  float millimeters_of_travel = abs(angular_travel) * radius;
  if (linear_travel)
  {
    // If we have any Z motion, add this to the total mm of travel.
    millimeters_of_travel = utilities::hypotf(millimeters_of_travel, linear_travel);
  }

  if (millimeters_of_travel < 0.001f) {
    return; // treat as succes because there is nothing to do;
  }

  // The speed restrictions will be based on some new parameters.  Removing the hard coded values
  // Increase segment size if printing faster then computation speed allows
  //uint16_t segments = (feedrate > 60.0f ? floor(millimeters_of_travel / min(static_cast<float>(args_.mm_per_arc_segment), feedrate * 0.01666f * static_cast<float>(args_.mm_per_arc_segment))) : floor(millimeters_of_travel / static_cast<float>(args_.mm_per_arc_segment)));
  
  // Use ceil here since the final segment could be nearly 2x as long as the rest.
  // Example, assume millimeters_of_travel = 1.999.  In this case segments will be 1,
  // the interpolation loop will be skipped, and the segment drawn will be of length
  // 1.999, which is not great.
  uint16_t segments = (uint16_t)utilities::ceil(millimeters_of_travel / (float)args_.mm_per_arc_segment);
  
  if (segments == 0)
    segments = 1;
  /*
    // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
    // by a number of discrete segments. The inverse feed_rate should be correct for the sum of
    // all segments.
    if (invert_feed_rate) { feed_rate *= segments; }
  */
  float theta_per_segment = angular_travel / segments;
  float linear_per_segment = linear_travel/segments;
  float extruder_per_segment = extruder_travel / segments;

  /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
     and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
         r_T = [cos(phi) -sin(phi);
                sin(phi)  cos(phi] * r ;
     For arc generation, the center of the circle is the axis of rotation and the radius vector is
     defined from the circle center to the initial position. Each line segment is formed by successive
     vector rotations. This requires only two cos() and sin() computations to form the rotation
     matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
     all double numbers are single precision on the Arduino. (True double precision will not have
     round off issues for CNC applications.) Single precision error can accumulate to be greater than
     tool precision in some cases. Therefore, arc path correction is implemented.
     Small angle approximation may be used to reduce computation overhead further. This approximation
     holds for everything, but very small circles and large mm_per_arc_segment values. In other words,
     theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
     to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
     numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
     issue for CNC machines with the single precision Arduino calculations.
     This approximation also allows mc_arc to immediately insert a line segment into the planner
     without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
     a correction, the planner should have caught up to the lag caused by the initial mc_arc overhead.
     This is important when there are successive arc motions.
  */
  // Vector rotation matrix values
  // Use two terms for better accuracy
  float sq_theta_per_segment = theta_per_segment * theta_per_segment;
  float cos_T = 1 - 0.5f * sq_theta_per_segment; // Small angle approximation
  float sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6;
  
  float arc_target[4];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count = 0;

  // Initialize the linear axis
  //arc_target[axis_linear] = position[axis_linear];

  // Initialize the extruder axis
  arc_target[E_AXIS] = position[E_AXIS]; // * Printer::invAxisStepsPerMM[E_AXIS]; -- Not sure what this does

  for (i = 1; i < segments; i++) {
    // Increment (segments-1)

    if (count < args_.n_arc_correction) { //25 pieces
        // Apply vector rotation matrix
      r_axisi = r_axis0 * sin_T + r_axis1 * cos_T;
      r_axis0 = r_axis0 * cos_T - r_axis1 * sin_T;
      r_axis1 = r_axisi;
      count++;
    }
    else {
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      cos_Ti = (float)utilities::cos(i * (double)theta_per_segment);
      sin_Ti = (float)utilities::sin(i * (double)theta_per_segment);
      r_axis0 = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      r_axis1 = -offset[0] * sin_Ti - offset[1] * cos_Ti;
      count = 0;
    }

    // Update arc_target location
    arc_target[X_AXIS] = center_axis0 + r_axis0;
    arc_target[Y_AXIS] = center_axis1 + r_axis1;
    arc_target[Z_AXIS] += linear_per_segment;
    arc_target[E_AXIS] += extruder_per_segment;
    moveToReal(arc_target[X_AXIS], arc_target[Y_AXIS], position[Z_AXIS], arc_target[E_AXIS]);
  }
  // Ensure last segment arrives at target location.
  moveToReal(target[X_AXIS], target[Y_AXIS], position[Z_AXIS], target[E_AXIS]);
}

/// <summary>
/// This function was adapted from the 1.0.4 release of Repetier firmware, which can be found at the following link:
/// https://github.com/repetier/Repetier-Firmware/blob/2bbda51eb6407faf29a09987fd635c86818d32db/src/ArduinoAVR/Repetier/motion.cpp
/// This copyright notice was taken from the link above:
/// 
/// This file is part of Repetier-Firmware.
/// Repetier-Firmware is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// Repetier-Firmware is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// You should have received a copy of the GNU General Public License
/// along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.
/// This firmware is a nearly complete rewrite of the sprinter firmware
/// by kliment (https://github.com/kliment/Sprinter)
/// which based on Tonokip RepRap firmware rewrite based off of Hydra-mmm firmware.
/// Functions in this file are used to communicate using ascii or repetier protocol.
/// </summary>
/// <param name="cart">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="clockwise">Is the motion clockwise or counterclockwise</param>
void repetier::arc_1_0_4(float* position, float* target, float* offset, float radius, uint8_t isclockwise)
{
  //   int acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
    //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
  float center_axis0 = position[X_AXIS] + offset[X_AXIS];
  float center_axis1 = position[Y_AXIS] + offset[Y_AXIS];
  //float linear_travel = 0; //target[axis_linear] - position[axis_linear];
  float extruder_travel = (target[E_AXIS] - position[E_AXIS]); // * Printer::invAxisStepsPerMM[E_AXIS];  -- Not sure what this does...
  float r_axis0 = -offset[0]; // Radius vector from center to current location
  float r_axis1 = -offset[1];
  float rt_axis0 = target[0] - center_axis0;
  float rt_axis1 = target[1] - center_axis1;
  /*long xtarget = Printer::destinationSteps[X_AXIS];
  long ytarget = Printer::destinationSteps[Y_AXIS];
  long ztarget = Printer::destinationSteps[Z_AXIS];
  long etarget = Printer::destinationSteps[E_AXIS];
  */
  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel = (float)utilities::atan2((double)r_axis0 * rt_axis1 - (double)r_axis1 * rt_axis0, (double)r_axis0 * rt_axis0 + (double)r_axis1 * rt_axis1);
  if ((!isclockwise && angular_travel <= 0.00001) || (isclockwise && angular_travel < -0.000001)) {
    angular_travel += 2.0f * PI_FLOAT;
  }
  if (isclockwise) {
    angular_travel -= 2.0f * PI_FLOAT;
  }

  float millimeters_of_travel = (float)utilities::fabs(angular_travel) * radius;
  if (millimeters_of_travel < 0.001f) {
    return; // treat as succes because there is nothing to do;
  }
  //uint16_t segments = (radius>=BIG_ARC_RADIUS ? floor(millimeters_of_travel/MM_PER_ARC_SEGMENT_BIG) : floor(millimeters_of_travel/MM_PER_ARC_SEGMENT));
  // Increase segment size if printing faster then computation speed allows
  uint16_t segments = (uint16_t)(feedrate > 60.0f ? utilities::floorf(millimeters_of_travel / utilities::minf(static_cast<float>(args_.mm_per_arc_segment), feedrate * 0.01666f * static_cast<float>(args_.mm_per_arc_segment))) : utilities::floorf(millimeters_of_travel / static_cast<float>(args_.mm_per_arc_segment)));
  if (segments == 0)
    segments = 1;
  /*
    // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
    // by a number of discrete segments. The inverse feed_rate should be correct for the sum of
    // all segments.
    if (invert_feed_rate) { feed_rate *= segments; }
  */
  float theta_per_segment = angular_travel / segments;
  //float linear_per_segment = linear_travel/segments;
  float extruder_per_segment = extruder_travel / segments;

  /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
     and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
         r_T = [cos(phi) -sin(phi);
                sin(phi)  cos(phi] * r ;
     For arc generation, the center of the circle is the axis of rotation and the radius vector is
     defined from the circle center to the initial position. Each line segment is formed by successive
     vector rotations. This requires only two cos() and sin() computations to form the rotation
     matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
     all double numbers are single precision on the Arduino. (True double precision will not have
     round off issues for CNC applications.) Single precision error can accumulate to be greater than
     tool precision in some cases. Therefore, arc path correction is implemented.
     Small angle approximation may be used to reduce computation overhead further. This approximation
     holds for everything, but very small circles and large mm_per_arc_segment values. In other words,
     theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
     to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
     numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
     issue for CNC machines with the single precision Arduino calculations.
     This approximation also allows mc_arc to immediately insert a line segment into the planner
     without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
     a correction, the planner should have caught up to the lag caused by the initial mc_arc overhead.
     This is important when there are successive arc motions.
  */
  // Vector rotation matrix values
  float cos_T = 1 - 0.5f * theta_per_segment * theta_per_segment; // Small angle approximation
  float sin_T = theta_per_segment;

  float arc_target[4];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count = 0;

  // Initialize the linear axis
  //arc_target[axis_linear] = position[axis_linear];

  // Initialize the extruder axis
  arc_target[E_AXIS] = position[E_AXIS]; // * Printer::invAxisStepsPerMM[E_AXIS]; -- Not sure what this does

  for (i = 1; i < segments; i++) {
    // Increment (segments-1)

    if (count < args_.n_arc_correction) { //25 pieces
        // Apply vector rotation matrix
      r_axisi = r_axis0 * sin_T + r_axis1 * cos_T;
      r_axis0 = r_axis0 * cos_T - r_axis1 * sin_T;
      r_axis1 = r_axisi;
      count++;
    }
    else {
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      cos_Ti = (float)utilities::cos(i * (double)theta_per_segment);
      sin_Ti = (float)utilities::sin(i * (double)theta_per_segment);
      r_axis0 = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      r_axis1 = -offset[0] * sin_Ti - offset[1] * cos_Ti;
      count = 0;
    }

    // Update arc_target location
    arc_target[X_AXIS] = center_axis0 + r_axis0;
    arc_target[Y_AXIS] = center_axis1 + r_axis1;
    //arc_target[axis_linear] += linear_per_segment;
    arc_target[E_AXIS] += extruder_per_segment;
    moveToReal(arc_target[X_AXIS], arc_target[Y_AXIS], position[Z_AXIS], arc_target[E_AXIS]);
  }
  // Ensure last segment arrives at target location.
  moveToReal(target[X_AXIS], target[Y_AXIS], position[Z_AXIS], target[E_AXIS]);
}


//void repetier::buffer_line_kinematic(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target)
void repetier::moveToReal(float x, float y, float z, float e)
{

  // create the target position
  firmware_position target;
  target.x = x;
  target.y = y;
  target.z = z;
  target.e = e;
  target.f = feedrate;
  if (gcodes_.size() > 0)
  {
    gcodes_ += "\n";
  }
  // Generate the gcode
  gcodes_ += g1_command(target);

  // update the current position
  set_current_position(target);
}

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


#include "prusa.h"
#include <cmath>
#include "utilities.h"
prusa::prusa(firmware_arguments args) : firmware(args) {
  apply_arguments();
};

void prusa::apply_arguments()
{
  const std::vector<std::string> prusa_firmware_version_names{
         "3.10.0", "3.11.0"
  };
  set_versions(prusa_firmware_version_names, "3.10.0");
  prusa_version_ = (prusa::prusa_firmware_versions)version_index_;
  std::vector<std::string> used_arguments;

  switch (prusa_version_)
  {
  case prusa::prusa_firmware_versions::V3_11_0:
    mc_arc_ = &prusa::mc_arc_3_11_0; 
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS, FIRMWARE_ARGUMENT_MIN_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    break;
  default:
    mc_arc_ = &prusa::mc_arc_3_10_0;
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    break;
  }
  args_.set_used_arguments(used_arguments);
}


firmware_arguments prusa::get_default_arguments_for_current_version() const
{
  // Start off with the current args so they are set up correctly for this firmware type and version
  firmware_arguments default_args = args_;

  // firmware defaults
  default_args.g90_g91_influences_extruder = false;
  // Leave the switch in here in case we want to add more versions.
  switch (prusa_version_)
  {
  case prusa::prusa_firmware_versions::V3_11_0:
    // Active Settings
    default_args.mm_per_arc_segment = 1.0f;
    default_args.n_arc_correction = 25;
    default_args.min_arc_segments = 24;
    // Inactive Settings
    default_args.arc_segments_per_r = 0;
    default_args.min_mm_per_arc_segment = 0;
    default_args.arc_segments_per_sec = 0;
    break;
  default:
    // Active Settings
    default_args.mm_per_arc_segment = 1.0f;
    default_args.min_arc_segments = 24;
    default_args.n_arc_correction = 25;
    // Inactive Settings
    default_args.arc_segments_per_r = 0;
    default_args.min_mm_per_arc_segment = 0;
    default_args.arc_segments_per_sec = 0;
    break;
  }
  return default_args;
}

std::string prusa::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
  // Clear the current list of gcodes
  gcodes_.clear();
  // Set up the necessary values to call mc_arc
  float prusa_position[4];
  prusa_position[X_AXIS] = static_cast<float>(position_.x);
  prusa_position[Y_AXIS] = static_cast<float>(position_.y);
  prusa_position[Z_AXIS] = static_cast<float>(position_.z);
  prusa_position[E_AXIS] = static_cast<float>(position_.e);
  float prusa_target[4];
  prusa_target[X_AXIS] = static_cast<float>(target.x);
  prusa_target[Y_AXIS] = static_cast<float>(target.y);
  prusa_target[Z_AXIS] = static_cast<float>(target.z);
  prusa_target[E_AXIS] = static_cast<float>(target.e);
  float prusa_offset[2];
  prusa_offset[0] = static_cast<float>(i);
  prusa_offset[1] = static_cast<float>(j);
  float prusa_radius = static_cast<float>(r);
  if (prusa_radius != 0)
  {
    prusa_radius = utilities::hypotf(prusa_offset[X_AXIS], prusa_offset[Y_AXIS]); // Compute arc radius for mc_arc
  }
  float prusa_f = static_cast<float>(target.f);
  
  uint8_t prusa_isclockwise = is_clockwise ? 1 : 0;
  
  (this->*mc_arc_)(prusa_position, prusa_target, prusa_offset, prusa_f, prusa_radius, prusa_isclockwise, 0);

  return gcodes_;
}

/// <summary>
/// This function was adapted from the 3.10.0 release of Prusa's firmware, which can be found at the following link:
/// https://github.com/prusa3d/Prusa-Firmware/blob/04de9c0c8a49a4bee11b8c4789be3f18c1e1ccfe/Firmware/motion_control.cpp
/// Copyright Notice found on that page:
/// 
/// motion_control.c - high level interface for issuing motion commands
/// Part of Grbl
/// Copyright(c) 2009 - 2011 Simen Svale Skogsrud
/// Copyright(c) 2011 Sungeun K.Jeon
/// 
/// Grbl is free software : you can redistribute it and /or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// Grbl is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
/// GNU General Public License for more details.
/// You should have received a copy of the GNU General Public License
/// along with Grbl.If not, see < http://www.gnu.org/licenses/>.
/// </summary>
/// <param name="position">The current position</param>
/// <param name="target">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="feed_rate">The target feedrate</param>
/// <param name="radius">The radius of the arc.</param>
/// <param name="isclockwise">Is the motion clockwise or counterclockwise</param>
/// <param name="extruder">unknown</param>
void prusa::mc_arc_3_10_0(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
{
  /// Setup * This code is NOT in the original source, and is added for compatibility with the function definition
  uint8_t axis_0 = X_AXIS;
  uint8_t axis_1 = Y_AXIS;
  uint8_t axis_linear = Z_AXIS;

  //   int acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
  //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
  float center_axis0 = position[axis_0] + offset[axis_0];
  float center_axis1 = position[axis_1] + offset[axis_1];
  float linear_travel = target[axis_linear] - position[axis_linear];
  float extruder_travel = target[E_AXIS] - position[E_AXIS];
  float r_axis0 = -offset[axis_0];  // Radius vector from center to current location
  float r_axis1 = -offset[axis_1];
  float rt_axis0 = target[axis_0] - center_axis0;
  float rt_axis1 = target[axis_1] - center_axis1;

  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel = (float)utilities::atan2((double)r_axis0 * rt_axis1 - (double)r_axis1 * rt_axis0, (double)r_axis0 * rt_axis0 + (double)r_axis1 * rt_axis1);
  if (angular_travel < 0) { angular_travel += 2.0f * PI_FLOAT; }
  if (isclockwise) { angular_travel -= 2.0f * PI_FLOAT; }

  //20141002:full circle for G03 did not work, e.g. G03 X80 Y80 I20 J0 F2000 is giving an Angle of zero so head is not moving
  //to compensate when start pos = target pos && angle is zero -> angle = 2Pi
  if (position[axis_0] == target[axis_0] && position[axis_1] == target[axis_1] && angular_travel == 0)
  {
    angular_travel += 2.0f * PI_FLOAT;
  }
  //end fix G03

  float millimeters_of_travel = (float)utilities::hypot((double)angular_travel * radius, utilities::absf(linear_travel));
  if (millimeters_of_travel < 0.001) { return; }
  uint16_t segments = (uint16_t)utilities::floorf(millimeters_of_travel / static_cast<float>(args_.mm_per_arc_segment));
  if (segments == 0) segments = 1;

  /*
    // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
    // by a number of discrete segments. The inverse feed_rate should be correct for the sum of
    // all segments.
    if (invert_feed_rate) { feed_rate *= segments; }
  */
  float theta_per_segment = angular_travel / segments;
  float linear_per_segment = linear_travel / segments;
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
  float cos_T = 1 - (float)0.5 * theta_per_segment * theta_per_segment; // Small angle approximation
  float sin_T = theta_per_segment;

  float arc_target[4];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count = 0;

  // Initialize the linear axis
  arc_target[axis_linear] = position[axis_linear];

  // Initialize the extruder axis
  arc_target[E_AXIS] = position[E_AXIS];

  for (i = 1; i < segments; i++) { // Increment (segments-1)

    if (count < args_.n_arc_correction) {
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
      r_axis0 = -offset[axis_0] * cos_Ti + offset[axis_1] * sin_Ti;
      r_axis1 = -offset[axis_0] * sin_Ti - offset[axis_1] * cos_Ti;
      count = 0;
    }

    // Update arc_target location
    arc_target[axis_0] = center_axis0 + r_axis0;
    arc_target[axis_1] = center_axis1 + r_axis1;
    arc_target[axis_linear] += linear_per_segment;
    arc_target[E_AXIS] += extruder_per_segment;

    clamp_to_software_endstops(arc_target);
    plan_buffer_line(arc_target[X_AXIS], arc_target[Y_AXIS], arc_target[Z_AXIS], arc_target[E_AXIS], feed_rate, extruder, NULL);

  }
  // Ensure last segment arrives at target location.
  plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feed_rate, extruder, NULL);

  //   plan_set_acceleration_manager_enabled(acceleration_manager_was_enabled);
}

/// <summary>
/// This function was taken from a pull request I submitted for the 3.11.0 release of Prusa's firmware, which can be found at the following link:
/// https://github.com/FormerLurker/Prusa-Firmware/blob/MK3/Firmware/motion_control.cpp
/// 
/// It was forked from the original at: https://github.com/prusa3d/Prusa-Firmware
/// 
/// Copyright Notice found on that page:
/// 
/// motion_control.c - high level interface for issuing motion commands
/// Part of Grbl
/// Copyright(c) 2009 - 2011 Simen Svale Skogsrud
/// Copyright(c) 2011 Sungeun K.Jeon
/// Copyright(C) 2021 Brad Hochgesang
/// Grbl is free software : you can redistribute it and /or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// Grbl is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
/// GNU General Public License for more details.
/// You should have received a copy of the GNU General Public License
/// along with Grbl.If not, see < http://www.gnu.org/licenses/>.
/// </summary>
/// <param name="position">The current position</param>
/// <param name="target">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="feed_rate">The target feedrate</param>
/// <param name="radius">The radius of the arc.</param>
/// <param name="isclockwise">Is the motion clockwise or counterclockwise</param>
/// <param name="extruder">unknown</param>  
void prusa::mc_arc_3_11_0(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
{
    
  float r_axis_x = -offset[X_AXIS];  // Radius vector from center to current location
  float r_axis_y = -offset[Y_AXIS];
  float center_axis_x = position[X_AXIS] - r_axis_x;
  float center_axis_y = position[Y_AXIS] - r_axis_y;
  float travel_z = target[Z_AXIS] - position[Z_AXIS];
  float rt_x = target[X_AXIS] - center_axis_x;
  float rt_y = target[Y_AXIS] - center_axis_y;
  // 20200419 - Add a variable that will be used to hold the arc segment length
  float mm_per_arc_segment = static_cast<float>(args_.mm_per_arc_segment);
  // 20210109 - Add a variable to hold the n_arc_correction value
  uint8_t n_arc_correction = args_.n_arc_correction;

  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel_total = (float)utilities::atan2((double)r_axis_x * rt_y - (double)r_axis_y * rt_x, (double)r_axis_x * rt_x + (double)r_axis_y * rt_y);
  if (angular_travel_total < 0) { angular_travel_total += 2.0f * PI_FLOAT; }

  if (args_.min_arc_segments > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    // Do this before converting the angular travel for clockwise rotation
    mm_per_arc_segment = radius * ((2.0f * PI_FLOAT) / args_.min_arc_segments);
  }
  if (args_.arc_segments_per_sec > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    float mm_per_arc_segment_sec = (feed_rate / 60.0f) * (1.0f / (float)args_.arc_segments_per_sec);
    if (mm_per_arc_segment_sec < mm_per_arc_segment)
      mm_per_arc_segment = mm_per_arc_segment_sec;
  }

  // Note:  no need to check to see if min_mm_per_arc_segment is enabled or not (i.e. = 0), since mm_per_arc_segment can never be below 0.
  if (mm_per_arc_segment < args_.min_mm_per_arc_segment)
  {
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    // This prevents a very high number of segments from being generated for curves of a short radius
    mm_per_arc_segment = static_cast<float>(args_.min_mm_per_arc_segment);
  }
  else if (mm_per_arc_segment > args_.mm_per_arc_segment) {
    // 20210113 - This can be implemented in an else if since  we can't be below the min AND above the max at the same time.
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    mm_per_arc_segment = static_cast<float>(args_.mm_per_arc_segment);
  }

  // Adjust the angular travel if the direction is clockwise
  if (isclockwise) { angular_travel_total -= 2.0f * PI_FLOAT; }

  //20141002:full circle for G03 did not work, e.g. G03 X80 Y80 I20 J0 F2000 is giving an Angle of zero so head is not moving
  //to compensate when start pos = target pos && angle is zero -> angle = 2Pi
  if (position[X_AXIS] == target[X_AXIS] && position[Y_AXIS] == target[Y_AXIS] && angular_travel_total == 0)
  {
    angular_travel_total += 2.0f * PI_FLOAT;
  }
  //end fix G03

  // 20200417 - FormerLurker - rename millimeters_of_travel to millimeters_of_travel_arc to better describe what we are
  // calculating here
  const float millimeters_of_travel_arc = (float)utilities::hypot(angular_travel_total * (double)radius, utilities::fabs(travel_z));
  if (millimeters_of_travel_arc < 0.001) { return; }

  // Calculate the number of arc segments
  uint16_t segments = static_cast<uint16_t>(utilities::ceilf(millimeters_of_travel_arc / mm_per_arc_segment));

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
     The small angle approximation was removed because of excessive errors for small circles (perhaps unique to
     3d printing applications, causing significant path deviation and extrusion issues).
     Now there will be no corrections applied, but an accurate initial sin and cos will be calculated.
     This seems to work with a very high degree of accuracy and results in much simpler code.
     Finding a faster way to approximate sin, knowing that there can be substantial deviations from the true
     arc when using the previous approximation, would be beneficial.
  */

  // If there is only one segment, no need to do a bunch of work since this is a straight line!
  if (segments > 1)
  {
    // Calculate theta per segments, and linear (z) travel per segment, e travel per segment
    // as well as the small angle approximation for sin and cos.
    const float theta_per_segment = angular_travel_total / segments,
      linear_per_segment = travel_z / (segments),
      segment_extruder_travel = (target[E_AXIS] - position[E_AXIS]) / (segments),
      sq_theta_per_segment = theta_per_segment * theta_per_segment,
      sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6,
      cos_T = 1 - 0.5f * sq_theta_per_segment;
    // Loop through all but one of the segments.  The last one can be done simply
    // by moving to the target.
    for (uint16_t i = 1; i < segments; i++) {
      if (n_arc_correction-- == 0) {
        // Calculate the actual position for r_axis_x and r_axis_y
        const float cos_Ti = (float)utilities::cos(i * (double)theta_per_segment), sin_Ti = (float)utilities::sin(i * (double)theta_per_segment);
        r_axis_x = -offset[X_AXIS] * cos_Ti + offset[Y_AXIS] * sin_Ti;
        r_axis_y = -offset[X_AXIS] * sin_Ti - offset[Y_AXIS] * cos_Ti;
        // reset n_arc_correction
        n_arc_correction = args_.n_arc_correction;
      }
      else {
        // Calculate X and Y using the small angle approximation
        const float r_axisi = r_axis_x * sin_T + r_axis_y * cos_T;
        r_axis_x = r_axis_x * cos_T - r_axis_y * sin_T;
        r_axis_y = r_axisi;
      }

      // Update Position
      position[X_AXIS] = center_axis_x + r_axis_x;
      position[Y_AXIS] = center_axis_y + r_axis_y;
      position[Z_AXIS] += linear_per_segment;
      position[E_AXIS] += segment_extruder_travel;
      // Clamp to the calculated position.
      clamp_to_software_endstops(position);
      // Insert the segment into the buffer
      plan_buffer_line(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS], feed_rate, extruder, position);
    }
  }
  // Clamp to the target position.
  clamp_to_software_endstops(target);
  // Ensure last segment arrives at target location.
  plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feed_rate, extruder, target);
}

void prusa::clamp_to_software_endstops(float* target)
{
    // Do nothing, just added to keep mc_arc identical to the firmware version
    return;
}

void prusa::plan_buffer_line(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target)
{
  // create the target position
  firmware_position target;
  target.x = x;
  target.y = y;
  target.z = z;
  target.e = e;
  target.f = feed_rate;
  if (gcodes_.size() > 0)
  {
    gcodes_ += "\n";
  }
  // Generate the gcode
  gcodes_ += g1_command(target);

  // update the current position
  set_current_position(target);
}

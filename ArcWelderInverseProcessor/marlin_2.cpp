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

#include "marlin_2.h"
#include "utilities.h"
marlin_2::marlin_2(firmware_arguments args) : firmware(args) {
  feedrate_mm_s = 0;
  current_position = new float[MARLIN_2_XYZE];
  apply_arguments();
};

marlin_2::~marlin_2()
{
  delete current_position;
}

void marlin_2::apply_arguments()
{
  static const std::vector<std::string> marlin_2_firmware_version_names{
       "2.0.9.1", "2.0.9.2"
  };
  set_versions(marlin_2_firmware_version_names, "2.0.9.1");
  marlin_2_version_ = (marlin_2::marlin_2_firmware_versions)version_index_;
  std::vector<std::string> used_arguments;
  switch (marlin_2_version_)
  {
  case marlin_2::marlin_2_firmware_versions::V2_0_9_2:
    used_arguments = { FIRMWARE_ARGUMENT_MIN_ARC_SEGMENT_MM, FIRMWARE_ARGUMENT_MAX_ARC_SEGMENT_MM, FIRMWARE_ARGUMENT_MIN_CIRCLE_SEGMENTS, FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    plan_arc_ = &marlin_2::plan_arc_2_0_9_2;
    break;
  default:
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_ARC_SEGMENT_PER_R, FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS, FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    plan_arc_ = &marlin_2::plan_arc_2_0_9_1;
    break;
  }
  args_.set_used_arguments(used_arguments);
}

firmware_arguments marlin_2::get_default_arguments_for_current_version() const
{
  // Start off with the current args so they are set up correctly for this firmware type and version
    firmware_arguments default_args = args_;
  
  // firmware defaults
  default_args.g90_g91_influences_extruder = true;

  switch (marlin_2_version_)
  {
  case marlin_2::marlin_2_firmware_versions::V2_0_9_2:
      // Active Settings
      default_args.min_arc_segment_mm = 0.1f;
      default_args.max_arc_segment_mm = 1.0f;
      default_args.min_circle_segments = 72;
      default_args.n_arc_correction = 25;
      // Inactive Settings
      default_args.arc_segments_per_r = 0;
      default_args.arc_segments_per_sec = 0;
    break;
    default:
      // Active Settings
      default_args.mm_per_arc_segment = 1.0f;
      default_args.min_arc_segments = 24;
      default_args.n_arc_correction = 25;
      // Inactive Settings
      default_args.arc_segments_per_r = 0;
      default_args.arc_segments_per_sec = 0;
      break;
  }
  return default_args;
}

std::string marlin_2::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
  // Clear the current list of gcodes
  gcodes_.clear();

  // Setup the current position
  current_position[X_AXIS] = static_cast<float>(position_.x);
  current_position[Y_AXIS] = static_cast<float>(position_.y);
  current_position[Z_AXIS] = static_cast<float>(position_.z);
  current_position[E_AXIS] = static_cast<float>(position_.e);
  float marlin_target[MARLIN_2_XYZE];
  marlin_target[X_AXIS] = static_cast<float>(target.x);
  marlin_target[Y_AXIS] = static_cast<float>(target.y);
  marlin_target[Z_AXIS] = static_cast<float>(target.z);
  marlin_target[E_AXIS] = static_cast<float>(target.e);
  float marlin_offset[2];
  marlin_offset[0] = static_cast<float>(i);
  marlin_offset[1] = static_cast<float>(j);
  // TODO:  handle R form!!

  // Set the feedrate
  feedrate_mm_s = static_cast<float>(target.f);
  uint8_t marlin_isclockwise = is_clockwise ? 1 : 0;

  (this->*plan_arc_)(marlin_target, marlin_offset, marlin_isclockwise, 0);

  return gcodes_;
}

/// <summary>
/// This function was adapted from the 2.0.9.1 release of Marlin firmware, which can be found at the following link:
/// https://github.com/MarlinFirmware/Marlin/blob/b878127ea04cc72334eb35ce0dca39ccf7d73a68/Marlin/src/gcode/motion/G2_G3.cpp
/// Copyright Notice found on that page:
/// 
/// 
/// Marlin 3D Printer Firmware
/// Copyright (C) 2016, 2017 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
/// 
/// Based on Sprinter and grbl.
/// Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
/// 
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
/// </summary>
/// <param name="cart">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="clockwise">Is the motion clockwise or counterclockwise</param>
void marlin_2::plan_arc_2_0_9_1(
  const float(&cart)[MARLIN_2_XYZE],   // Destination position
  const float(&offset)[2], // Center of rotation relative to current_position
  const bool clockwise,     // Clockwise?
  const uint8_t circles     // Take the scenic route
)
{
  uint8_t p_axis = X_AXIS, q_axis = Y_AXIS, l_axis = Z_AXIS;

  // Radius vector from center to current location
  float rvec[2];
  rvec[0] = - offset[X_AXIS];
  rvec[1] = - offset[Y_AXIS];

  const float radius = utilities::hypotf(rvec[0], rvec[1]),
    center_P = current_position[p_axis] - rvec[0],
    center_Q = current_position[q_axis] - rvec[1],
    rt_X = cart[p_axis] - center_P,
    rt_Y = cart[q_axis] - center_Q,
    start_L = current_position[l_axis];

  uint16_t min_segments = args_.min_arc_segments > 0 ? args_.min_arc_segments : 1;

  // Angle of rotation between position and target from the circle center.
  float angular_travel;

  // Do a full circle if starting and ending positions are "identical"
  if (NEAR(current_position[p_axis], cart[p_axis]) && NEAR(current_position[q_axis], cart[q_axis])) {
    // Preserve direction for circles
    angular_travel = clockwise ? -utilities::radiansf(360.0f) : utilities::radiansf(360.0f);
  }
  else {
    // Calculate the angle
    angular_travel = utilities::atan2f(rvec[0] * rt_Y - rvec[1] * rt_X, rvec[0] * rt_X + rvec[1] * rt_Y);

    // Angular travel too small to detect? Just return.
    if (!angular_travel) return;

    // Make sure angular travel over 180 degrees goes the other way around.
    switch (((angular_travel < 0) << 1) | (int)clockwise) {
    case 1: angular_travel -= utilities::radiansf(360.0f); break; // Positive but CW? Reverse direction.
    case 2: angular_travel += utilities::radiansf(360.0f); break; // Negative but CCW? Reverse direction.
    }

    if (args_.min_arc_segments > 1)
    {
      min_segments = (uint16_t)utilities::ceilf(min_segments * utilities::absf(angular_travel) / utilities::radiansf(360.0f));
      NOLESS(min_segments, 1U);
    }
  }


  float linear_travel = cart[Z_AXIS] - start_L;
  float extruder_travel = cart[E_AXIS] - current_position[E_AXIS];

  const float flat_mm = radius * angular_travel,
    mm_of_travel = linear_travel ? utilities::hypotf(flat_mm, linear_travel) : utilities::absf(flat_mm);
  if (mm_of_travel < 0.001f) return;

  const float scaled_fr_mm_s = MMS_SCALED(feedrate_mm_s);

  // Start with a nominal segment length
  float seg_length = (float)args_.mm_per_arc_segment;
  if (args_.arc_segments_per_r > 0)
  {
    seg_length = utilities::constrainf((float)args_.mm_per_arc_segment * radius, (float)args_.mm_per_arc_segment, (float)args_.arc_segments_per_r);
  }
  else if (args_.arc_segments_per_sec > 0)
  {
    seg_length = utilities::maxf(scaled_fr_mm_s * utilities::reciprocalf((float)args_.arc_segments_per_sec), (float)args_.mm_per_arc_segment);
  }

  // Divide total travel by nominal segment length
  uint16_t segments = (uint16_t)utilities::floorf(mm_of_travel / seg_length);
  //uint16_t segments = utilities::floorf(mm_of_travel / seg_length);
  NOLESS(segments, min_segments);         // At least some segments
  seg_length = mm_of_travel / segments;

  /**
   * Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
   * and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
   *     r_T = [cos(phi) -sin(phi);
   *            sin(phi)  cos(phi)] * r ;
   *
   * For arc generation, the center of the circle is the axis of rotation and the radius vector is
   * defined from the circle center to the initial position. Each line segment is formed by successive
   * vector rotations. This requires only two cos() and sin() computations to form the rotation
   * matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
   * all double numbers are single precision on the Arduino. (True double precision will not have
   * round off issues for CNC applications.) Single precision error can accumulate to be greater than
   * tool precision in some cases. Therefore, arc path correction is implemented.
   *
   * Small angle approximation may be used to reduce computation overhead further. This approximation
   * holds for everything, but very small circles and large MM_PER_ARC_SEGMENT values. In other words,
   * theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
   * to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
   * numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
   * issue for CNC machines with the single precision Arduino calculations.
   *
   * This approximation also allows plan_arc to immediately insert a line segment into the planner
   * without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
   * a correction, the planner should have caught up to the lag caused by the initial plan_arc overhead.
   * This is important when there are successive arc motions.
   */
   // Vector rotation matrix values
  float raw[MARLIN_2_XYZE];
  const float theta_per_segment = angular_travel / segments,
    sq_theta_per_segment = utilities::sqf(theta_per_segment),
    sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6,
    cos_T = 1 - 0.5f * sq_theta_per_segment; // Small angle approximation


  const float linear_per_segment = linear_travel / segments;
  const float extruder_per_segment = extruder_travel / segments;

  // Initialize the linear axis
  raw[l_axis] = current_position[l_axis];

  // Initialize the extruder axis
  raw[E_AXIS] = current_position[E_AXIS];

  int8_t arc_recalc_count = args_.n_arc_correction;

  for (uint16_t i = 1; i < segments; i++) { // Iterate (segments-1) times

    if (args_.n_arc_correction > 1 && --arc_recalc_count)
    {
      // Apply vector rotation matrix to previous rvec[0] / 1
      const float r_new_Y = rvec[0] * sin_T + rvec[1] * cos_T;
      rvec[0] = rvec[0] * cos_T - rvec[1] * sin_T;
      rvec[1] = r_new_Y;
    }
    else
    {
      if (args_.n_arc_correction > 1)
      {
        arc_recalc_count = args_.n_arc_correction;
      }
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      // To reduce stuttering, the sin and cos could be computed at different times.
      // For now, compute both at the same time.
      const float cos_Ti = (float)utilities::cos((i * (double)theta_per_segment));
      const float sin_Ti = (float)utilities::sin((i * (double)theta_per_segment));
      rvec[0] = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      rvec[1] = -offset[0] * sin_Ti - offset[1] * cos_Ti;
    }

    // Update raw location
    raw[p_axis] = center_P + rvec[0];
    raw[q_axis] = center_Q + rvec[1];
    raw[l_axis] = start_L, raw[l_axis] + linear_per_segment;

    raw[E_AXIS] += extruder_per_segment;

    apply_motion_limits(raw);

    if (!buffer_line(raw, scaled_fr_mm_s, 0))
    {
      break;
    }
  }

  // Ensure last segment arrives at target location.
  COPY(raw, cart);
  raw[l_axis] = start_L;

  apply_motion_limits(raw);


  buffer_line(raw, scaled_fr_mm_s, 0);

  raw[l_axis] = start_L;
  COPY(current_position, raw);
}



/// <summary>
/// This function was adapted from the 2.0.9.2 release of Marlin firmware, which can be found at the following link:
/// https://github.com/MarlinFirmware/Marlin/blob/b878127ea04cc72334eb35ce0dca39ccf7d73a68/Marlin/src/gcode/motion/G2_G3.cpp
/// Copyright Notice found on that page:
/// 
/// 
/// Marlin 3D Printer Firmware
/// Copyright (C) 2016, 2017 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
/// 
/// Based on Sprinter and grbl.
/// Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
/// 
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
/// </summary>
/// <param name="cart">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="clockwise">Is the motion clockwise or counterclockwise</param>
void marlin_2::plan_arc_2_0_9_2(
  const float(&cart)[MARLIN_2_XYZE],   // Destination position
  const float(&offset)[2], // Center of rotation relative to current_position
  const bool clockwise,     // Clockwise?
  const uint8_t circles     // Take the scenic route
)
{
  int min_circle_segments = args_.min_circle_segments > 0 ? args_.min_circle_segments : 1;
  uint8_t p_axis = X_AXIS, q_axis = Y_AXIS, l_axis = Z_AXIS;

  // Radius vector from center to current location
  float rvec[2];
  rvec[0] = -offset[X_AXIS];
  rvec[1] = -offset[Y_AXIS];

  const float radius = utilities::hypotf(rvec[0], rvec[1]),
    center_P = current_position[p_axis] - rvec[0],
    center_Q = current_position[q_axis] - rvec[1],
    rt_X = cart[p_axis] - center_P,
    rt_Y = cart[q_axis] - center_Q,
    start_L = current_position[l_axis];

  uint16_t min_segments = args_.min_arc_segments > 0 ? args_.min_arc_segments : 1;

  // Angle of rotation between position and target from the circle center.
  float angular_travel, abs_angular_travel;

  // Do a full circle if starting and ending positions are "identical"
  if (NEAR(current_position[p_axis], cart[p_axis]) && NEAR(current_position[q_axis], cart[q_axis])) {
    // Preserve direction for circles
    angular_travel = clockwise ? -utilities::radiansf(360.0f) : utilities::radiansf(360.0f);
    abs_angular_travel = utilities::radiansf(360.0f);
    min_segments = min_circle_segments;
  }
  else {
    // Calculate the angle
    angular_travel = utilities::atan2f(rvec[0] * rt_Y - rvec[1] * rt_X, rvec[0] * rt_X + rvec[1] * rt_Y);

    // Angular travel too small to detect? Just return.
    if (!angular_travel) return;

    // Make sure angular travel over 180 degrees goes the other way around.
    switch (((angular_travel < 0) << 1) | (int)clockwise) {
    case 1: angular_travel -= utilities::radiansf(360.0f); break; // Positive but CW? Reverse direction.
    case 2: angular_travel += utilities::radiansf(360.0f); break; // Negative but CCW? Reverse direction.
    }

    abs_angular_travel = utilities::absf(angular_travel);

    // Apply minimum segments to the arc
    const float portion_of_circle = abs_angular_travel / utilities::radiansf(360.0f);  // Portion of a complete circle (0 < N < 1)
    min_segments = (uint16_t)utilities::ceilf((min_circle_segments)*portion_of_circle);     // Minimum segments for the arc
  }

  float travel_L = cart[Z_AXIS] - start_L;
  float travel_E = cart[E_AXIS] - current_position[E_AXIS];
  
  // Millimeters in the arc, assuming it's flat
  const float flat_mm = radius * abs_angular_travel;
  if (flat_mm < 0.001f 
    && travel_L < 0.001f
  ) return;

  // Feedrate for the move, scaled by the feedrate multiplier
  const float scaled_fr_mm_s = MMS_SCALED(feedrate_mm_s);

  // Get the nominal segment length based on settings
  float nominal_segment_mm;
  if (args_.arc_segments_per_sec > 0) {
    nominal_segment_mm = utilities::constrainf(scaled_fr_mm_s * utilities::reciprocalf((float)args_.arc_segments_per_sec), (float)args_.min_arc_segment_mm, (float)args_.max_arc_segment_mm);
  }
  else {
    nominal_segment_mm = (float)args_.max_arc_segment_mm;
  }
  // Number of whole segments based on the nominal segment length
  const float nominal_segments = utilities::maxf(utilities::floorf(flat_mm / nominal_segment_mm), min_segments);

  // A new segment length based on the required minimum
  const float segment_mm = utilities::constrainf(flat_mm / nominal_segments, (float)args_.min_arc_segment_mm, (float)args_.max_arc_segment_mm);

  // The number of whole segments in the arc, ignoring the remainder
  uint16_t segments = (uint16_t)utilities::floorf(flat_mm / segment_mm);

  // Are the segments now too few to reach the destination?
  const float segmented_length = segment_mm * segments;
  const bool tooshort = segmented_length < flat_mm - 0.0001f;
  const float proportion = tooshort ? segmented_length / flat_mm : 1.0f;
  

  /**
   * Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
   * and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
   *     r_T = [cos(phi) -sin(phi);
   *            sin(phi)  cos(phi)] * r ;
   *
   * For arc generation, the center of the circle is the axis of rotation and the radius vector is
   * defined from the circle center to the initial position. Each line segment is formed by successive
   * vector rotations. This requires only two cos() and sin() computations to form the rotation
   * matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
   * all double numbers are single precision on the Arduino. (True double precision will not have
   * round off issues for CNC applications.) Single precision error can accumulate to be greater than
   * tool precision in some cases. Therefore, arc path correction is implemented.
   *
   * Small angle approximation may be used to reduce computation overhead further. This approximation
   * holds for everything, but very small circles and large MM_PER_ARC_SEGMENT values. In other words,
   * theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
   * to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
   * numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
   * issue for CNC machines with the single precision Arduino calculations.
   *
   * This approximation also allows plan_arc to immediately insert a line segment into the planner
   * without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
   * a correction, the planner should have caught up to the lag caused by the initial plan_arc overhead.
   * This is important when there are successive arc motions.
   */
   // Vector rotation matrix values
  float raw[MARLIN_2_XYZE];
  const float theta_per_segment = proportion * angular_travel / segments,
    sq_theta_per_segment = utilities::sqf(theta_per_segment),
    sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6,
    cos_T = 1 - 0.5f * sq_theta_per_segment; // Small angle approximation


  const float per_segment_L = proportion * travel_L / segments;
  const float extruder_per_segment = proportion * travel_E / segments;

  // For shortened segments, run all but the remainder in the loop
  if (tooshort) segments++;


  // Initialize the linear axis
  raw[l_axis] = current_position[l_axis];
  // Initialize the extruder axis
  raw[E_AXIS] = current_position[E_AXIS];

  int8_t arc_recalc_count = args_.n_arc_correction;

  for (uint16_t i = 1; i < segments; i++) { // Iterate (segments-1) times

    if (args_.n_arc_correction > 1 && --arc_recalc_count)
    {
      // Apply vector rotation matrix to previous rvec[0] / 1
      const float r_new_Y = rvec[0] * sin_T + rvec[1] * cos_T;
      rvec[0] = rvec[0] * cos_T - rvec[1] * sin_T;
      rvec[1] = r_new_Y;
    }
    else
    {
      if (args_.n_arc_correction > 1)
      {
        arc_recalc_count = args_.n_arc_correction;
      }
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      // To reduce stuttering, the sin and cos could be computed at different times.
      // For now, compute both at the same time.
      const float cos_Ti = (float)utilities::cos(i * (double)theta_per_segment);
      const float sin_Ti = (float)utilities::sin(i * (double)theta_per_segment);
      rvec[0] = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      rvec[1] = -offset[0] * sin_Ti - offset[1] * cos_Ti;
    }

    // Update raw location
    raw[p_axis] = center_P + rvec[0];
    raw[q_axis] = center_Q + rvec[1];
    raw[l_axis] = start_L, raw[l_axis] + per_segment_L;
    raw[E_AXIS] += extruder_per_segment;

    apply_motion_limits(raw);

    if (!buffer_line(raw, scaled_fr_mm_s, 0))
    {
      break;
    }
  }

  // Ensure last segment arrives at target location.
  COPY(raw, cart);
  raw[l_axis] = start_L;

  apply_motion_limits(raw);


  buffer_line(raw, scaled_fr_mm_s, 0);

  raw[l_axis] = start_L;
  COPY(current_position, raw);
}
// Marlin Function Defs
void marlin_2::NOLESS(uint16_t& x, uint16_t y)
{
    if (x < y)
        x = y;
}
float marlin_2::MMS_SCALED(float x)
{
  // No scaling
  return x;
}

bool marlin_2::NEAR_ZERO(float x)
{
  return utilities::withinf(x, -0.000001f, 0.000001f);
}
bool marlin_2::NEAR(float x, float y)
{
  return NEAR_ZERO((x)-(y));
}

void marlin_2::COPY(float target[MARLIN_2_XYZE], const float(&source)[MARLIN_2_XYZE])
{
  // This is a slow copy, but speed isn't much of an issue here.
  for (int i = 0; i < MARLIN_2_XYZE; i++)
  {
    target[i] = source[i];
  }
}

void marlin_2::apply_motion_limits(float (&pos)[MARLIN_2_XYZE])
{
  // do nothing
  return;
}

//void marlin::buffer_line_kinematic(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target)
bool marlin_2::buffer_line(const float(&cart)[MARLIN_2_XYZE], double fr_mm_s, int active_extruder)
{

  // create the target position
  firmware_position target;
  target.x = cart[AxisEnum::X_AXIS];
  target.y = cart[AxisEnum::Y_AXIS];
  target.z = cart[AxisEnum::Z_AXIS];
  target.e = cart[AxisEnum::E_AXIS];
  target.f = fr_mm_s;
  if (gcodes_.size() > 0)
  {
    gcodes_ += "\n";
  }
  // Generate the gcode
  gcodes_ += g1_command(target);

  return true;
}

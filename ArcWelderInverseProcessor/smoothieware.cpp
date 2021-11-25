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
#include "smoothieware.h"
#include "utilities.h"
smoothieware::smoothieware(firmware_arguments args) : firmware(args) {
  feed_rate = 0;
  for (int i = 0; i < REPETIER_XYZE; i++)
  {
    machine_position[i] = 0;
  }
  THEKERNEL = new SmoothiewareKernel();
  apply_arguments();
};


void smoothieware::apply_arguments()
{
  static const std::vector<std::string> smoothieware_firmware_version_names{ "2021-06-19" };
  set_versions(smoothieware_firmware_version_names, "2021-06-19");
  smoothieware_version_ = (smoothieware::smoothieware_firmware_versions)version_index_;
  std::vector<std::string> used_arguments;
  // Add switch back in if we ever add more versions
  //switch (smoothieware_version_)
  //{
  //default:
    append_arc_ = &smoothieware::append_arc_2021_06_19;
    used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT,  FIRMWARE_ARGUMENT_MM_MAX_ARC_ERROR, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
    //break;
  //}
  
  args_.set_used_arguments(used_arguments);

}

smoothieware::~smoothieware()
{
    delete THEKERNEL;
}

firmware_arguments smoothieware::get_default_arguments_for_current_version() const
{
  // Start off with the current args so they are set up correctly for this firmware type and version
    firmware_arguments default_args = args_;
  // firmware defaults
  default_args.g90_g91_influences_extruder = true;
  // Add the switch back if we ever need to add more versions
  //switch (smoothieware_version_)
  //{
  //default:
    // Active Settings
    default_args.mm_per_arc_segment = 0.0f;
    default_args.mm_max_arc_error = 0.01;
    default_args.n_arc_correction = 5;
    //break;
  //}
  return default_args;
}

std::string smoothieware::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
  // Clear the current list of gcodes
  gcodes_.clear();

  // Setup the current position
  machine_position[X_AXIS] = static_cast<float>(position_.x);
  machine_position[Y_AXIS] = static_cast<float>(position_.y);
  machine_position[Z_AXIS] = static_cast<float>(position_.z);
  machine_position[E_AXIS] = static_cast<float>(position_.e);
  float smoothieware_target[k_max_actuators];
  smoothieware_target[X_AXIS] = static_cast<float>(target.x);
  smoothieware_target[Y_AXIS] = static_cast<float>(target.y);
  smoothieware_target[Z_AXIS] = static_cast<float>(target.z);
  smoothieware_target[E_AXIS] = static_cast<float>(target.e);
  float smoothieware_offset[2];
  smoothieware_offset[0] = static_cast<float>(i);
  smoothieware_offset[1] = static_cast<float>(j);
  float radius = static_cast<float>(r);
  

  // Set the feedrate
  feed_rate = static_cast<float>(target.f);
  uint8_t smoothieware_isclockwise = is_clockwise ? 1 : 0;

  (this->*append_arc_)(&gcode_, smoothieware_target, smoothieware_offset, radius, smoothieware_isclockwise);

  return gcodes_;
}
// Append an arc to the queue ( cutting it into segments as needed )
bool smoothieware::append_arc_2021_06_19(SmoothiewareGcode* gcode, const float target[], const float offset[], float radius, bool is_clockwise)
{
  float rate_mm_s = this->feed_rate / seconds_per_minute;
  // catch negative or zero feed rates and return the same error as GRBL does
  if (rate_mm_s <= 0.0F) {
    gcode->is_error = true;
    gcode->txt_after_ok = (rate_mm_s == 0 ? "Undefined feed rate" : "feed rate < 0");
    return false;
  }

  // Scary math.
  float center_axis0 = this->machine_position[this->plane_axis_0] + offset[this->plane_axis_0];
  float center_axis1 = this->machine_position[this->plane_axis_1] + offset[this->plane_axis_1];
  float linear_travel = target[this->plane_axis_2] - this->machine_position[this->plane_axis_2];
  float r_axis0 = -offset[this->plane_axis_0]; // Radius vector from center to start position
  float r_axis1 = -offset[this->plane_axis_1];
  float rt_axis0 = target[this->plane_axis_0] - this->machine_position[this->plane_axis_0] - offset[this->plane_axis_0]; // Radius vector from center to target position
  float rt_axis1 = target[this->plane_axis_1] - this->machine_position[this->plane_axis_1] - offset[this->plane_axis_1];
  float angular_travel = 0;
  //check for condition where atan2 formula will fail due to everything canceling out exactly
  if ((this->machine_position[this->plane_axis_0] == target[this->plane_axis_0]) && (this->machine_position[this->plane_axis_1] == target[this->plane_axis_1])) {
    if (is_clockwise) { // set angular_travel to -2pi for a clockwise full circle
      angular_travel = (-2 * PI_FLOAT);
    }
    else { // set angular_travel to 2pi for a counterclockwise full circle
      angular_travel = (2 * PI_FLOAT);
    }
  }
  else {
    // Patch from GRBL Firmware - Christoph Baumann 04072015
    // CCW angle between position and target from circle center. Only one atan2() trig computation required.
    // Only run if not a full circle or angular travel will incorrectly result in 0.0f
    angular_travel = (float)utilities::atan2((double)r_axis0 * rt_axis1 - (double)r_axis1 * rt_axis0, (double)r_axis0 * rt_axis0 + (double)r_axis1 * rt_axis1);
    if (plane_axis_2 == Y_AXIS) { is_clockwise = !is_clockwise; }  //Math for XZ plane is reverse of other 2 planes
    if (is_clockwise) { // adjust angular_travel to be in the range of -2pi to 0 for clockwise arcs
      if (angular_travel > 0) { angular_travel -= (2 * PI_FLOAT); }
    }
    else {  // adjust angular_travel to be in the range of 0 to 2pi for counterclockwise arcs
      if (angular_travel < 0) { angular_travel += (2 * PI_FLOAT); }
    }
  }

  // initialize linear travel for ABC
#if SMOOTHIEWARE_MAX_ROBOT_ACTUATORS > 3
  float abc_travel[n_motors - 3];
  for (int i = A_AXIS; i < n_motors; i++) {
    abc_travel[i - 3] = target[i] - this->machine_position[i];
  }
#endif


  // Find the distance for this gcode
  float millimeters_of_travel = (float)utilities::hypot((double)angular_travel * radius, utilities::fabsf(linear_travel));

  // We don't care about non-XYZ moves ( for example the extruder produces some of those )
  if (millimeters_of_travel < 0.000001F) {
    return false;
  }

  // limit segments by maximum arc error
  float arc_segment = (float)args_.mm_per_arc_segment;
  if ((args_.mm_max_arc_error > 0) && (2.0 * (double)radius > args_.mm_max_arc_error)) {
    float min_err_segment = 2 * (float)utilities::sqrt((args_.mm_max_arc_error * (2.0 * (double)radius - args_.mm_max_arc_error)));
    if (args_.mm_per_arc_segment < min_err_segment) {
      arc_segment = min_err_segment;
    }
  }

  // catch fall through on above
  if (arc_segment < 0.0001F) {
    arc_segment = 0.5F; /// the old default, so we avoid the divide by zero
  }

  // Figure out how many segments for this gcode
  // TODO for deltas we need to make sure we are at least as many segments as requested, also if mm_per_line_segment is set we need to use the
  uint16_t segments = (uint16_t)utilities::floorf(millimeters_of_travel / arc_segment);
  bool moved = false;

  if (segments > 1) {
    float theta_per_segment = angular_travel / segments;
    float linear_per_segment = linear_travel / segments;
#if SMOOTHIEWARE_MAX_ROBOT_ACTUATORS > 3
    float abc_per_segment[n_motors - 3];
    for (int i = 0; i < n_motors - 3; i++) {
      abc_per_segment[i] = abc_travel[i] / segments;
    }
#endif

    /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
    and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
    r_T = [cos(phi) -sin(phi);
    sin(phi) cos(phi] * r ;
    For arc generation, the center of the circle is the axis of rotation and the radius vector is
    defined from the circle center to the initial position. Each line segment is formed by successive
    vector rotations. This requires only two cos() and sin() computations to form the rotation
    matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
    all float numbers are single precision on the Arduino. (True float precision will not have
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
    float cos_T = 1 - 0.5F * theta_per_segment * theta_per_segment; // Small angle approximation
    float sin_T = theta_per_segment;

    float arc_target[n_motors];
    float sin_Ti;
    float cos_Ti;
    float r_axisi;
    uint16_t i;
    int8_t count = 0;

    // init array for all axis
    utilities::memcpy(arc_target, machine_position, n_motors * sizeof(float));

    // Initialize the linear axis
    arc_target[this->plane_axis_2] = this->machine_position[this->plane_axis_2];

    for (i = 1; i < segments; i++) { // Increment (segments-1)
      if (THEKERNEL->is_halted()) return false; // don't queue any more segments

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
        r_axis0 = -offset[this->plane_axis_0] * cos_Ti + offset[this->plane_axis_1] * sin_Ti;
        r_axis1 = -offset[this->plane_axis_0] * sin_Ti - offset[this->plane_axis_1] * cos_Ti;
        count = 0;
      }

      // Update arc_target location
      arc_target[this->plane_axis_0] = center_axis0 + r_axis0;
      arc_target[this->plane_axis_1] = center_axis1 + r_axis1;
      arc_target[this->plane_axis_2] += linear_per_segment;
#if SMOOTHIEWARE_MAX_ROBOT_ACTUATORS > 3
      for (int a = A_AXIS; a < n_motors; a++) {
        arc_target[a] += abc_per_segment[a - 3];
      }
#endif

      // Append this segment to the queue
      bool b = this->append_milestone(arc_target, rate_mm_s);
      moved = moved || b;
    }
  }

  // Ensure last segment arrives at target location.
  if (this->append_milestone(target, rate_mm_s)) moved = true;

  return moved;
}

bool smoothieware::append_milestone(const float target[], double rate_mm_s)
{
  double rate_mm_min = rate_mm_s * 60;
  // create the target position
  firmware_position gcode_target;
  gcode_target.x = target[AxisEnum::X_AXIS];
  gcode_target.y = target[AxisEnum::Y_AXIS];
  gcode_target.z = target[AxisEnum::Z_AXIS];
  gcode_target.e = target[AxisEnum::E_AXIS];
  gcode_target.f = rate_mm_min;
  if (gcodes_.size() > 0)
  {
    gcodes_ += "\n";
  }
  // Generate the gcode
  gcodes_ += g1_command(gcode_target);

  return true;
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Inverse Processor Console Application
//
// Converts G2/G3(arc) commands back to G0/G1 commands.  Intended to test firmware changes to improve arc support.
// This reduces file size and the number of gcodes per second.
//
// Built using the 'Arc Welder: Anti Stutter' library
//
// Copyright(C) 2020 - Brad Hochgesang
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

// This file includes portions from Marlin's motion_control.c file since it is intended to test some firmware modifications.
// This file was included in the AntiStutter project for convenience, and will not be included within the final version.
/*
  motion_control.c - high level interface for issuing motion commands
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011 Sungeun K. Jeon
  Copyright (c) 2020 Brad Hochgesang

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "inverse_processor.h"
#include <cmath>

//#include "Marlin.h"
//#include "stepper.h"
//#include "planner.h"

inverse_processor::inverse_processor(std::string source_path, std::string target_path, bool g90_g91_influences_extruder, int buffer_size, ConfigurationStore cs_)
{
    source_path_ = source_path;
    target_path_ = target_path;
    p_source_position_ = new gcode_position(get_args_(g90_g91_influences_extruder, buffer_size));
    cs = cs_;
    // ** Gloabal Variable Definition **
    // 20200417 - FormerLurker - Declare two globals and pre-calculate some values that will reduce the
    // amount of trig funcitons we need to call while printing.  For the price of having two globals we
    // save one trig calc per G2/G3 for both MIN_ARC_SEGMENTS and MIN_MM_PER_ARC_SEGMENT.  This is a good trade IMHO.

    
#ifdef MIN_ARC_SEGMENTS
// Determines the radius at which the transition from using MM_PER_ARC_SEGMENT to MIN_ARC_SEGMENTS
    arc_max_radius_threshold = MM_PER_ARC_SEGMENT / (2.0F * sin(M_PI / MIN_ARC_SEGMENTS));
#endif
/*
#if defined(MIN_ARC_SEGMENTS) && defined(MIN_MM_PER_ARC_SEGMENT)
    // Determines the radius at which the transition from using MIN_ARC_SEGMENTS to MIN_MM_PER_ARC_SEGMENT.
    arc_min_radius_threshold = MIN_MM_PER_ARC_SEGMENT / (2.0F * sin(M_PI / MIN_ARC_SEGMENTS));
#endif
*/
}

gcode_position_args inverse_processor::get_args_(bool g90_g91_influences_extruder, int buffer_size)
{
    gcode_position_args args;
    // Configure gcode_position_args
    args.g90_influences_extruder = g90_g91_influences_extruder;
    args.position_buffer_size = buffer_size;
    args.autodetect_position = true;
    args.home_x = 0;
    args.home_x_none = true;
    args.home_y = 0;
    args.home_y_none = true;
    args.home_z = 0;
    args.home_z_none = true;
    args.shared_extruder = true;
    args.zero_based_extruder = true;


    args.default_extruder = 0;
    args.xyz_axis_default_mode = "absolute";
    args.e_axis_default_mode = "absolute";
    args.units_default = "millimeters";
    args.location_detection_commands = std::vector<std::string>();
    args.is_bound_ = false;
    args.is_circular_bed = false;
    args.x_min = -9999;
    args.x_max = 9999;
    args.y_min = -9999;
    args.y_max = 9999;
    args.z_min = -9999;
    args.z_max = 9999;
    return args;
}

inverse_processor::~inverse_processor()
{
    delete p_source_position_;
}

void inverse_processor::process()
{
    // Create a stringstream we can use for messaging.
    std::stringstream stream;

    int read_lines_before_clock_check = 5000;
    //std::cout << "stabilization::process_file - Processing file.\r\n";
    stream << "Decompressing gcode file.";
    stream << "Source File: " << source_path_ << "\n";
    stream << "Target File: " << target_path_ << "\n";
    std::cout << stream.str();
    const clock_t start_clock = clock();

    // Create the source file read stream and target write stream
    std::ifstream gcode_file;
    gcode_file.open(source_path_.c_str());
    output_file_.open(target_path_.c_str());
    std::string line;
    int lines_with_no_commands = 0;
    gcode_file.sync_with_stdio(false);
    output_file_.sync_with_stdio(false);
    gcode_parser parser;
    int gcodes_processed = 0;
    if (gcode_file.is_open())
    {
        if (output_file_.is_open())
        {
            //stream.clear();
            //stream.str("");
            //stream << "Opened file for reading.  File Size: " << file_size_ << "\n";
            //std::cout << stream.str();
            parsed_command cmd;
            // Communicate every second
            while (std::getline(gcode_file, line))
            {
                lines_processed_++;

                cmd.clear();
                parser.try_parse_gcode(line.c_str(), cmd);
                bool has_gcode = false;
                if (cmd.gcode.length() > 0)
                {
                    has_gcode = true;
                    gcodes_processed++;
                }
                else
                {
                    lines_with_no_commands++;
                }

                p_source_position_->update(cmd, lines_processed_, gcodes_processed, -1);

                if (cmd.command == "G2" || cmd.command == "G3")
                {
                    position* p_cur_pos = p_source_position_->get_current_position_ptr();
                    position* p_pre_pos = p_source_position_->get_previous_position_ptr();
                    float position[4];
                    position[X_AXIS] = static_cast<float>(p_pre_pos->get_gcode_x());
                    position[Y_AXIS] = static_cast<float>(p_pre_pos->get_gcode_y());
                    position[Z_AXIS] = static_cast<float>(p_pre_pos->get_gcode_z());
                    position[E_AXIS] = static_cast<float>(p_pre_pos->get_current_extruder().get_offset_e());
                    float target[4];
                    target[X_AXIS] = static_cast<float>(p_cur_pos->get_gcode_x());
                    target[Y_AXIS] = static_cast<float>(p_cur_pos->get_gcode_y());
                    target[Z_AXIS] = static_cast<float>(p_cur_pos->get_gcode_z());
                    target[E_AXIS] = static_cast<float>(p_cur_pos->get_current_extruder().get_offset_e());
                    float offset[2];
                    offset[0] = 0.0;
                    offset[1] = 0.0;
                    for (unsigned int index = 0; index < cmd.parameters.size(); index++)
                    {
                        parsed_command_parameter p = cmd.parameters[index];
                        if (p.name == "I")
                        {
                            offset[0] = static_cast<float>(p.double_value);
                        }
                        else if (p.name == "J")
                        {
                            offset[1] = static_cast<float>(p.double_value);
                        }
                    }
                    float radius = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc
                    uint8_t isclockwise = cmd.command == "G2" ? 1 : 0;
                    output_relative_ = p_cur_pos->is_extruder_relative;
                    switch (cs.interpolation_function)
                    {
                      case InterpolationFunction::INT_SEGMENTS:
                        mc_arc_int_segments(position, target, offset, static_cast<float>(p_cur_pos->f), radius, isclockwise, 0);
                        break;
                      case InterpolationFunction::FLOAT_SEGMENTS:
                        mc_arc_float_segments(position, target, offset, static_cast<float>(p_cur_pos->f), radius, isclockwise, 0);
                    }
                    
                }
                else
                {
                    output_file_ << line << "\n";
                }

            }
            output_file_.close();
        }
        else
        {
            std::cout << "Unable to open the output file for writing.\n";
        }
        std::cout << "Closing the input file.\n";
        gcode_file.close();
    }
    else
    {
        std::cout << "Unable to open the gcode file for processing.\n";
    }

    const clock_t end_clock = clock();
    const double total_seconds = (static_cast<double>(end_clock) - static_cast<double>(start_clock)) / CLOCKS_PER_SEC;
    
    stream.clear();
    stream.str("");
    stream << "Completed file processing\r\n";
    stream << "\tLines Processed      : " << lines_processed_ << "\r\n";
    stream << "\tTotal Seconds        : " << total_seconds << "\r\n";
    stream << "\tExtra Trig Count     : " << trig_calc_count << "\r\n";
    stream << "\tTotal E Adjustment   : " << total_e_adjustment << "\r\n";
    std::cout << stream.str();
}

// The arc is approximated by generating a huge number of tiny, linear segments. The length of each 
// segment is configured in settings.mm_per_arc_segment.  
void inverse_processor::mc_arc_int_segments(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
{
  float r_axis_x = -offset[X_AXIS];  // Radius vector from center to current location
  float r_axis_y = -offset[Y_AXIS];
  float center_axis_x = position[X_AXIS] - r_axis_x;
  float center_axis_y = position[Y_AXIS] - r_axis_y;
  float travel_z = target[Z_AXIS] - position[Z_AXIS];
  float rt_x = target[X_AXIS] - center_axis_x;
  float rt_y = target[Y_AXIS] - center_axis_y;
  // 20200419 - Add a variable that will be used to hold the arc segment length
  float mm_per_arc_segment = cs.mm_per_arc_segment;
  // 20210109 - Add a variable to hold the n_arc_correction value
  uint8_t n_arc_correction = cs.n_arc_correction;

  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel_total = atan2(r_axis_x * rt_y - r_axis_y * rt_x, r_axis_x * rt_x + r_axis_y * rt_y);
  if (angular_travel_total < 0) { angular_travel_total += 2 * M_PI; }

  if (cs.min_arc_segments > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    // Do this before converting the angular travel for clockwise rotation
    mm_per_arc_segment = radius * ((2.0f * M_PI) / cs.min_arc_segments);
  }
  if (cs.arc_segments_per_sec > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    float mm_per_arc_segment_sec = (feed_rate / 60.0f) * (1.0f / cs.arc_segments_per_sec);
    if (mm_per_arc_segment_sec < mm_per_arc_segment)
      mm_per_arc_segment = mm_per_arc_segment_sec;
  }

  // Note:  no need to check to see if min_mm_per_arc_segment is enabled or not (i.e. = 0), since mm_per_arc_segment can never be below 0.
  if (mm_per_arc_segment < cs.min_mm_per_arc_segment)
  {
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    // This prevents a very high number of segments from being generated for curves of a short radius
    mm_per_arc_segment = cs.min_mm_per_arc_segment;
  }
  else if (mm_per_arc_segment > cs.mm_per_arc_segment) {
    // 20210113 - This can be implemented in an else if since  we can't be below the min AND above the max at the same time.
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    mm_per_arc_segment = cs.mm_per_arc_segment;
  }

  // Adjust the angular travel if the direction is clockwise
  if (isclockwise) { angular_travel_total -= 2.0f * M_PI; }

  //20141002:full circle for G03 did not work, e.g. G03 X80 Y80 I20 J0 F2000 is giving an Angle of zero so head is not moving
  //to compensate when start pos = target pos && angle is zero -> angle = 2Pi
  if (position[X_AXIS] == target[X_AXIS] && position[Y_AXIS] == target[Y_AXIS] && angular_travel_total == 0)
  {
    angular_travel_total += 2 * M_PI;
  }
  //end fix G03

  // 20200417 - FormerLurker - rename millimeters_of_travel to millimeters_of_travel_arc to better describe what we are
  // calculating here
  const float millimeters_of_travel_arc = hypot(angular_travel_total * radius, fabs(travel_z));
  if (millimeters_of_travel_arc < 0.001f) { return; }
  // Calculate the total travel per segment
  // Calculate the number of arc segments as a float.  This is important so that the extrusion
  // and z travel is consistant throughout the arc.  Otherwise we will see artifacts and gaps
  uint16_t segments = static_cast<uint16_t>(ceil(millimeters_of_travel_arc / mm_per_arc_segment));

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
  //std::cout << "Generating arc with " << segments << " segments.  Extruding " << target[E_AXIS] - position[E_AXIS] << "mm.\n";
  // There must be more than 1 segment, else this is just a linear move
  float interpolatedExtrusion = 0;
  float interpolatedTravel = 0;
  float totalExtrusion = target[E_AXIS] - position[E_AXIS];
  if (segments > 1)
  {
    
    // Calculate theta per segments and linear (z) travel per segment
    const float theta_per_segment = angular_travel_total / segments,
      linear_per_segment = travel_z / (segments),
      segment_extruder_travel = (target[E_AXIS] - position[E_AXIS]) / (segments),
      sq_theta_per_segment = theta_per_segment * theta_per_segment,
      sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6,
      cos_T = 1 - 0.5f * sq_theta_per_segment;
    //  Calculate the number of interpolations we will do, but use the ceil 
    // function so that we can start our index with I=1.  This is important
    // so that the arc correction starts out with segment 1, and not 0, without
    // doing extra calculations
    
    for (uint16_t i = 1; i < segments; i++) { // Increment (segments-1)
      if (n_arc_correction-- == 0) {
        // Calculate the actual position for r_axis_x and r_axis_y
        const float cos_Ti = cos((i) * theta_per_segment), sin_Ti = sin((i) * theta_per_segment);
        r_axis_x = -offset[X_AXIS] * cos_Ti + offset[Y_AXIS] * sin_Ti;
        r_axis_y = -offset[X_AXIS] * sin_Ti - offset[Y_AXIS] * cos_Ti;
        // reset n_arc_correction
        n_arc_correction = cs.n_arc_correction;
      }
      else {
        const float r_axisi = r_axis_x * sin_T + r_axis_y * cos_T;
        r_axis_x = r_axis_x * cos_T - r_axis_y * sin_T;
        r_axis_y = r_axisi;
      }
      interpolatedExtrusion += segment_extruder_travel;
      interpolatedTravel += hypot(position[X_AXIS] - (center_axis_x + r_axis_x), position[Y_AXIS] - (center_axis_y + r_axis_y));
      // Update arc_target location
      position[X_AXIS] = center_axis_x + r_axis_x;
      position[Y_AXIS] = center_axis_y + r_axis_y;
      position[Z_AXIS] += linear_per_segment;
      position[E_AXIS] += segment_extruder_travel;
      // We can't clamp to the target because we are interpolating!  We would need to update a position, clamp to it
      // after updating from calculated values.
      clamp_to_software_endstops(position);
      
      plan_buffer_line(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS], feed_rate, extruder);
    }
  }
  // Ensure last segment arrives at target location.
  // Here we could clamp, but why bother.  We would need to update our current position, clamp to it
  clamp_to_software_endstops(target);
  if (segments > 1)
  {
    float fil_per_mm_interpolated = interpolatedTravel == 0 ? 0 : interpolatedExtrusion / interpolatedTravel;
    float mm_travel_final = hypot(target[X_AXIS] - position[X_AXIS], target[Y_AXIS] - position[Y_AXIS]);
    float extrusion_final = target[E_AXIS] - position[E_AXIS];
    float fil_per_mm_final = mm_travel_final == 0 ? 0 : extrusion_final / mm_travel_final;
    float fil_per_mm_difference = fabs(fil_per_mm_interpolated - fil_per_mm_final);
    //if (mm_travel_final > 0.001 && totalExtrusion - (interpolatedExtrusion + extrusion_final) > 0.00001f && max_extrusion_rate_difference < fil_per_mm_difference)
    if (mm_travel_final > 0.001 && (totalExtrusion - interpolatedExtrusion) > 0.00001f && max_extrusion_rate_difference < fil_per_mm_difference)
    {
      max_extrusion_rate_difference = fil_per_mm_difference;
      std::cout << "New Maximum extrusion rate difference detected:" << max_extrusion_rate_difference << "\n";
    }
  }

  plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feed_rate, extruder);

}


// The arc is approximated by generating a huge number of tiny, linear segments. The length of each 
// segment is configured in settings.mm_per_arc_segment.  
void inverse_processor::mc_arc_float_segments(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
{
  float r_axis_x = -offset[X_AXIS];  // Radius vector from center to current location
  float r_axis_y = -offset[Y_AXIS];
  float center_axis_x = position[X_AXIS] - r_axis_x;
  float center_axis_y = position[Y_AXIS] - r_axis_y;
  float travel_z = target[Z_AXIS] - position[Z_AXIS];
  float rt_x = target[X_AXIS] - center_axis_x;
  float rt_y = target[Y_AXIS] - center_axis_y;
  // 20200419 - Add a variable that will be used to hold the arc segment length
  float mm_per_arc_segment = cs.mm_per_arc_segment;
  // 20210109 - Add a variable to hold the n_arc_correction value
  uint8_t n_arc_correction = cs.n_arc_correction;

  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel_total = atan2(r_axis_x * rt_y - r_axis_y * rt_x, r_axis_x * rt_x + r_axis_y * rt_y);
  if (angular_travel_total < 0) { angular_travel_total += 2 * M_PI; }

  if (cs.min_arc_segments > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    // Do this before converting the angular travel for clockwise rotation
    mm_per_arc_segment = radius * ((2.0f * M_PI) / cs.min_arc_segments);
  }
  if (cs.arc_segments_per_sec > 0)
  {
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    float mm_per_arc_segment_sec = (feed_rate / 60.0f) * (1.0f / cs.arc_segments_per_sec);
    if (mm_per_arc_segment_sec < mm_per_arc_segment)
      mm_per_arc_segment = mm_per_arc_segment_sec;
  }

  // Note:  no need to check to see if min_mm_per_arc_segment is enabled or not (i.e. = 0), since mm_per_arc_segment can never be below 0.
  if (mm_per_arc_segment < cs.min_mm_per_arc_segment)
  {
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    // This prevents a very high number of segments from being generated for curves of a short radius
    mm_per_arc_segment = cs.min_mm_per_arc_segment;
  }
  else if (mm_per_arc_segment > cs.mm_per_arc_segment) {
    // 20210113 - This can be implemented in an else if since  we can't be below the min AND above the max at the same time.
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    mm_per_arc_segment = cs.mm_per_arc_segment;
  }

  // Adjust the angular travel if the direction is clockwise
  if (isclockwise) { angular_travel_total -= 2 * M_PI; }

  //20141002:full circle for G03 did not work, e.g. G03 X80 Y80 I20 J0 F2000 is giving an Angle of zero so head is not moving
  //to compensate when start pos = target pos && angle is zero -> angle = 2Pi
  if (position[X_AXIS] == target[X_AXIS] && position[Y_AXIS] == target[Y_AXIS] && angular_travel_total == 0)
  {
    angular_travel_total += 2 * M_PI;
  }
  //end fix G03

  // 20200417 - FormerLurker - rename millimeters_of_travel to millimeters_of_travel_arc to better describe what we are
  // calculating here
  const float millimeters_of_travel_arc = hypot(angular_travel_total * radius, fabs(travel_z));
  if (millimeters_of_travel_arc < 0.001) { return; }
  // Calculate the total travel per segment
  // Calculate the number of arc segments as a float.  This is important so that the extrusion
  // and z travel is consistant throughout the arc.  Otherwise we will see artifacts and gaps
  float segments = millimeters_of_travel_arc / mm_per_arc_segment;

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
  float interpolatedExtrusion = 0;
  float interpolatedTravel = 0;
  float totalExtrusion = target[E_AXIS] - position[E_AXIS];
  // There must be more than 1 segment, else this is just a linear move
  if (segments > 1)
  {
    // Calculate theta per segments and linear (z) travel per segment
    const float theta_per_segment = angular_travel_total / segments,
      linear_per_segment = travel_z / (segments),
      segment_extruder_travel = (target[E_AXIS] - position[E_AXIS]) / (segments),
      sq_theta_per_segment = theta_per_segment * theta_per_segment,
      //sin_T = theta_per_segment - sq_theta_per_segment * theta_per_segment / 6,
      //cos_T = 1 - 0.5f * sq_theta_per_segment;
      sin_T = sin(theta_per_segment),
      cos_T = cos(theta_per_segment);
    //  Calculate the number of interpolations we will do, but use the ceil 
    // function so that we can start our index with I=1.  This is important
    // so that the arc correction starts out with segment 1, and not 0, without
    // doing extra calculations
    uint16_t num_interpolations = static_cast<uint16_t>(ceil(segments));
    for (uint16_t i = 1; i < num_interpolations; i++) { // Increment (segments-1)
      /*if (n_arc_correction-- == 0) {
        // Calculate the actual position for r_axis_x and r_axis_y
        const float cos_Ti = cos((i)*theta_per_segment), sin_Ti = sin((i)*theta_per_segment);
        r_axis_x = -offset[X_AXIS] * cos_Ti + offset[Y_AXIS] * sin_Ti;
        r_axis_y = -offset[X_AXIS] * sin_Ti - offset[Y_AXIS] * cos_Ti;
        // reset n_arc_correction
        n_arc_correction = cs.n_arc_correction;
      }
      else { */
        const float r_axisi = r_axis_x * sin_T + r_axis_y * cos_T;
        r_axis_x = r_axis_x * cos_T - r_axis_y * sin_T;
        r_axis_y = r_axisi;
      //}
      interpolatedExtrusion += segment_extruder_travel; 
      interpolatedTravel += hypot(position[X_AXIS] - (center_axis_x + r_axis_x), position[Y_AXIS] - (center_axis_y + r_axis_y));
      // Update arc_target location
      position[X_AXIS] = center_axis_x + r_axis_x;
      position[Y_AXIS] = center_axis_y + r_axis_y;
      position[Z_AXIS] += linear_per_segment;
      position[E_AXIS] += segment_extruder_travel;
      // We can't clamp to the target because we are interpolating!  We would need to update a position, clamp to it
      // after updating from calculated values.
      clamp_to_software_endstops(position);
      plan_buffer_line(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[E_AXIS], feed_rate, extruder);
    }
  }
  // Ensure last segment arrives at target location.
  // Here we could clamp, but why bother.  We would need to update our current position, clamp to it
  clamp_to_software_endstops(target);
  if (segments > 1)
  {
    float fil_per_mm_interpolated = interpolatedTravel == 0 ? 0 : interpolatedExtrusion / interpolatedTravel;
    float mm_travel_final = hypot(target[X_AXIS] - position[X_AXIS], target[Y_AXIS] - position[Y_AXIS]);
    float extrusion_final = target[E_AXIS] - position[E_AXIS];
    float fil_per_mm_final = mm_travel_final == 0 ? 0 : extrusion_final / mm_travel_final;
    float fil_per_mm_difference = fabs(fil_per_mm_interpolated - fil_per_mm_final);
    if (segments > 1 && mm_travel_final > 0.001 && totalExtrusion - (interpolatedExtrusion + extrusion_final) > 0.00001f && max_extrusion_rate_difference < fil_per_mm_difference)
    {
      max_extrusion_rate_difference = fil_per_mm_difference;
      std::cout << "New Maximum extrusion rate difference detected:" << max_extrusion_rate_difference << "\n";
    }
  }
  
  plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feed_rate, extruder);

}

void inverse_processor::clamp_to_software_endstops(float* target)
{
    // Do nothing, just added to keep mc_arc identical to the firmware version
    return;
}

void inverse_processor::plan_buffer_line(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target)
{
    std::stringstream stream;
    stream << std::fixed;
    bool has_z = z > 0;
    
    //output_file_ <<

    stream << "G1 X" << std::setprecision(3) << x << " Y" << y;
    if (has_z)
    {
      stream << " Z" << z;
    }
    stream << std::setprecision(5) << " E" << e;
    
    stream << std::setprecision(0) << " F" << feed_rate << "\n";
    output_file_ << stream.str();
}

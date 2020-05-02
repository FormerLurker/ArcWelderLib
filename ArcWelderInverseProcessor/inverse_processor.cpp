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
#include "math.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

//#include "Marlin.h"
//#include "stepper.h"
//#include "planner.h"

inverse_processor::inverse_processor(std::string source_path, std::string target_path, bool g90_g91_influences_extruder, int buffer_size)
{
    source_path_ = source_path;
    target_path_ = target_path;
    p_source_position_ = new gcode_position(get_args_(g90_g91_influences_extruder, buffer_size));
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
    std::ofstream output_file;
    gcode_file.open(source_path_.c_str());
    output_file.open(target_path_.c_str());
    std::string line;
    int lines_with_no_commands = 0;
    gcode_file.sync_with_stdio(false);
    output_file.sync_with_stdio(false);
    gcode_parser parser;
    int lines_processed = 0;
    int gcodes_processed = 0;
    if (gcode_file.is_open())
    {
        if (output_file.is_open())
        {
            //stream.clear();
            //stream.str("");
            //stream << "Opened file for reading.  File Size: " << file_size_ << "\n";
            //std::cout << stream.str();
            parsed_command cmd;
            // Communicate every second
            while (std::getline(gcode_file, line))
            {
                lines_processed++;

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

                p_source_position_->update(cmd, lines_processed, gcodes_processed, -1);

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
                    output_file << mc_arc(position, target, offset, X_AXIS, Y_AXIS, Z_AXIS, static_cast<float>(p_cur_pos->f), radius, isclockwise, 0, p_cur_pos->is_extruder_relative) << "\n";
                }
                else
                {
                    output_file << line << "\n";
                }

            }
            output_file.close();
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
    stream << "\tLines Processed      : " << lines_processed << "\r\n";
    stream << "\tTotal Seconds        : " << total_seconds << "\r\n";
    stream << "\tExtra Trig Count     : " << trig_calc_count << "\r\n";
    stream << "\tTotal E Adjustment   : " << total_e_adjustment << "\r\n";
    std::cout << stream.str();
}

// The arc is approximated by generating a huge number of tiny, linear segments. The length of each 
// segment is configured in settings.mm_per_arc_segment.  
std::string inverse_processor::mc_arc(float* position, float* target, float* offset, uint8_t axis_0, uint8_t axis_1,
    uint8_t axis_linear, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder, bool output_relative)
{
    std::stringstream stream;
    stream << std::fixed;
    std::string gcodes;
    
    // Start Modifications
    float center_axis0 = position[axis_0] + offset[axis_0];
    float center_axis1 = position[axis_1] + offset[axis_1];
    float linear_travel = target[axis_linear] - position[axis_linear];
    float extruder_travel_total = target[E_AXIS] - position[E_AXIS];
    float r_axis0 = -offset[axis_0];  // Radius vector from center to current location
    float r_axis1 = -offset[axis_1];
    float rt_axis0 = target[axis_0] - center_axis0;
    float rt_axis1 = target[axis_1] - center_axis1;
    // 20200419 - Add a variable that will be used to hold the arc segment length
    float mm_per_arc_segment = MM_PER_ARC_SEGMENT;

    // CCW angle between position and target from circle center. Only one atan2() trig computation required.
    float angular_travel_total = atan2(r_axis0 * rt_axis1 - r_axis1 * rt_axis0, r_axis0 * rt_axis0 + r_axis1 * rt_axis1);
    if (angular_travel_total < 0) { angular_travel_total += 2 * M_PI; }

#ifdef MIN_ARC_SEGMENTS
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    // Do this before converting the angular travel for clockwise rotation
    if (radius < arc_max_radius_threshold) mm_per_arc_segment = radius * ((2.0f * M_PI) / MIN_ARC_SEGMENTS);
#endif

#ifdef ARC_SEGMENTS_PER_SEC
    // 20200417 - FormerLurker - Implement MIN_ARC_SEGMENTS if it is defined - from Marlin 2.0 implementation
    float mm_per_arc_segment_sec = (feed_rate / 60.0f) * (1.0f / ARC_SEGMENTS_PER_SEC);
    if (mm_per_arc_segment_sec < mm_per_arc_segment) 
        mm_per_arc_segment = mm_per_arc_segment_sec;
#endif

#ifdef MIN_MM_PER_ARC_SEGMENT
    // 20200417 - FormerLurker - Implement MIN_MM_PER_ARC_SEGMENT if it is defined
    // This prevents a very high number of segments from being generated for curves of a short radius
    if (mm_per_arc_segment < MIN_MM_PER_ARC_SEGMENT)  mm_per_arc_segment = MIN_MM_PER_ARC_SEGMENT;
#endif

#if defined(MIN_MM_PER_ARC_SEGMENT) || defined(ARC_SEGMENTS_PER_SEC) || defined(MIN_ARC_SEGMENTS)
    if (mm_per_arc_segment > MM_PER_ARC_SEGMENT)
        mm_per_arc_segment = MM_PER_ARC_SEGMENT;
#endif


    // Adjust the angular travel if the direction is clockwise
    if (isclockwise) { angular_travel_total -= 2 * M_PI; }

    //20141002:full circle for G03 did not work, e.g. G03 X80 Y80 I20 J0 F2000 is giving an Angle of zero so head is not moving
    //to compensate when start pos = target pos && angle is zero -> angle = 2Pi
    if (position[axis_0] == target[axis_0] && position[axis_1] == target[axis_1] && angular_travel_total == 0)
    {
        angular_travel_total += 2 * M_PI;
    }
    //end fix G03

    // 20200417 - FormerLurker - rename millimeters_of_travel to millimeters_of_travel_arc to better describe what we are
    // calculating here
    float millimeters_of_travel_arc = hypot(angular_travel_total * radius, fabs(linear_travel));
    if (millimeters_of_travel_arc < 0.001) { return ""; }
    // Calculate the total travel per segment
    // Calculate the number of arc segments
    uint16_t segments = static_cast<uint16_t>(ceil(millimeters_of_travel_arc / mm_per_arc_segment));
    // Ensure at least one segment
    //if (segments < 1) segments = 1;

    // Calculate theta per segments and linear (z) travel per segment
    float theta_per_segment = angular_travel_total / segments;
    float linear_per_segment = linear_travel / (segments);

#ifdef ARC_EXTRUSION_CORRECTION
    // 20200417 - FormerLurker - The feedrate needs to be adjusted becaue the perimeter of a regular polygon is always
    // less than that of a circumscribed circle.  However, after testing it has been determined that this
    // value is very small and may not be worth the clock cycles unless the settings are vastlyl different than the 
    // defaults

    // Calculate the individual segment arc and chord length
    float segment_length_arc = millimeters_of_travel_arc / segments;
    float segment_length_chord = 2.0f * radius * sin(fabs(theta_per_segment) * 0.5f); // This is a costly calculation..
    // Determine the correction factor
    float extrusion_correction_factor = fabs(segment_length_chord / segment_length_arc);
    // Calculate the corrected extrusion amount per segment
    float segment_extruder_travel = (extruder_travel_total / segments) * extrusion_correction_factor;
#else
    // Calculate the extrusion amount per segment
    float segment_extruder_travel = extruder_travel_total / (segments);
#endif

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

    // The initial approximation causes irregular movements in some cases, causing back travel to the final segment.
    // Initialize the linear axis
    float arc_target[4];
    arc_target[axis_linear] = position[axis_linear];

    // Initialize the extruder axis
    arc_target[E_AXIS] = position[E_AXIS];

    // Don't bother calculating cot_T or sin_T if there is only 1 segment.  This will speed up arcs that only have
    // 1 segment.
    if (segments > 1)
    {
        float cos_T;
        float sin_T;
        // 20200417 - FormerLurker - Using the small angle approximation causes drift if theta is large.
        // Use true cos/sin if the angle is large (do we need a definition for this?)
        if (theta_per_segment < (2.0f * M_PI / 16.0f) && theta_per_segment >(-2.0f * M_PI / 16.0f))
        {
            // Avoids cos and sin calculations.  However, in my testing this doesn't save much time
            // since the majority of the cost is adding the segments to the planner.  If possible,
            // I believe it's better to reduce the number of segments as much as possible, even if it
            // means a bit more overhead in this function.  However, for small angles this works fine
            // and is very fast.
            cos_T = 1.0f - 0.5f * theta_per_segment * theta_per_segment; // Small angle approximation
            sin_T = theta_per_segment;
        }
        else
        {
            // This seems to work even without N_ARC_CORRECTION enabled for all values I tested. It produces 
            // extremely accurate segment endpoints even when an extremely high number of segments are 
            // generated. With 3 decimals of precision on XYZ, this should work for any reasonable settings 
            // without correction.
            cos_T = cos(theta_per_segment);
            sin_T = sin(theta_per_segment);
        }
        float sin_Ti;
        float cos_Ti;
        float r_axisi;
        uint16_t i;
        int8_t count = 0;

        for (i = 1; i < segments; i++) { // Increment (segments-1)

#ifdef N_ARC_CORRECTION
// 20200417 - FormerLurker - Make N_ARC_CORRECTION optional.
            if (count < N_ARC_CORRECTION) {
#endif
                // Apply vector rotation matrix 
                float x_0 = r_axis0;
                float y_0 = r_axis1;
                r_axisi = r_axis0 * sin_T + r_axis1 * cos_T;
                r_axis0 = r_axis0 * cos_T - r_axis1 * sin_T;
                r_axis1 = r_axisi;
#ifdef N_ARC_CORRECTION
                // 20200417 - FormerLurker - Make N_ARC_CORRECTION optional.
                count++;
            }
            else {
                // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
                // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
                cos_Ti = cos(i * theta_per_segment);
                sin_Ti = sin(i * theta_per_segment);
                r_axis0 = -offset[axis_0] * cos_Ti + offset[axis_1] * sin_Ti;
                r_axis1 = -offset[axis_0] * sin_Ti - offset[axis_1] * cos_Ti;
                count = 0;
            }
#endif
            // Update arc_target location
            arc_target[axis_0] = center_axis0 + r_axis0;
            arc_target[axis_1] = center_axis1 + r_axis1;
            arc_target[axis_linear] += linear_per_segment;
            arc_target[E_AXIS] += segment_extruder_travel;

            //********** TODO:  MOVE THIS TO ANOTHER FUNCITON AND USE THE ORIGINAL C AS MUCH AS POSSIBLE
            if (stream.tellp() > 1U)
                stream << "\n";

            stream << "G1 X" << std::setprecision(3) << arc_target[X_AXIS] << " Y" << arc_target[Y_AXIS];
            if (output_relative)
            {
                stream << std::setprecision(5) << " E" << segment_extruder_travel;
            }
            else
            {
                stream << std::setprecision(5) << " E" << arc_target[E_AXIS];
            }

            stream << std::setprecision(0) << " F" << feed_rate;
            //********** END TODO
        }
    }

#ifdef ARC_EXTRUSION_CORRECTION
    // 20200417 - FormerLurker - adjust the final absolute e coordinate based on our extruder correction
    target[E_AXIS] = arc_target[E_AXIS] + segment_extruder_travel;
#endif
    // Ensure last segment arrives at target location.
    if (stream.tellp() > 1U)
        stream << "\n";

    stream << "G1 X" << std::setprecision(3) << target[X_AXIS] << " Y" << target[Y_AXIS];
    if (output_relative)
    {
        stream << std::setprecision(5) << " E" << segment_extruder_travel;
    }
    else
    {
        stream << std::setprecision(5) << " E" << target[E_AXIS];
    }
    stream << std::setprecision(0) << " F" << feed_rate;
#ifdef ARC_EXTRUSION_CORRECTION
    // 20200417 - FormerLurker - Hide the e axis corrections from the planner
    // Is this necessary, and is this the prefered way to accomplish this?
    //plan_set_e_position(target[E_AXIS]);
#endif
    return stream.str();

    //   plan_set_acceleration_manager_enabled(acceleration_manager_was_enabled);
}


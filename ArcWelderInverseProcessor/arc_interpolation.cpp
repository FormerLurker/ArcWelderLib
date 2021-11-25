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

#include "arc_interpolation.h"
#include <string>
#include "gcode_position.h"
#include "marlin_1.h"
#include "marlin_2.h"
#include "repetier.h"
#include "prusa.h"
#include "smoothieware.h"
#include "utilities.h"


gcode_position_args arc_interpolation::get_args_(bool g90_g91_influences_extruder, int buffer_size)
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

arc_interpolation::arc_interpolation()
{
  p_current_firmware_ = NULL;
}

arc_interpolation::arc_interpolation(arc_interpolation_args args) 
{
  args_ = args;
  switch (args.firmware_args.firmware_type)
  {
    case firmware_types::MARLIN_1:
      p_current_firmware_ = new marlin_1(args.firmware_args);
      break;
    case firmware_types::MARLIN_2:
      p_current_firmware_ = new marlin_2(args.firmware_args);
      break;
    case firmware_types::REPETIER:
      p_current_firmware_ = new repetier(args.firmware_args);
      break;
    case firmware_types::PRUSA:
      p_current_firmware_ = new prusa(args.firmware_args);
      break;
    case firmware_types::SMOOTHIEWARE:
      p_current_firmware_ = new smoothieware(args.firmware_args);
  }
  // Initialize the source position
  p_source_position_ = new gcode_position(get_args_(p_current_firmware_->get_g90_g91_influences_extruder(), DEFAULT_GCODE_BUFFER_SIZE));
}

arc_interpolation::~arc_interpolation()
{
  delete p_source_position_;
  if (p_current_firmware_ != NULL)
  {
    delete p_current_firmware_;
  }
 
}

void arc_interpolation::process()
{
  // Create a stringstream we can use for messaging.
  std::stringstream stream;

  // Create a current and target position variable for arc processing
  firmware_state state;
  firmware_position current;
  firmware_position target;
  // I, J, and R parameter values
  double i=0, j=0, r=0;
  // bool values for is_clockwise and is_relative
  bool is_clockwise = false, is_relative = false;
  // e absolute offset, in case we are outputting absolute e
  double offset_absolute_e = 0;

  int read_lines_before_clock_check = 5000;
  //std::cout << "stabilization::process_file - Processing file.\r\n";
  stream << "Decompressing gcode file.";
  stream << "Source File: " << args_.source_path << "\n";
  stream << "Target File: " << args_.target_path << "\n";
  std::cout << stream.str();
  const clock_t start_clock = clock();

  // Create the source file read stream and target write stream
  std::ifstream gcode_file;
  gcode_file.open(args_.source_path.c_str());
  output_file_.open(args_.target_path.c_str());
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
      // Add the gcode file header
        output_file_ << p_current_firmware_->get_gcode_header_comment()<<"\n";
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
          // increment the number of arc commands encountered
          num_arc_commands_++;
          // Get the current and previous positions
          position* p_cur_pos = p_source_position_->get_current_position_ptr();
          position* p_pre_pos = p_source_position_->get_previous_position_ptr();
          // create the current and target positions
          current.x = p_pre_pos->get_gcode_x();
          current.y = p_pre_pos->get_gcode_y();
          current.z = p_pre_pos->get_gcode_z();
          current.e = p_pre_pos->get_current_extruder().get_offset_e();
          current.f = p_pre_pos->f;
          // set the current firmware position
          p_current_firmware_->set_current_position(current);

          target.x = p_cur_pos->get_gcode_x();
          target.y = p_cur_pos->get_gcode_y();
          target.z = p_cur_pos->get_gcode_z();
          target.e = p_cur_pos->get_current_extruder().get_offset_e();
          target.f = p_cur_pos->f;

          state.is_extruder_relative = p_pre_pos->is_extruder_relative;
          state.is_relative = p_pre_pos->is_relative;
          // set the current firmware state
          p_current_firmware_->set_current_state(state);
          
          // get I, J, and R
          i = 0;
          j = 0;
          r = 0;
          for (unsigned int index = 0; index < cmd.parameters.size(); index++)
          {
            parsed_command_parameter p = cmd.parameters[index];
            if (p.name == "I")
            {
              i = p.double_value;
            }
            else if (p.name == "J")
            {
              j = p.double_value;
            }
            else if (p.name == "R")
            {
              r = p.double_value;
            }
          }

          // If r is 0, calculate the radius
          if(r==0)
          {
            r = utilities::hypot(i, j);
          }
          
          is_clockwise = cmd.command == "G2" ? 1 : 0;
          is_relative = p_cur_pos->is_extruder_relative;
          offset_absolute_e = p_pre_pos->get_current_extruder().get_offset_e();

          // run the callback and capture any created gcode commands
          std::string gcodes = p_current_firmware_->interpolate_arc(target, i, j, r, is_clockwise);
          if (gcodes.length() > 0)
          {
            // there are gcodes to write, write them!
            output_file_ << gcodes << "\n";
          }
        }
        else
        {
          // Nothing to do with the current line, just write it to disk.
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
  stream << "\tLines Processed       : " << lines_processed_ << "\r\n";
  stream << "\tArc Commands Processed: " << num_arc_commands_ << "\r\n";
  stream << "\tArc Segments Generated: " << p_current_firmware_->get_num_arc_segments_generated() << "\r\n";
  stream << "\tTotal Seconds         : " << total_seconds << "\r\n";
  std::cout << stream.str();
}

std::string arc_interpolation::get_firmware_arguments_description(std::string separator, std::string argument_prefix, std::string replacement_string, std::string replacement_value) const
{
  return p_current_firmware_->get_arguments_description(separator, argument_prefix, replacement_string, replacement_value);
}
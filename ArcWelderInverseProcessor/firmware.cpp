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

#include "firmware.h"
#include "utilities.h"

firmware::firmware() {
  version_index_ = -1;
  num_arc_segments_generated_ = 0;
};

firmware::firmware(firmware_arguments args) : args_(args) {
  version_index_ = -1;
  num_arc_segments_generated_ = 0;
};

std::string firmware::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
  throw "Function not yet implemented";
}

void firmware::apply_arguments()
{
  throw "Function not yet implemented";
}

void firmware::set_current_position(firmware_position& position)
{
  position_ = position;
}

void firmware::set_current_state(firmware_state& state)
{
  state_ = state;
}

int firmware::get_num_arc_segments_generated()
{
  return num_arc_segments_generated_;
}

std::string firmware::g1_command(firmware_position& target)
{
  num_arc_segments_generated_++;
  std::string gcode = "G1 ";
  gcode.reserve(96);

  bool has_x = position_.x != target.x;
  bool has_y = position_.y != target.y;
  bool has_z = position_.z != target.z;
  bool has_e = position_.e != target.e;
  bool has_f = position_.f != target.f;
  bool is_first_parameter = true;
  if (has_x)
  {
    gcode += is_first_parameter ? "X" : " X";
    gcode += utilities::dtos(state_.is_relative ? target.x - position_.x : target.x, 3);
    is_first_parameter = false;
  }

  if (has_y)
  {
    gcode += is_first_parameter ? "Y" : " Y";
    gcode += utilities::dtos(state_.is_relative ? target.y - position_.y : target.y, 3);
    is_first_parameter = false;
  }

  if (has_z)
  {
    gcode += is_first_parameter ? "Z" : " Z";
    gcode += utilities::dtos(state_.is_relative ? target.z - position_.z : target.z, 3);
    is_first_parameter = false;
  }

  if (has_e)
  {
    gcode += is_first_parameter ? "E" : " E";
    gcode += utilities::dtos(state_.is_extruder_relative ? target.e - position_.e : target.e, 5);
    is_first_parameter = false;
  }

  if (has_f)
  {
    gcode += is_first_parameter ? "F" : " F";
    gcode += utilities::dtos(target.f, 0);
  }
  
  return gcode;
}

bool firmware::is_valid_version(std::string version)
{
  if (version == LATEST_FIRMWARE_VERSION_NAME)
  {
    return true;
  }
  for (std::vector<std::string>::const_iterator i = version_names_.begin(); i != version_names_.end(); ++i) {
    // process i
    if (*i == version)
    {
      return true;
    }
  }
  return false;
}

std::vector<std::string> firmware::get_version_names()
{
  return version_names_;
}

std::string firmware::get_version_names_string()
{
    std::vector<std::string> version_names_with_release_info;
    bool foundLatestRelease = false;
    for (int index = 0; index < version_names_.size(); index++)
    {
        std::string version_name = version_names_[index];
        
        if (foundLatestRelease)
        {
            version_name.append(" (").append("NON_RELEASE_VERSION").append(")");
        }
        else if (version_name == args_.latest_release_version)
        {
            version_name.append(" (").append(LATEST_FIRMWARE_VERSION_NAME).append(")");
            foundLatestRelease = true;
        }
        version_names_with_release_info.push_back(version_name);
    }
    return utilities::join(version_names_with_release_info, ",");
}

bool firmware::get_g90_g91_influences_extruder()
{
  return args_.g90_g91_influences_extruder;
}

std::string firmware::get_arguments_description(std::string separator, std::string argument_prefix, std::string replacement_string, std::string replacement_value) {
  
  return args_.get_arguments_description(separator, argument_prefix, replacement_string, replacement_value);
}

std::string firmware::get_gcode_header_comment()
{
    return args_.get_gcode_header_comment();
}

void firmware::set_versions(std::vector<std::string> version_names, std::string latest_release_version_name)
{
  args_.latest_release_version = latest_release_version_name;
  std::string requested_version = args_.version;
  if (requested_version == LATEST_FIRMWARE_VERSION_NAME || args_.version == latest_release_version_name)
  {
    requested_version = latest_release_version_name;
  }
  version_index_ = (int)version_names.size() - 1;
  for (int i = 0; i < version_names.size(); i++)
  {
    std::string version = version_names[i];
    version_names_.push_back(version);
    if (version == requested_version)
    {
      version_index_ = i;
    }
  }
}

firmware_arguments firmware::get_default_arguments_for_current_version() const
{
  return firmware_arguments();
}

void firmware::set_arguments(firmware_arguments args)
{
  args_ = args;
  this->apply_arguments();
}

firmware_arguments firmware::arguments_changed(firmware_arguments current_args, firmware_arguments new_args)
{
  return new_args;
}
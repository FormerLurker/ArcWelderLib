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
#include "firmware_types.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utilities.h>
#include "version.h"

#define DEFAULT_FIRMWARE_TYPE firmware_types::MARLIN_2
#define LATEST_FIRMWARE_VERSION_NAME "LATEST_RELEASE"
#define DEFAULT_FIRMWARE_VERSION_NAME LATEST_FIRMWARE_VERSION_NAME
// Arc interpretation settings:
#define DEFAULT_MM_PER_ARC_SEGMENT 0 // REQUIRED - The enforced maximum length of an arc segment
#define DEFAULT_ARC_SEGMENTS_PER_R 0; // 0 to disable
#define DEFAULT_MAX_MM_PER_ARC_SEGMENT 0 // Required - The enforced maximum length of an arc segment
#define DEFAULT_MIN_ARC_SEGMENT_MM 0; // 0 to disable
#define DEFAULT_MIN_MM_PER_ARC_SEGMENT 0 /* OPTIONAL - the enforced minimum length of an interpolated segment.  Must be smaller than
  MM_PER_ARC_SEGMENT.  Only has an effect if MIN_ARC_SEGMENTS > 0 or ARC_SEGMENTS_PER_SEC > 0 */
  // If both MIN_ARC_SEGMENTS and ARC_SEGMENTS_PER_SEC is defined, the minimum calculated segment length is used.  Set to 0 to disable
#define DEFAULT_MIN_ARC_SEGMENTS 0 // OPTIONAL - The enforced minimum segments in a full circle of the same radius.  Set to 0 to disable.
#define DEFAULT_MIN_CIRCLE_SEGMENTS 0 // OPTIONAL - The enforced minimum segments in a full circle of the same radius.
#define DEFAULT_ARC_SEGMENTS_PER_SEC 0 // OPTIONAL - Use feedrate to choose segment length.
// approximation will not be used for the first segment.  Subsequent segments will be corrected following DEFAULT_N_ARC_CORRECTION.  Set to 0 to disable.
#define DEFAULT_N_ARC_CORRECTIONS 0
// This setting is for the gcode position processor to help interpret G90/G91 behavior
#define DEFAULT_G90_G91_INFLUENCES_EXTRUDER false
// This currently is only used in Smoothieware.   The maximum error for line segments that divide arcs.  Set to 0 to disable.
#define DEFAULT_MM_MAX_ARC_ERROR 0

struct firmware_state {
  firmware_state() {
    is_relative = false;
    is_extruder_relative = false;
  }
  bool is_relative;
  bool is_extruder_relative;
};

struct firmware_position {
  firmware_position() {
    x = 0;
    y = 0;
    z = 0;
    e = 0;
    f = 0;
  }
  double x;
  double y;
  double z;
  double e;
  double f;
};

// parameter name defines
#define FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT "mm_per_arc_segment"
#define FIRMWARE_ARGUMENT_ARC_SEGMENT_PER_R "arc_segments_per_r"
#define FIRMWARE_ARGUMENT_MIN_MM_PER_ARC_SEGMENT "min_mm_per_arc_segment"
#define FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS "min_arc_segments"
#define FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC "arc_segments_per_sec"
#define FIRMWARE_ARGUMENT_N_ARC_CORRECTION "n_arc_correction"
#define FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER "g90_g91_influences_extruder"
#define FIRMWARE_ARGUMENT_MM_MAX_ARC_ERROR "mm_max_arc_error"
#define FIRMWARE_ARGUMENT_MIN_CIRCLE_SEGMENTS "min_circle_segments"
#define FIRMWARE_ARGUMENT_MIN_ARC_SEGMENT_MM "min_arc_segment_mm"
#define FIRMWARE_ARGUMENT_MAX_ARC_SEGMENT_MM "max_arc_segment_mm"

struct firmware_arguments {
public:
    
    firmware_arguments() {
    mm_per_arc_segment = DEFAULT_MM_PER_ARC_SEGMENT;
    min_arc_segment_mm = DEFAULT_MIN_ARC_SEGMENT_MM;
    max_arc_segment_mm = DEFAULT_MAX_MM_PER_ARC_SEGMENT;
    arc_segments_per_r = DEFAULT_ARC_SEGMENTS_PER_R;
    min_mm_per_arc_segment = DEFAULT_MIN_MM_PER_ARC_SEGMENT;
    min_circle_segments = DEFAULT_MIN_CIRCLE_SEGMENTS;
    min_arc_segments = DEFAULT_MIN_ARC_SEGMENTS;
    arc_segments_per_sec = DEFAULT_ARC_SEGMENTS_PER_SEC;
    n_arc_correction = DEFAULT_N_ARC_CORRECTIONS;
    g90_g91_influences_extruder = DEFAULT_G90_G91_INFLUENCES_EXTRUDER;
    mm_max_arc_error = DEFAULT_MM_MAX_ARC_ERROR;
    version = DEFAULT_FIRMWARE_VERSION_NAME;
    firmware_type = (firmware_types)DEFAULT_FIRMWARE_TYPE;
    latest_release_version = LATEST_FIRMWARE_VERSION_NAME;

    // add a list of all possible arguments, including aliases
    all_arguments_.clear();
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_ARC_SEGMENT_PER_R);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MIN_MM_PER_ARC_SEGMENT);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_N_ARC_CORRECTION);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MM_MAX_ARC_ERROR);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MIN_CIRCLE_SEGMENTS);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MIN_ARC_SEGMENT_MM);
    all_arguments_.push_back(FIRMWARE_ARGUMENT_MAX_ARC_SEGMENT_MM);
  };

  /// <summary>
  /// The maximum mm per arc segment.
  /// </summary>
  double mm_per_arc_segment;
  /// <summary>
  /// The maximum segment length
  /// </summary>
  double arc_segments_per_r;
  /// <summary>
  /// The minimum mm per arc segment.  If less than or equal to 0, this is disabled
  /// </summary>
  double min_mm_per_arc_segment;
  /// <summary>
  /// The minimum mm per arc segment.  If less than or equal to 0, this is disabled
  /// </summary>
  double min_arc_segment_mm;
  /// <summary>
  /// The maximum mm per arc segment.
  /// </summary>
  double max_arc_segment_mm;


  /// <summary>
  /// The number of arc segments that will be drawn per second based on the given feedrate.  
  /// If less than or equal to zero, this is disabled.
  /// </summary>
  
  double arc_segments_per_sec;
  /// <summary>
  /// This currently is only used in Smoothieware.   The maximum error for line segments that divide arcs.  Set to 0 to disable.
  /// </summary>
  double mm_max_arc_error;
  /// <summary>
  /// The minimum number of arc segments in a full circle of the arc's radius.
  /// If less than or equal to zero, this is disabled
  /// </summary>
  int min_arc_segments;

  /// <summary>
  /// The minimum number of arc segments in a full circle of the arc's radius.
  /// If less than or equal to zero, this is disabled
  /// </summary>
  int min_circle_segments;
  /// <summary>
  /// // Number of interpolated segments before true sin and cos corrections will be applied.  
  /// If less than or equal to zero, true sin and cos will always be used.
  /// </summary>
  int n_arc_correction;
  /// <summary>
  /// This value will set the behavior of G90/G91.
  /// </summary>
  bool g90_g91_influences_extruder;

  /// <summary>
  /// The type of firmware to use when interpolating.
  /// </summary>
  firmware_types firmware_type;
  /// <summary>
  /// The firmware version to use.  Defaults to LATEST
  /// </summary>
  std::string version;
  /// <summary>
  /// True if the current version is the latest release.  For informational purposes only
  /// </summary>
  std::string latest_release_version;
  
  void set_used_arguments(std::vector<std::string> arguments)
  {
    used_arguments_ = arguments;
  }

  std::vector<std::string> get_unused_arguments()
  {
    std::vector<std::string> unused_arguments;
    for (std::vector<std::string>::iterator it = all_arguments_.begin(); it != all_arguments_.end(); it++)
    {
      if (!is_argument_used(*it))
      {
        unused_arguments.push_back(*it);
      }
    }
    return unused_arguments;
  }

  std::string get_unused_arguments_string(std::string separator = "", std::string argument_prefix = "", std::string replacement_string = "", std::string replacement_value = "")
  {
      return get_arguments_string_(get_unused_arguments(), separator, argument_prefix, replacement_string, replacement_value);
    std::string unusaed_argument_string = "";
    std::vector<std::string> unused_argumnts = get_unused_arguments();
    for (std::vector<std::string>::iterator it = unused_argumnts.begin(); it != unused_argumnts.end(); it++)
    {
      if (unusaed_argument_string.size() > 0)
      {
        unusaed_argument_string += separator;
      }
      unusaed_argument_string += *it;
    }
    return unusaed_argument_string;
  }

  std::string get_available_arguments_string(std::string separator = "", std::string argument_prefix = "", std::string replacement_string = "", std::string replacement_value = "")
  {
      return get_arguments_string_(used_arguments_, separator, argument_prefix, replacement_string, replacement_value);
  }

  std::vector<std::string> get_available_arguments()
  {
    return used_arguments_;
  }
  std::string get_gcode_header_comment()
  {
      std::string comment_start = "; ";
      std::stringstream stream;
      stream << comment_start << "Postprocessed by [ArcStraightener](https://github.com/FormerLurker/ArcWelderLib)\n";
      stream << comment_start << "Copyright(C) " << COPYRIGHT_DATE << " - " << AUTHOR << "\n";
      stream << comment_start << "Version: " << GIT_TAGGED_VERSION << ", Branch: " << GIT_BRANCH << ", BuildDate: " << BUILD_DATE << "\n";

      stream << comment_start << "firmware_type=" << firmware_type_names[firmware_type] << "\n";
      stream << comment_start << "firmware_version=" << (version == LATEST_FIRMWARE_VERSION_NAME || version == latest_release_version ? latest_release_version + " (" + LATEST_FIRMWARE_VERSION_NAME + ")" : version) << "\n";
      
      stream << std::fixed << std::setprecision(0);
      std::string argument_string;

      // Bool values
      argument_string = FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << (g90_g91_influences_extruder ? "True" : "False") << "\n";
      }

      // Int values
      argument_string = FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << min_arc_segments << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_MIN_CIRCLE_SEGMENTS;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << min_circle_segments << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_N_ARC_CORRECTION;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << n_arc_correction << "\n";
      }

      stream << std::fixed << std::setprecision(2);
      // Double values
      argument_string = FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << mm_per_arc_segment << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_ARC_SEGMENT_PER_R;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << arc_segments_per_r << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << arc_segments_per_sec << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_MM_MAX_ARC_ERROR;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << mm_max_arc_error << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_MIN_MM_PER_ARC_SEGMENT;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << min_mm_per_arc_segment << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_MIN_ARC_SEGMENT_MM;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << min_arc_segment_mm << "\n";
      }
      argument_string = FIRMWARE_ARGUMENT_MAX_ARC_SEGMENT_MM;
      if (is_argument_used(argument_string))
      {
          stream << comment_start << argument_string << "=" << max_arc_segment_mm << "\n";
      }

      return stream.str();
  }

  std::string get_arguments_description(std::string separator="", std::string argument_prefix = "", std::string replacement_string = "", std::string replacement_value = "") {
    std::stringstream stream;
    stream << "Firmware Arguments:\n";
    stream << "\tFirmware Type               : " << firmware_type_names[firmware_type] << "\n";
    stream << "\tFirmware Version            : " << (version == LATEST_FIRMWARE_VERSION_NAME || version == latest_release_version ? latest_release_version + " (" + LATEST_FIRMWARE_VERSION_NAME + ")" : version) <<"\n";
    stream << std::fixed << std::setprecision(0);
    std::string argument_string;

    // Bool values
    argument_string = FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << " : " << (g90_g91_influences_extruder ? "True" : "False") << "\n";
    }
    
    // Int values
    argument_string = FIRMWARE_ARGUMENT_MIN_ARC_SEGMENTS;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "            : " << min_arc_segments << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_MIN_CIRCLE_SEGMENTS;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "         : " << min_circle_segments << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_N_ARC_CORRECTION;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "            : " << n_arc_correction << "\n";
    }

    stream << std::fixed << std::setprecision(2);
    // Double values
    argument_string = FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "          : " << mm_per_arc_segment << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_ARC_SEGMENT_PER_R;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "          : " << arc_segments_per_r << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_ARC_SEGMENTS_PER_SEC;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "        : " << arc_segments_per_sec << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_MM_MAX_ARC_ERROR;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "            : " << mm_max_arc_error << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_MIN_MM_PER_ARC_SEGMENT;
    if (is_argument_used(argument_string))
    {
        stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "      : " << min_mm_per_arc_segment << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_MIN_ARC_SEGMENT_MM;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "          : " << min_arc_segment_mm << "\n";
    }
    argument_string = FIRMWARE_ARGUMENT_MAX_ARC_SEGMENT_MM;
    if (is_argument_used(argument_string))
    {
      stream << "\t" << get_argument_string(argument_string, "", replacement_string, replacement_value) << "          : " << max_arc_segment_mm << "\n";
    }
    std::string unused_argument_string = get_unused_arguments_string(separator, argument_prefix, replacement_string, replacement_value);
    if (unused_argument_string.size() > 0)
    {
      stream << "The following parameters do not apply to this firmware version: " << unused_argument_string << "\n";
    }
    return stream.str();

  }
  bool is_argument_used(std::string argument_name)
  {
    return (std::find(used_arguments_.begin(), used_arguments_.end(), argument_name) != used_arguments_.end());
  }
  static std::string get_argument_string(std::string argument_name, std::string argument_prefix, std::string replacement_string = "", std::string replacement_value = "")
  {
      return argument_prefix + utilities::replace(argument_name, replacement_string, replacement_value);
  }

  private:
    std::vector<std::string> all_arguments_;
    std::vector<std::string> used_arguments_;
    std::string get_arguments_string_(std::vector<std::string> string_values, std::string separator, std::string argument_prefix, std::string replacement_string = "", std::string replacement_value = "")
    {
        std::string available_argument_string = "";

        for (std::vector<std::string>::iterator it = string_values.begin(); it != string_values.end(); it++)
        {
            if (available_argument_string.size() > 0)
            {
                available_argument_string += separator;
            }
            available_argument_string += get_argument_string(*it, argument_prefix , replacement_string, replacement_value);
        }
        return available_argument_string;
    }
    
    
};

class firmware
{
public:

  firmware();

  firmware(firmware_arguments args);

  /// <summary>
  /// Generate G1 gcode strings separated by line breaks representing the supplied G2/G3 command.
  /// </summary>
  /// <param name="current">The current printer position</param>
  /// <param name="target">The target printer position</param>
  /// <param name="i">Specifies the X offset for the arc's center.</param>
  /// <param name="j">Specifies the Y offset for the arc's center.</param>
  /// <param name="r">Specifies the radius of the arc.  If r is greater than 0, this will override the i and j parameters.</param>
  /// <param name="is_clockwise">If true, this is a G2 command.  If false, this is a G3 command.</param>
  /// <param name="is_relative">If this is true, the extruder is currently in relative mode.  Else it is in absolute mode.</param>
  /// <param name="offest_absolute_e">This is the absolute offset for absolute E coordinates if the extruder is not in relative mode.</param>
  /// <returns></returns>
  virtual std::string interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise);

  /// <summary>
  /// Sets the current position.  Should be called before interpolate_arc.
  /// </summary>
  /// <param name="position">The position to set</param>
  void set_current_position(firmware_position& position);

  /// <summary>
  /// Sets firmware offsets and the xyze axis mode.
  /// </summary>
  /// <param name="state">The state to set</param>
  void set_current_state(firmware_state& state);
  /// <summary>
  /// Create a G1 command from the current position and offsets.
  /// </summary>
  /// <param name="target">The position of the printer after the G1 command is completed.</param>
  /// <returns>The G1 command</returns>
  virtual std::string g1_command(firmware_position& target);

  /// <summary>
  /// Checks a string to see if it is a valid version.
  /// </summary>
  /// <param name="version">The version to check.</param>
  /// <returns>True if the supplied version is valid</returns>
  bool is_valid_version(std::string version);

  /// <summary>
  /// Returns all valid versions for this firmware.
  /// </summary>
  /// <returns>Vector of strings, one for each supported version</returns>
  std::vector<std::string> get_version_names();

  /// <summary>
  /// Returns all valid versions for this firmware in one comma separated string.
  /// </summary>
  /// <returns>Vector of strings, one for each supported version</returns>
  std::string get_version_names_string();

  /// <summary>
  /// Returns the current g90_g91_influences_extruder value for the firmware.
  /// </summary>
  /// <returns></returns>
  bool get_g90_g91_influences_extruder();

  /// <summary>
  /// Returns the number of arc segments that were generated from g2/g3 commands.
  /// </summary>
  /// <returns></returns>
  int get_num_arc_segments_generated();

  /// <summary>
  /// Outputs a string description of the firmware arguments.
  /// </summary>
  /// <returns></returns>
  std::string get_arguments_description(std::string separator , std::string argument_prefix = "", std::string replacement_string = "", std::string replacement_value = "");

  /// <summary>
  /// Returns a gcode comment containing the current settings
  /// </summary>
  /// <returns></returns>
  std::string get_gcode_header_comment();

  /// <summary>
  /// Sets all available versions names and the version index based on args_.version
  /// </summary>
  /// <returns></returns>
  void set_versions(std::vector<std::string> version_names, std::string latest_release_version_name);

  virtual firmware_arguments get_default_arguments_for_current_version() const;

  void set_arguments(firmware_arguments args);

  virtual void apply_arguments();

protected:
  firmware_position position_;
  firmware_state state_;
  firmware_arguments args_;
  std::vector<std::string> version_names_;
  int version_index_;
  int num_arc_segments_generated_;

  virtual firmware_arguments arguments_changed(firmware_arguments current_args, firmware_arguments new_args);
};

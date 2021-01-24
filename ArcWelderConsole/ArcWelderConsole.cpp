////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Anti-Stutter Console Application
//
// Compresses many G0/G1 commands into G2/G3(arc) commands where possible, ensuring the tool paths stay within the specified resolution.
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
#if _MSC_VER > 1200
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include "ArcWelderConsole.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "gcode_position.h"
#include <tclap/CmdLine.h>
#define DEFAULT_ARG_DOUBLE_PRECISION 4

int main(int argc, char* argv[])
{
  std::string source_file_path;
  std::string target_file_path;
  double resolution_mm;
  double max_radius_mm;
  int min_arc_segments;
  double mm_per_arc_segment;
  double path_tolerance_percent;
  bool g90_g91_influences_extruder;
  bool hide_progress;
  bool overwrite_source_file = false;
  bool allow_3d_arcs = false;
  bool allow_dynamic_precision = DEFAULT_ALLOW_DYNAMIC_PRECISION;
  unsigned char default_xyz_precision = DEFAULT_XYZ_PRECISION;
  unsigned char default_e_precision = DEFAULT_E_PRECISION;
  std::string log_level_string;
  std::string log_level_string_default = "INFO";
  int log_level_value;

  // Add info about the application   
  std::string info = "Arc Welder: Anti-Stutter - Reduces the number of gcodes per second sent to a 3D printer that supports arc commands (G2 G3).";
  // Add the current vesion information
  info.append("\nVersion: ").append(GIT_TAGGED_VERSION);
  info.append(", Branch: ").append(GIT_BRANCH);
  info.append(", BuildDate: ").append(BUILD_DATE);
  info.append("\n").append("Copyright(C) ").append(COPYRIGHT_DATE).append(" - ").append(AUTHOR);
  info.append("\n").append("An algorithm for producing fast floating point strings, fpconv, was added with the following notice:  Copyright (C) 2014 Milo Yip");
  info.append("\n").append("The original fpconv algorithm provides the following notice: Copyright(c) 2013 Andreas Samoljuk");
  
  std::stringstream arg_description_stream;
  
  arg_description_stream << std::fixed << std::setprecision(5);
  // Extract arguments
  try {
    // Define the command line object
    TCLAP::CmdLine cmd(info, '=', GIT_TAGGED_VERSION);

    // Define Arguments

    // <SOURCE>
    TCLAP::UnlabeledValueArg<std::string> source_arg("source", "The source gcode file to convert.", true, "", "path to source gcode file");

    // <TARGET>
    TCLAP::UnlabeledValueArg<std::string> target_arg("target", "The target gcode file containing the converted code.  If this is not supplied, the source path will be used and the source file will be overwritten.", false, "", "path to target gcode file");

    // -r --resolution-mm
    arg_description_stream << "The resolution in mm of the of the output.  Determines the maximum tool path deviation allowed during conversion. Default Value: " << DEFAULT_RESOLUTION_MM;
    TCLAP::ValueArg<double> resolution_arg("r", "resolution-mm", arg_description_stream.str(), false, DEFAULT_RESOLUTION_MM, "float");

    // -t --path-tolerance-percent
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "This is the maximum allowable difference between the arc path and the original toolpath.  Since most slicers use interpolation when generating arc moves, this value can be relatively high without impacting print quality.";
    arg_description_stream << "  Expressed as a decimal percent, where 0.05 = 5.0%. Default Value: " << ARC_LENGTH_PERCENT_TOLERANCE_DEFAULT;
    TCLAP::ValueArg<double> path_tolerance_percent_arg("t", "path-tolerance-percent", arg_description_stream.str(), false, DEFAULT_RESOLUTION_MM, "float");

    // -m --max-radius-mm
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The maximum radius of any arc in mm. Default Value: " << DEFAULT_MAX_RADIUS_MM;
    TCLAP::ValueArg<double> max_radius_arg("m", "max-radius-mm", arg_description_stream.str(), false, DEFAULT_MAX_RADIUS_MM, "float");


    // -s --mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The mm per arc segment as defined in your firmware.   Used to compensate for firmware without min-arc-segments setting.  Requires that min-arc-segments be set.  Default Value: " << DEFAULT_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> mm_per_arc_segment_arg("s", "mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MM_PER_ARC_SEGMENT, "float");

    // -a --min-arc-segments
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum number of segments in a full circle of the same radius as any given arc.  Can only be used if --mm-per-arc-segment is also set.  Used to compensate for firmware without min-arc-segments setting.  Default: " << DEFAULT_MIN_ARC_SEGMENTS;
    TCLAP::ValueArg<int> min_arc_segments_arg("a", "min-arc-segments", arg_description_stream.str(), false, DEFAULT_MIN_ARC_SEGMENTS, "int");

    // -g --g90-influences-extruder
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "If supplied, G90/G91 influences the extruder axis.  Default Value: " << DEFAULT_G90_G91_INFLUENCES_EXTRUDER;
    TCLAP::SwitchArg g90_arg("g", "g90-influences-extruder", arg_description_stream.str(), DEFAULT_G90_G91_INFLUENCES_EXTRUDER);

    // -z --allow-3d-arcs
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "(experimental) - If supplied, 3D arcs will be allowed (supports spiral vase mode).  Not all firmware supports this.  Default Value: " << DEFAULT_ALLOW_3D_ARCS;
    TCLAP::SwitchArg allow_3d_arcs_arg("z", "allow-3d-arcs", arg_description_stream.str(), DEFAULT_ALLOW_3D_ARCS);

    // -d --allow-dynamic-precision
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "If supplied, arcwelder will adjust the precision of the outputted gcode based on the precision of the input gcode.  Default Value: " << DEFAULT_ALLOW_DYNAMIC_PRECISION;
    TCLAP::SwitchArg allow_dynamic_precision_arg("d", "allow-dynamic-precision", arg_description_stream.str(), DEFAULT_ALLOW_DYNAMIC_PRECISION);

    // -x --default-xyz-precision
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The default precision of X, Y, Z, I and J output gcode parameters.  The precision may be larger than this value if allow-dynamic-precision is set to true.  Default Value: " << DEFAULT_XYZ_PRECISION;
    TCLAP::ValueArg<unsigned char> default_xyz_precision_arg("x", "default-xyz-precision", arg_description_stream.str(), false, DEFAULT_XYZ_PRECISION, "unsigned char");

    // -e --default-e-precision
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The default precision of E output gcode parameters.  The precision may be larger than this value if allow-dynamic-precision is set to true.  Default Value: " << DEFAULT_E_PRECISION;
    TCLAP::ValueArg<unsigned char> default_e_precision_arg("e", "default-e-precision", arg_description_stream.str(), false, DEFAULT_E_PRECISION, "unsigned char");

    // -g --hide-progress
    TCLAP::SwitchArg hide_progress_arg("p", "hide-progress", "If supplied, prevents progress updates from being displayed.", false);

    // -l --log-level
    std::vector<std::string> log_levels_vector;
    log_levels_vector.push_back("NOSET");
    log_levels_vector.push_back("VERBOSE");
    log_levels_vector.push_back("DEBUG");
    log_levels_vector.push_back("INFO");
    log_levels_vector.push_back("WARNING");
    log_levels_vector.push_back("ERROR");
    log_levels_vector.push_back("CRITICAL");
    log_levels_vector.push_back("");
    TCLAP::ValuesConstraint<std::string> log_levels_constraint(log_levels_vector);
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "Sets console log level. Default Value: " << log_level_string_default; 
    TCLAP::ValueArg<std::string> log_level_arg("l", "log-level", arg_description_stream.str(), false, log_level_string_default, &log_levels_constraint);

    // Add all arguments
    cmd.add(source_arg);
    cmd.add(target_arg);
    cmd.add(resolution_arg);
    cmd.add(path_tolerance_percent_arg);
    cmd.add(max_radius_arg);
    cmd.add(min_arc_segments_arg);
    cmd.add(mm_per_arc_segment_arg);
    cmd.add(allow_3d_arcs_arg);
    cmd.add(allow_dynamic_precision_arg);
    cmd.add(default_xyz_precision_arg);
    cmd.add(default_e_precision_arg);
    cmd.add(g90_arg);
    cmd.add(hide_progress_arg);
    cmd.add(log_level_arg);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg. 
    source_file_path = source_arg.getValue();
    target_file_path = target_arg.getValue();

    if (target_file_path.size() == 0)
    {
      target_file_path = source_file_path;
    }

    resolution_mm = resolution_arg.getValue();
    max_radius_mm = max_radius_arg.getValue();
    min_arc_segments = min_arc_segments_arg.getValue();
    mm_per_arc_segment = mm_per_arc_segment_arg.getValue();
    path_tolerance_percent = path_tolerance_percent_arg.getValue();
    allow_3d_arcs = allow_3d_arcs_arg.getValue();
    g90_g91_influences_extruder = g90_arg.getValue();
    allow_dynamic_precision = allow_dynamic_precision_arg.getValue();
    default_xyz_precision = default_xyz_precision_arg.getValue();
    default_e_precision = default_e_precision_arg.getValue();

    hide_progress = hide_progress_arg.getValue();
    log_level_string = log_level_arg.getValue();
    log_level_value = -1;

    // Check the entered values
    bool has_error = false;
    if (resolution_mm <= 0)
    {
      std::cerr << "error: The provided resolution of " << resolution_mm << " is negative, which is not allowed." <<std::endl;
      has_error = true;
    }
    
    if (path_tolerance_percent <= 0)
    {
      std::cerr << "error: The provided path tolerance percentage of " << path_tolerance_percent << " is negative, which is not allowed." << std::endl;
      has_error = true;
    }

    if (max_radius_mm > 1000000)
    {
      // warning
      std::cout << "warning: The provided path max radius of " << max_radius_mm << "mm is greater than 1000000 (1km), which is not recommended." << std::endl;
    }

    if (min_arc_segments < 0)
    {
      // warning
      std::cout << "warning: The provided min_arc_segments " << min_arc_segments << " is less than zero.  Setting to 0." << std::endl;
      min_arc_segments = 0;
    }

    if (mm_per_arc_segment < 0)
    {
      // warning
      std::cout << "warning: The provided mm_per_arc_segment " << mm_per_arc_segment << "mm is less than zero.  Setting to 0." << std::endl;
      mm_per_arc_segment = 0;
    }

    if (path_tolerance_percent > 0.05)
    {
      // warning
      std::cout << "warning: The provided path tolerance percent of " << path_tolerance_percent << " is greater than 0.05 (5%), which is not recommended." << std::endl;
    }
    else if (path_tolerance_percent < 0.0001 && path_tolerance_percent > 0)
    {
      // warning
      std::cout << "warning: The provided path tolerance percent of " << path_tolerance_percent << " is less than greater than 0.001 (0.1%), which is not recommended." << std::endl;
    }

    if (default_xyz_precision < 3)
    {
      // warning
      std::cout << "warning: The provided default_xyz_precision " << default_xyz_precision << "mm is less than 3, with will cause issues printing arcs.  A value of 3 will be used instead." << std::endl;
      default_xyz_precision = 3;
    }

    if (default_e_precision < DEFAULT_E_PRECISION)
    {
      // warning
      std::cout << "warning: The provided default_e_precision " << default_e_precision << "mm is less than 3, with will cause extrusion issues.  A value of 3 will be used instead." << std::endl;
      default_e_precision = 3;
    }

    if (default_xyz_precision > 6)
    {
      // warning
      std::cout << "warning: The provided default_xyz_precision " << default_xyz_precision << "mm is greater than 6, which may cause gcode checksum errors while printing depending on your firmeware, so a value of 6 will be used instead." << std::endl;
      default_xyz_precision = 6;
    }

    if (default_e_precision > 6)
    {
      // warning
      std::cout << "warning: The provided default_e_precision " << default_e_precision << "mm is greater than 6, which may cause gcode checksum errors while printing depending on your firmeware, so value of 6 will be used instead." << std::endl;
      default_e_precision = 6;
    }

    if (has_error)
    {
      return 1;
    }

    for (unsigned int log_name_index = 0; log_name_index < log_level_names_size; log_name_index++)
    {
      if (log_level_string == log_level_names[log_name_index])
      {
        log_level_value = log_level_values[log_name_index];
        break;
      }
    }
    if (log_level_value == -1)
    {
      // TODO:  Does this work?
      throw new TCLAP::ArgException("Unknown log level");
    }

  }
  // catch argument exceptions
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  // Ensure the log level name is valid
  
  std::vector<std::string> log_names;
  log_names.push_back("arc_welder.gcode_conversion");
  std::vector<int> log_levels;
  log_levels.push_back(log_levels::DEBUG);
  logger* p_logger = new logger(log_names, log_levels);
  p_logger->set_log_level_by_value(log_level_value);

  std::stringstream log_messages;
  std::string temp_file_path = "";
  log_messages << std::fixed << std::setprecision(DEFAULT_ARG_DOUBLE_PRECISION);
  if (source_file_path == target_file_path)
  {
    overwrite_source_file = true;
    if (!utilities::get_temp_file_path_for_file(source_file_path, temp_file_path))
    {
      log_messages << "The source and target path are the same, but a temporary file path could not be created.  Is the path empty?";
      p_logger->log(0, INFO, log_messages.str());
      log_messages.clear();
      log_messages.str("");
    }

    // create a uuid with a tmp extension for the temporary file
    log_messages << "Source and target path are the same.  The source file will be overwritten.  Temporary file path: " << temp_file_path;
    p_logger->log(0, INFO, log_messages.str());
    log_messages.clear();
    log_messages.str("");
    target_file_path = temp_file_path;
  }
  log_messages << "Processing Gcode\n";
  log_messages << "\tSource File Path             : " << source_file_path << "\n";
  if (overwrite_source_file)
  {
    log_messages << "\tTarget File Path (overwrite) : " << target_file_path << "\n";
    log_messages << "\tTemporary File Path          : " << temp_file_path << "\n";
  }
  else 
  {
    log_messages << "\tTarget File File             : " << target_file_path << "\n";
  }
  
  log_messages << "\tResolution                   : " << resolution_mm << "mm (+-" << std::setprecision(5) << resolution_mm/2.0 << "mm)\n";
  log_messages << "\tPath Tolerance               : " << std::setprecision(3) << path_tolerance_percent*100.0 << "%\n";
  log_messages << "\tMaximum Arc Radius           : " << std::setprecision(0) << max_radius_mm << "mm\n";
  log_messages << "\tMin Arc Segments             : " << std::setprecision(0) << min_arc_segments << "\n";
  log_messages << "\tMM Per Arc Segment           : " << std::setprecision(3) << mm_per_arc_segment << "\n";
  log_messages << "\tAllow 3D Arcs                : " << (allow_3d_arcs ? "True" : "False") << "\n";
  log_messages << "\tAllow Dynamic Precision      : " << (allow_dynamic_precision ? "True" : "False") << "\n";
  log_messages << "\tDefault XYZ Precision        : " << std::setprecision(0) << static_cast<int>(default_xyz_precision) << "\n";
  log_messages << "\tDefault E Precision          : " << std::setprecision(0) << static_cast<int>(default_e_precision) << "\n";
  log_messages << "\tG90/G91 Influences Extruder  : " << (g90_g91_influences_extruder ? "True" : "False") << "\n";
  log_messages << "\tLog Level                    : " << log_level_string << "\n";
  log_messages << "\tHide Progress Updates        : " << (hide_progress ? "True" : "False");
  p_logger->log(0, INFO, log_messages.str());
  arc_welder* p_arc_welder = NULL;

  if (overwrite_source_file)
  {
    target_file_path = temp_file_path;
  }
  if (!hide_progress)
    p_arc_welder = new arc_welder(source_file_path, target_file_path, p_logger, resolution_mm, path_tolerance_percent, max_radius_mm, min_arc_segments, mm_per_arc_segment, g90_g91_influences_extruder, allow_3d_arcs, allow_dynamic_precision, default_xyz_precision, default_e_precision,  DEFAULT_GCODE_BUFFER_SIZE, on_progress);
  else
    p_arc_welder = new arc_welder(source_file_path, target_file_path, p_logger, resolution_mm, path_tolerance_percent, max_radius_mm, min_arc_segments, mm_per_arc_segment, g90_g91_influences_extruder, allow_3d_arcs, allow_dynamic_precision, default_xyz_precision, default_e_precision, DEFAULT_GCODE_BUFFER_SIZE, suppress_progress);

  arc_welder_results results = p_arc_welder->process();
  if (results.success)
  {
    log_messages.clear();
    log_messages.str("");
    log_messages << "Target file at '" << target_file_path << "' created.";

    if (overwrite_source_file)
    {
      log_messages.clear();
      log_messages.str("");
      log_messages << "Deleting the original source file at '" << source_file_path << "'.";
      p_logger->log(0, INFO, log_messages.str());
      log_messages.clear();
      log_messages.str("");
      std::remove(source_file_path.c_str());
      log_messages << "Renaming temporary file at '" << target_file_path << "' to '" << source_file_path <<"'.";
      p_logger->log(0, INFO, log_messages.str());
      std::rename(target_file_path.c_str(), source_file_path.c_str());
    }
    log_messages.clear();
    log_messages.str("");
    log_messages << std::endl << results.progress.segment_statistics.str();
    p_logger->log(0, INFO, log_messages.str() );
    
    log_messages.clear();
    log_messages.str("");
    log_messages << "Arc Welder process completed successfully.";
    p_logger->log(0, INFO, log_messages.str());
  }
  else
  {
    log_messages.clear();
    log_messages.str("");
    log_messages << "File processing failed.";
    p_logger->log(0, INFO, log_messages.str());
  }

  delete p_arc_welder;
  return 0;
}

bool on_progress(arc_welder_progress progress, logger* p_logger, int logger_type)
{
  std::cout << "Progress: "<< progress.str() << std::endl;
  std::cout.flush();
  return true;
}
bool suppress_progress(arc_welder_progress progress, logger* p_logger, int logger_type)
{
  return true;
}


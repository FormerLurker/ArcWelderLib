////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Anti-Stutter Console Application
//
// Compresses many G0/G1 commands into G2/G3(arc) commands where possible, ensuring the tool paths stay within the specified resolution.
// This reduces file size and the number of gcodes per second.
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
#include <tclap/tclap_version.h>
#define DEFAULT_ARG_DOUBLE_PRECISION 4

#define PROGRESS_TYPE_NONE "NONE"
#define PROGRESS_TYPE_SIMPLE "SIMPLE"
#define PROGRESS_TYPE_FULL "FULL"
int main(int argc, char* argv[])
{
  arc_welder_args args;
  std::string log_level_string;
  std::string log_level_string_default = "INFO";
  std::string progress_type;
  int log_level_value;
  bool hide_progress = false;

  // Add info about the application
  std::string info = "Arc Welder: Anti-Stutter\nConverts G0/G1 commands to G2/G3 (arc) commands. Reduces the number of gcodes per second sent to a 3D printer, which can reduce stuttering.";
  // Add the current vesion information
  info.append("\nVersion: ").append(GIT_TAGGED_VERSION);
  info.append(", Branch: ").append(GIT_BRANCH);
  info.append(", BuildDate: ").append(BUILD_DATE);
  info.append("\n").append("Copyright(C) ").append(COPYRIGHT_DATE).append(" - ").append(AUTHOR);
  info.append("\n").append("Includes source code derived from fpconv. ").append(FPCONV_COPYRIGHT_STRING);
  info.append("\n").append("Includes TCLAP v").append(TCLAP_VERSION_STRING).append(". ").append(TCLAP_COPYRIGHT_STRING);
  
  std::stringstream arg_description_stream;
  
  arg_description_stream << std::fixed << std::setprecision(5);
  // Define the command line object
  TCLAP::CmdLine cmd(info, '=', GIT_TAGGED_VERSION);
  // Define Arguments
    // <SOURCE>
  TCLAP::UnlabeledValueArg<std::string> source_arg("source", "The source gcode file to convert.", true, "", "path to source gcode file");

  // <TARGET>
  TCLAP::UnlabeledValueArg<std::string> target_arg("target", "The target gcode file containing the converted code. If this is not supplied, the source path will be used and the source file will be overwritten.", false, "", "path to target gcode file");

  // -g --g90-influences-extruder
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "If supplied, G90/G91 influences the extruder axis. Default Value: " << DEFAULT_G90_G91_INFLUENCES_EXTRUDER;
  TCLAP::SwitchArg g90_arg("g", "g90-influences-extruder", arg_description_stream.str(), DEFAULT_G90_G91_INFLUENCES_EXTRUDER);

  // -r --resolution-mm
  arg_description_stream << "The resolution in mm of the of the output. Determines the maximum tool path deviation allowed during conversion. Restrictions: Only values greater than 0 are allowed. Default Value: " << DEFAULT_RESOLUTION_MM;
  TCLAP::ValueArg<double> resolution_arg("r", "resolution-mm", arg_description_stream.str(), false, DEFAULT_RESOLUTION_MM, "float");

  // -t --path-tolerance-percent
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "This is the maximum allowable difference between the arc path and the original toolpath.";
  arg_description_stream << " Expressed as a decimal percent, where 0.05 = 5.0%. The lower this value is, the more arcs will be aborted, but values over 0.25 (25%) are not recommended, as they could negatively impact print quality. Restrictions: Only values greater than 0 (0%) and less than 1.0 (100%) are allowed. Default Value: " << ARC_LENGTH_PERCENT_TOLERANCE_DEFAULT << " (" << ARC_LENGTH_PERCENT_TOLERANCE_DEFAULT * 100 << "%)";
  TCLAP::ValueArg<double> path_tolerance_percent_arg("t", "path-tolerance-percent", arg_description_stream.str(), false, DEFAULT_RESOLUTION_MM, "float");

  // -m --max-radius-mm
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The maximum radius of any arc in mm. Restrictions: Only values greater than 0.0 are allowed. Default Value: " << DEFAULT_MAX_RADIUS_MM;
  TCLAP::ValueArg<double> max_radius_arg("m", "max-radius-mm", arg_description_stream.str(), false, DEFAULT_MAX_RADIUS_MM, "float");

  // -z --allow-3d-arcs
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "(experimental) - If supplied, 3D arcs will be allowed (supports spiral vase mode). Not all firmware supports this. Default Value: " << DEFAULT_ALLOW_3D_ARCS;
  TCLAP::SwitchArg allow_3d_arcs_arg("z", "allow-3d-arcs", arg_description_stream.str(), DEFAULT_ALLOW_3D_ARCS);

  // -y --allow-travel-arcs
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "(experimental) - If supplied, travel arcs will be allowed. Default Value: " << DEFAULT_ALLOW_TRAVEL_ARCS;
  TCLAP::SwitchArg allow_travel_arcs_arg("y", "allow-travel-arcs", arg_description_stream.str(), DEFAULT_ALLOW_TRAVEL_ARCS);

  // -d --allow-dynamic-precision
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "If supplied, arcwelder will adjust the precision of the outputted gcode based on the precision of the input gcode. Default Value: " << DEFAULT_ALLOW_DYNAMIC_PRECISION;
  TCLAP::SwitchArg allow_dynamic_precision_arg("d", "allow-dynamic-precision", arg_description_stream.str(), DEFAULT_ALLOW_DYNAMIC_PRECISION);

  // -x --default-xyz-precision
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The default precision of X, Y, Z, I and J output gcode parameters. The precision may be larger than this value if allow-dynamic-precision is set to true. Restrictions: Allowed values are 3, 4, 5, or 6. Default Value: " << DEFAULT_XYZ_PRECISION;
  TCLAP::ValueArg<unsigned int> default_xyz_precision_arg("x", "default-xyz-precision", arg_description_stream.str(), false, DEFAULT_XYZ_PRECISION, "unsigned int");

  // -e --default-e-precision
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The default precision of E output gcode parameters. The precision may be larger than this value if allow-dynamic-precision is set to true. Restrictions: Allowed values are 3, 4, 5, or 6. Default Value: " << DEFAULT_E_PRECISION;
  TCLAP::ValueArg<unsigned int> default_e_precision_arg("e", "default-e-precision", arg_description_stream.str(), false, DEFAULT_E_PRECISION, "unsigned int");

  // -s --mm-per-arc-segment
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The mm per arc segment as defined in your firmware. Used to compensate for firmware without min-arc-segments setting. Restrictions: Only values greater than or equal to 0.0 are allowed. If set greater than 0, min-arc-segments must also be set. Default Value: " << DEFAULT_MM_PER_ARC_SEGMENT;
  TCLAP::ValueArg<double> mm_per_arc_segment_arg("s", "mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MM_PER_ARC_SEGMENT, "float");

  // -a --min-arc-segments
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The minimum number of segments in a full circle of the same radius as any given arc. Used to compensate for firmware without min-arc-segments setting. Restrictions: Only values greater than or equal to 0.0 are allowed. If set greater than 0, mm-per-arc-segment must also be set. Default Value: " << DEFAULT_MIN_ARC_SEGMENTS;
  TCLAP::ValueArg<int> min_arc_segments_arg("a", "min-arc-segments", arg_description_stream.str(), false, DEFAULT_MIN_ARC_SEGMENTS, "int");

  // -v --extrusion-rate-variance
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "(experimental) - The allowed variance in extrusion rate by percent, where 0.05 = 5.0%. A value of 0 will disable this feature. Restrictions: Only values greater than or equal to 0.0 are allowed. Default Value: " << DEFAULT_EXTRUSION_RATE_VARIANCE_PERCENT << " (" << DEFAULT_EXTRUSION_RATE_VARIANCE_PERCENT * 100 << "%)";
  TCLAP::ValueArg<double> extrusion_rate_variance_percent_arg("v", "extrusion-rate-variance-percent", arg_description_stream.str(), false, DEFAULT_EXTRUSION_RATE_VARIANCE_PERCENT, "double");

  // -c --max-gcode-length
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "The maximum length allowed for a generated G2/G3 command, not including any comments. 0 = no limit. Restrictions: Can be set to 0, or values > 30. Default Value: " << DEFAULT_MAX_GCODE_LENGTH;
  TCLAP::ValueArg<int> max_gcode_length_arg("c", "max-gcode-length", arg_description_stream.str(), false, DEFAULT_MAX_GCODE_LENGTH, "int");

  // -p --progress-type
  std::vector<std::string> progress_type_vector;
  std::string progress_type_default_string = PROGRESS_TYPE_SIMPLE;
  progress_type_vector.push_back(PROGRESS_TYPE_NONE);
  progress_type_vector.push_back(PROGRESS_TYPE_SIMPLE);
  progress_type_vector.push_back(PROGRESS_TYPE_FULL);
  TCLAP::ValuesConstraint<std::string> progress_type_constraint(progress_type_vector);
  arg_description_stream.clear();
  arg_description_stream.str("");
  arg_description_stream << "Sets the progress type display. Default Value " << progress_type_default_string;
  TCLAP::ValueArg<std::string> progress_type_arg("p", "progress-type", arg_description_stream.str(), false, progress_type_default_string, &progress_type_constraint);

  // -l --log-level
  std::vector<std::string> log_levels_vector;
  log_levels_vector.push_back("NOSET");
  log_levels_vector.push_back("VERBOSE");
  log_levels_vector.push_back("DEBUG");
  log_levels_vector.push_back("INFO");
  log_levels_vector.push_back("WARNING");
  log_levels_vector.push_back("ERROR");
  log_levels_vector.push_back("CRITICAL");
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
  cmd.add(allow_travel_arcs_arg);
  cmd.add(allow_dynamic_precision_arg);
  cmd.add(default_xyz_precision_arg);
  cmd.add(default_e_precision_arg);
  cmd.add(extrusion_rate_variance_percent_arg);
  cmd.add(max_gcode_length_arg);
  cmd.add(g90_arg);
  cmd.add(progress_type_arg);
  cmd.add(log_level_arg);

  try {
    
    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg. 
    args.source_path = source_arg.getValue();
    args.target_path = target_arg.getValue();

    if (args.target_path.size() == 0)
    {
      args.target_path = args.source_path;
    }

    args.resolution_mm = resolution_arg.getValue();
    args.max_radius_mm = max_radius_arg.getValue();
    args.min_arc_segments = min_arc_segments_arg.getValue();
    args.mm_per_arc_segment = mm_per_arc_segment_arg.getValue();
    args.path_tolerance_percent = path_tolerance_percent_arg.getValue();
    args.allow_3d_arcs = allow_3d_arcs_arg.getValue();
    args.allow_travel_arcs = allow_travel_arcs_arg.getValue();
    args.g90_g91_influences_extruder = g90_arg.getValue();
    args.allow_dynamic_precision = allow_dynamic_precision_arg.getValue();
    args.default_xyz_precision = static_cast<unsigned char>(default_xyz_precision_arg.getValue());
    args.default_e_precision = static_cast<unsigned char>(default_e_precision_arg.getValue());
    args.extrusion_rate_variance_percent = extrusion_rate_variance_percent_arg.getValue();
    args.max_gcode_length = max_gcode_length_arg.getValue();
    progress_type = progress_type_arg.getValue();
    log_level_string = log_level_arg.getValue();
    log_level_value = -1;
     
    // ensure the source file exists
    if (!utilities::does_file_exist(args.source_path))
    {
        throw TCLAP::ArgException("The source file does not exist at the specified path.", source_arg.getName(), "File does not exist error");
    }

    if (args.resolution_mm <= 0)
    {
      throw TCLAP::ArgException("The provided value is less than or equal to 0.", resolution_arg.toString());
    }
    
    if (args.path_tolerance_percent <= 0)
    {
        throw TCLAP::ArgException("The provided value is less than or equal to 0.", path_tolerance_percent_arg.toString());
    }

    if (args.path_tolerance_percent >= 1.0)
    {
        throw TCLAP::ArgException("The provided value is greater than or equal to 1 (100%).", path_tolerance_percent_arg.toString());
    }

    if (args.mm_per_arc_segment < 0)
    {
        throw TCLAP::ArgException("The provided value is negative.", mm_per_arc_segment_arg.toString());
    }

    if (args.min_arc_segments < 0)
    {
        throw TCLAP::ArgException("The provided value is negative.", min_arc_segments_arg.toString());
    }

    if (args.mm_per_arc_segment != 0.0 && args.min_arc_segments == 0)
    {
        throw TCLAP::ArgException("You must also specify a non zero value for the " + min_arc_segments_arg.toString() + " argument.", mm_per_arc_segment_arg.toString());
    }
    else if (args.mm_per_arc_segment == 0.0 && args.min_arc_segments != 0)
    {
        throw TCLAP::ArgException("You must also specify a non zero value for the " + mm_per_arc_segment_arg.toString() + " argument.", min_arc_segments_arg.toString());
    }

    if (args.default_xyz_precision < 3)
    {
        throw TCLAP::ArgException("The provided value is less than 3.", default_xyz_precision_arg.toString());
    }

    if (args.default_e_precision < 3)
    {
        throw TCLAP::ArgException("The provided value is less than 3.", default_e_precision_arg.toString());
    }

    if (args.default_xyz_precision > 6)
    {
        throw TCLAP::ArgException("The provided value is greater than 6.", default_xyz_precision_arg.toString());
    }

    if (args.default_e_precision > 6)
    {
        throw TCLAP::ArgException("The provided value is greater than 6.", default_e_precision_arg.toString());
    }
    
    if (args.extrusion_rate_variance_percent < 0)
    {
        throw TCLAP::ArgException("The provided value is negative.", extrusion_rate_variance_percent_arg.toString());
    }

    if (args.max_gcode_length < 0)
    {
        throw TCLAP::ArgException("The provided value is negative.", max_gcode_length_arg.toString());
    }

    if (args.max_gcode_length > 0 && args.max_gcode_length < 31)
    {
        throw TCLAP::ArgException("The provided value is greater than 0 and less than 31.", max_gcode_length_arg.toString());
    }
    if (args.max_radius_mm <= 0)
    {
        throw TCLAP::ArgException("The provided value is less than or equal to 0.", max_radius_arg.toString());
    }

    if (args.extrusion_rate_variance_percent == 0)
    {
        // warning
        std::cout << "warning: The provided path max radius of " << args.max_radius_mm << "mm is greater than 1000000 (1km), which is not recommended.\n";
    }

    if (args.max_radius_mm > 1000000)
    {
        // warning
        std::cout << "warning: The provided path max radius of " << args.max_radius_mm << "mm is greater than 1000000 (1km), which is not recommended.\n";
    }

    if (args.path_tolerance_percent > 0.25)
    {
        // warning
        std::cout << "warning: The provided path tolerance percent of " << args.path_tolerance_percent << " is greater than 0.25 (25%), which is not recommended.\n";
    }
    else if (args.path_tolerance_percent < 0.001 && args.path_tolerance_percent > 0)
    {
        // warning
        std::cout << "warning: The provided path tolerance percent of " << args.path_tolerance_percent << " is less than 0.001 (0.1%), which is not recommended, and will result in very few arcs being generated.\n";
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
      throw new TCLAP::ArgException("Unknown log level", log_level_arg.toString());
    }

  }
  // catch argument exceptions
  catch (TCLAP::ArgException& e)
  {
    // This will raise an exit exception
    cmd.getOutput()->failure(cmd, e);
    return 1;
  }
  
  // Ensure the log level name is valid
  
  std::vector<std::string> log_names;
  log_names.push_back(ARC_WELDER_LOGGER_NAME);
  std::vector<int> log_levels;
  log_levels.push_back((int)log_levels::DEBUG);
  logger* p_logger = new logger(log_names, log_levels);
  p_logger->set_log_level_by_value(log_level_value);
  args.log = p_logger;
  
  arc_welder* p_arc_welder = NULL;
  
  if (progress_type == PROGRESS_TYPE_NONE)
  {
    p_logger->log(0, log_levels::INFO, "Suppressing progress messages.");
    args.callback = on_progress_suppress;
  }
  else if (progress_type == PROGRESS_TYPE_FULL)
  {
    p_logger->log(0, log_levels::INFO, "Displaying full progress messages.");
    args.callback = on_progress_full;
  }
  else {
    args.callback = on_progress_simple;  
  }
  // Log the arguments
  std::stringstream log_messages;
  log_messages << "Processing GCode.";
  p_logger->log(0, log_levels::INFO, log_messages.str());
  log_messages.clear();
  log_messages.str("");
  log_messages << args.str();
  p_logger->log(0, log_levels::INFO, log_messages.str());

  // Set the box encoding
  args.box_encoding = args.box_encoding = utilities::box_drawing::ASCII;

  p_arc_welder = new arc_welder(args);
  
  arc_welder_results results = p_arc_welder->process();
  if (results.success)
  {
    if (args.allow_travel_arcs)
    {
      log_messages.clear();
      log_messages.str("");
      if (results.progress.travel_statistics.total_count_source == results.progress.travel_statistics.total_count_target)
      {
        log_messages << "Target File Travel Statistics: No travel arcs converted." ;
      }
      else
      {
        log_messages << "\n" << results.progress.travel_statistics.str("Target File Travel Statistics", utilities::box_drawing::ASCII);
      }
      
      p_logger->log(0, log_levels::INFO, log_messages.str());
    }

    log_messages.clear();
    log_messages.str("");
    // Extrusion Statistics
    source_target_segment_statistics combined_stats = source_target_segment_statistics::add(results.progress.segment_statistics, results.progress.segment_retraction_statistics);
    log_messages << "\n" << combined_stats.str("Target File Extrusion Statistics", utilities::box_drawing::ASCII);
    p_logger->log(0, INFO, log_messages.str() );
  
    
    
    log_messages.clear();
    log_messages.str("");
    log_messages << "Arc Welder process completed successfully.";
    p_logger->log(0, log_levels::INFO, log_messages.str());
  }
  else
  {
    log_messages.clear();
    log_messages.str("");
    log_messages << "File processing failed.";
    p_logger->log(0, log_levels::INFO, log_messages.str());
  }

  delete p_arc_welder;
  return 0;
}

bool on_progress_full(arc_welder_progress progress, logger* p_logger, int logger_type)
{
  std::cout << "Progress: "<< progress.str() << std::endl;
  std::cout.flush();
  return true;
}

bool on_progress_simple(arc_welder_progress progress, logger* p_logger, int logger_type)
{
  std::cout << "Progress: " << progress.simple_progress_str() << std::endl;
  std::cout.flush();
  return true;
}

bool on_progress_suppress(arc_welder_progress progress, logger* p_logger, int logger_type)
{
  return true;
}


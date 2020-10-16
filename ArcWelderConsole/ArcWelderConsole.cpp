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
#include "ArcWelderConsole.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "gcode_position.h"
#include <tclap/CmdLine.h>

int main(int argc, char* argv[])
{
  std::string source_file_path;
  std::string target_file_path;
  double resolution_mm;
  double max_radius_mm;
  bool g90_g91_influences_extruder;
  bool hide_progress;
  std::string log_level_string;
  std::string log_level_string_default = "INFO";
  int log_level_value;
  std::string version = "0.1.0";
  std::string info = "Arc Welder: Anti-Stutter - Reduces the number of gcodes per second sent to a 3D printer that supports arc commands (G2 G3)\nCopyright(C) 2020 - Brad Hochgesang";
  std::stringstream arg_description_stream;
  arg_description_stream << std::fixed << std::setprecision(2);
  // Extract arguments
  try {
    // Define the command line object
    TCLAP::CmdLine cmd(info, '=', version);

    // Define Arguments

    // <SOURCE>
    TCLAP::UnlabeledValueArg<std::string> source_arg("source", "The source gcode file to convert.", true, "", "path to source gcode file");

    // <TARGET>
    TCLAP::UnlabeledValueArg<std::string> target_arg("target", "The target gcode file containing the converted code.", true, "", "path to target gcode file");

    // -r --resolution-mm
    arg_description_stream << "The resolution in mm of the of the output.  Determines the maximum tool path deviation allowed during conversion. Default Value: " << DEFAULT_RESOLUTION_MM;
    TCLAP::ValueArg<double> resolution_arg("r", "resolution-mm", arg_description_stream.str(), false, DEFAULT_RESOLUTION_MM, "float");

    // -m --max-radius-mm
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The maximum radius of any arc in mm. Default Value: " << DEFAULT_MAX_RADIUS_MM;
    TCLAP::ValueArg<double> max_radius_arg("m", "max-radius-mm", arg_description_stream.str(), false, DEFAULT_MAX_RADIUS_MM, "float");
    
    // -g --g90-influences-extruder
    TCLAP::SwitchArg g90_arg("g", "g90-influences-extruder", "If supplied, G90/G91 influences the extruder axis.", false);

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
    cmd.add(max_radius_arg);
    cmd.add(g90_arg);
    cmd.add(hide_progress_arg);
    cmd.add(log_level_arg);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg. 
    source_file_path = source_arg.getValue();
    target_file_path = target_arg.getValue();
    resolution_mm = resolution_arg.getValue();
    max_radius_mm = max_radius_arg.getValue();
    g90_g91_influences_extruder = g90_arg.getValue();
    hide_progress = hide_progress_arg.getValue();
    log_level_string = log_level_arg.getValue();

    log_level_value = -1;
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
  log_messages << "Processing Gcode\n";
  log_messages << "\tSource File Path            : " << source_file_path << "\n";
  log_messages << "\tTarget File File            : " << target_file_path << "\n";
  log_messages << "\tResolution in MM            : " << resolution_mm << "\n";
  log_messages << "\tMaximum Arc Radius in MM    : " << max_radius_mm << "\n";
  log_messages << "\tG90/G91 Influences Extruder : " << (g90_g91_influences_extruder ? "True" : "False") << "\n";
  log_messages << "\tLog Level                   : " << log_level_string << "\n";
  log_messages << "\tHide Progress Updates       : " << (hide_progress ? "True" : "False") << "\n";
  p_logger->log(0, INFO, log_messages.str());
  arc_welder* p_arc_welder = NULL;

  if (!hide_progress)
    p_arc_welder = new arc_welder(source_file_path, target_file_path, p_logger, resolution_mm, max_radius_mm, g90_g91_influences_extruder, 50, on_progress);
  else
    p_arc_welder = new arc_welder(source_file_path, target_file_path, p_logger, resolution_mm, max_radius_mm, g90_g91_influences_extruder, 50);

  p_arc_welder->process();

  delete p_arc_welder;
  log_messages.clear();
  log_messages.str("");
  log_messages << "Target file at '" << target_file_path << "' created.  Exiting.";
  p_logger->log(0, INFO, log_messages.str());

  return 0;
}

static bool on_progress(arc_welder_progress progress)
{
  std::cout << progress.str() << std::endl;
  std::cout.flush();
  return true;
}


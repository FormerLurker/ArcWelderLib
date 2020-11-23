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
#if _MSC_VER > 1200
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "inverse_processor.h"
#include "ArcWelderInverseProcessor.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "gcode_position.h"
#include "logger.h"
#include "version.h"
#include "utilities.h"
#include <tclap/CmdLine.h>
#define DEFAULT_ARG_DOUBLE_PRECISION 4

int main(int argc, char* argv[])
{
	std::string info = "Arc Straightener - Converts G2/G3 commands to G1/G2 commands..";

  info.append("\nVersion: ").append(GIT_TAGGED_VERSION);
  info.append(", Branch: ").append(GIT_BRANCH);
  info.append(", BuildDate: ").append(BUILD_DATE);
  info.append("\n").append("Copyright(C) ").append(COPYRIGHT_DATE).append(" - ").append(AUTHOR);

  std::stringstream arg_description_stream;
  arg_description_stream << std::fixed << std::setprecision(5);

  std::string source_file_path;
  std::string target_file_path;
  bool overwrite_source_file = false;
  bool g90_g91_influences_extruder;

  ConfigurationStore cs;
  double mm_per_arc_segment;
  double min_mm_per_arc_segment;
  int min_arc_segments;
  double arc_segments_per_sec;
  
  std::string log_level_string;
  std::string log_level_string_default = "INFO";
  int log_level_value;
  // Extract arguments
  try {
    // Define the command line object
    TCLAP::CmdLine cmd(info, '=', GIT_TAGGED_VERSION);

    // Define Arguments

    // <SOURCE>
    TCLAP::UnlabeledValueArg<std::string> source_arg("source", "The source gcode file to convert.", true, "", "path to source gcode file");

    // <TARGET>
    TCLAP::UnlabeledValueArg<std::string> target_arg("target", "The target gcode file containing the converted code.  If this is not supplied, the source path will be used and the source file will be overwritten.", false, "", "path to target gcode file");
    
    // -g --g90-influences-extruder
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "If supplied, G90/G91 influences the extruder axis.  Default Value: " << DEFAULT_G90_G91_INFLUENCES_EXTRUDER;
    TCLAP::SwitchArg g90_arg("g", "g90-influences-extruder", arg_description_stream.str(), false);

    // -m --mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The default segment length. Default Value: " << DEFAULT_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> mm_per_arc_segment_arg("m", "mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MM_PER_ARC_SEGMENT, "float");

    // -n --min-mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum mm per arc segment.  Used to prevent unnecessarily small segments from being generated. A value less than or equal to 0 will disable this feature. Default Value: " << DEFAULT_MIN_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> min_mm_per_arc_segment_arg("n", "min-mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MIN_MM_PER_ARC_SEGMENT, "float");

    // -s --min-arc-segments
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum number of segments within a circle of the same radius as the arc.  Can be used to increase detail on small arcs.  The smallest segment generated will be no larger than min_mm_per_arc_segment.  A value less than or equal to 0 will disable this feature.  Default Value: " << DEFAULT_MIN_ARC_SEGMENTS;
    TCLAP::ValueArg<int> min_arc_segments_arg("r", "min-arc-segments", arg_description_stream.str(), false, DEFAULT_MIN_ARC_SEGMENTS, "int");

    // -s --arc-segments-per-second
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The number of segments per second.  This will produce a constant number of arcs, clamped between mm-per-arc-segment and min-mm-per-arc-segment.  Can be used to prevent stuttering when printing very quickly.  A value less than or equal to 0 will disable this feature.  Default Value: " << DEFAULT_ARC_SEGMENTS_PER_SEC;
    TCLAP::ValueArg<double> arc_segments_per_sec_arg("s", "arc-segments-per-second", arg_description_stream.str(), false, DEFAULT_MIN_MM_PER_ARC_SEGMENT, "float");

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
    cmd.add(g90_arg);

    cmd.add(mm_per_arc_segment_arg);
    cmd.add(min_mm_per_arc_segment_arg);
    cmd.add(min_arc_segments_arg);
    cmd.add(arc_segments_per_sec_arg);

    cmd.add(log_level_arg);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg. 
    source_file_path = source_arg.getValue();
    target_file_path = target_arg.getValue();
    mm_per_arc_segment = mm_per_arc_segment_arg.getValue();
    min_mm_per_arc_segment = min_mm_per_arc_segment_arg.getValue();
    min_arc_segments = min_arc_segments_arg.getValue();
    arc_segments_per_sec = arc_segments_per_sec_arg.getValue();

    cs.mm_per_arc_segment = (float)mm_per_arc_segment;
    cs.min_mm_per_arc_segment = (float)min_mm_per_arc_segment;
    cs.min_arc_segments = min_arc_segments;
    cs.arc_segments_per_sec = arc_segments_per_sec;

    if (target_file_path.size() == 0)
    {
      target_file_path = source_file_path;
    }
    g90_g91_influences_extruder = g90_arg.getValue();

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

    log_messages << "Source and target path are the same.  The source file will be overwritten.  Temporary file path: " << target_file_path;
    p_logger->log(0, INFO, log_messages.str());
    log_messages.clear();
    log_messages.str("");
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

  log_messages << "\tLog Level                    : " << log_level_string << "\n";
  p_logger->log(0, INFO, log_messages.str());
  
  if (overwrite_source_file)
  {
    target_file_path = temp_file_path;
  }
  
  inverse_processor processor(source_file_path, target_file_path, g90_g91_influences_extruder, 50, cs);
  processor.process();
  // Todo:  get some results!
  if (true)
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
      log_messages << "Renaming temporary file at '" << target_file_path << "' to '" << source_file_path << "'.";
      p_logger->log(0, INFO, log_messages.str());
      std::rename(target_file_path.c_str(), source_file_path.c_str());
    }
    /*
    log_messages.clear();
    log_messages.str("");
    log_messages << std::endl << results.progress.segment_statistics.str();
    p_logger->log(0, INFO, log_messages.str());
    */
    log_messages.clear();
    log_messages.str("");
    log_messages << "Process completed successfully.";
    p_logger->log(0, INFO, log_messages.str());
  }
  else
  {
    log_messages.clear();
    log_messages.str("");
    log_messages << "File processing failed.";
    p_logger->log(0, INFO, log_messages.str());
  }
  return 0;

}

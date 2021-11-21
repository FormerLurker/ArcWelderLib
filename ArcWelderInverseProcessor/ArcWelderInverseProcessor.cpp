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

#include "ArcWelderInverseProcessor.h"
#include "arc_interpolation.h"
#include "marlin_1.h"
#include "marlin_2.h"
#include "repetier.h"
#include "prusa.h"
#include "smoothieware.h"
#include "logger.h"
#include "version.h"
#include "utilities.h"
#include <tclap/CmdLine.h>
#define DEFAULT_ARG_DOUBLE_PRECISION 4
int main(int argc, char* argv[])
{
  try {
    run_arc_straightener(argc, argv);
  }
  catch (TCLAP::ArgException *e) {
    std::cout << (*e).what() << " - " << (*e).typeDescription() << "\n";
    return -1;
  }
}
int run_arc_straightener(int argc, char* argv[])
{
  std::string info = "Arc Straightener - Converts G2/G3 commands to G1/G2 commands..";

  info.append("\nVersion: ").append(GIT_TAGGED_VERSION);
  info.append(", Branch: ").append(GIT_BRANCH);
  info.append(", BuildDate: ").append(BUILD_DATE);
  info.append("\n").append("Copyright(C) ").append(COPYRIGHT_DATE).append(" - ").append(AUTHOR);

  std::stringstream arg_description_stream;
  arg_description_stream << std::fixed << std::setprecision(5);

  arc_interpolation_args args;
  bool overwrite_source_file = false;

  std::string log_level_string;
  std::string log_level_string_default = "INFO";
  int log_level_value;

  // Create an instance of all supported firmware types using the default args
  marlin_1 marlin_1_firmware(args.firmware_args);
  marlin_2 marlin_2_firmware(args.firmware_args);
  repetier repetier_firmware(args.firmware_args);
  prusa prusa_firmware(args.firmware_args);
  smoothieware smoothieware_firmware(args.firmware_args);

  // Extract arguments
  try {
    // Define the command line object
    TCLAP::CmdLine cmd(info, '=', GIT_TAGGED_VERSION);

    // Define Arguments

    // <SOURCE>
    TCLAP::UnlabeledValueArg<std::string> source_arg("source", "The source gcode file to convert.", true, "", "path to source gcode file");

    // <TARGET>
    TCLAP::UnlabeledValueArg<std::string> target_arg("target", "The target gcode file containing the converted code.  If this is not supplied, the source path will be used and the source file will be overwritten.", false, "", "path to target gcode file");

    // -f --firmware-type
    std::vector<std::string> firmware_types_vector;
    for (int i = 0; i < NUM_FIRMWARE_TYPES; i++)
    {
      firmware_types_vector.push_back(firmware_type_names[i]);
    }
    TCLAP::ValuesConstraint<std::string> firmware_type_constraint(firmware_types_vector);
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "Sets the firmware to emulate.  Default Value: " << firmware_type_names[DEFAULT_FIRMWARE_TYPE];
    TCLAP::ValueArg<std::string> firmware_type_arg("f", "firmware-type", arg_description_stream.str(), false, firmware_type_names[DEFAULT_FIRMWARE_TYPE], &firmware_type_constraint);

    // -v --firmware-version
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "Sets the firmware version to use.  The available versions depend on the firmware type selected.  " << DEFAULT_FIRMWARE_VERSION_NAME << " will select the most recent version available.\n";
    arg_description_stream << "\tMARLIN 1 versions: " << utilities::join(marlin_1_firmware.get_version_names(), ", ") << "\n";
    arg_description_stream << "\tMARLIN 2 versions: " << utilities::join(marlin_2_firmware.get_version_names(), ", ") << "\n";
    arg_description_stream << "\tREPETIER versions: " << utilities::join(repetier_firmware.get_version_names(), ", ") << "\n";
    arg_description_stream << "\tPRUSA versions: " << utilities::join(prusa_firmware.get_version_names(), ", ") << "\n";
    arg_description_stream << "\tSMOOTHIEWARE versions: " << utilities::join(smoothieware_firmware.get_version_names(), ", ") << "\n";
    arg_description_stream << "\tDefault Value: " << DEFAULT_FIRMWARE_VERSION_NAME;
    TCLAP::ValueArg<std::string> firmware_version_arg("v", "firmware-version", arg_description_stream.str(), false, DEFAULT_FIRMWARE_VERSION_NAME, "string");

    // -g --g90-influences-extruder
    std::string g90_g91_influences_extruder_default_value = "DEFAULT";
    std::vector<std::string> g90_g91_influences_extruder_vector;
    g90_g91_influences_extruder_vector.push_back("TRUE");
    g90_g91_influences_extruder_vector.push_back("FALSE");
    g90_g91_influences_extruder_vector.push_back(g90_g91_influences_extruder_default_value);
    TCLAP::ValuesConstraint<std::string> g90_g91_influences_extruder_constraint(g90_g91_influences_extruder_vector);
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "Sets the firmware's G90/G91 influences extruder axis behavior.  By default this is determined by the firmware's behavior.  Default Value: " << g90_g91_influences_extruder_default_value;
    TCLAP::ValueArg<std::string> g90_arg("g", "g90-influences-extruder", arg_description_stream.str(), false, g90_g91_influences_extruder_default_value, &g90_g91_influences_extruder_constraint);

    // -m --mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The default segment length. Default Value: " << DEFAULT_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> mm_per_arc_segment_arg("m", "mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MM_PER_ARC_SEGMENT, "float");
    
    // max_arc_segment_mm_arg
    // -d --mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The maximum length of an arc segment. Default Value: " << DEFAULT_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> max_arc_segment_mm_arg("d", "max-arc-segment-mm", arg_description_stream.str(), false, DEFAULT_MM_PER_ARC_SEGMENT, "float");

    // -n --min-mm-per-arc-segment
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum mm per arc segment.  Used to prevent unnecessarily small segments from being generated. A value less than or equal to 0 will disable this feature. Default Value: " << DEFAULT_MIN_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> min_mm_per_arc_segment_arg("n", "min-mm-per-arc-segment", arg_description_stream.str(), false, DEFAULT_MIN_MM_PER_ARC_SEGMENT, "float");

    // min_arc_segment_mm
    // -b --min-arc-segment-mm
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum mm per arc segment.  Used to prevent unnecessarily small segments from being generated. A value less than or equal to 0 will disable this feature. Default Value: " << DEFAULT_MIN_MM_PER_ARC_SEGMENT;
    TCLAP::ValueArg<double> min_arc_segment_mm_arg("b", "min-arc-segment-mm", arg_description_stream.str(), false, DEFAULT_MIN_MM_PER_ARC_SEGMENT, "float");

    // -r --min-arc-segments
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum number of segments within a circle of the same radius as the arc.  Can be used to increase detail on small arcs.  The smallest segment generated will be no larger than min_mm_per_arc_segment.  A value less than or equal to 0 will disable this feature.  Default Value: " << DEFAULT_MIN_ARC_SEGMENTS;
    TCLAP::ValueArg<int> min_arc_segments_arg("r", "min-arc-segments", arg_description_stream.str(), false, DEFAULT_MIN_ARC_SEGMENTS, "int");

    // min_circle_segments_arg
    // -a --min-circle-segments-arg
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The minimum number of segments within a circle of the same radius as the arc.  Can be used to increase detail on small arcs.  The smallest segment generated will be no larger than min_mm_per_arc_segment.  A value less than or equal to 0 will disable this feature.  Default Value: " << DEFAULT_MIN_ARC_SEGMENTS;
    TCLAP::ValueArg<int> min_circle_segments_arg("a", "min-circle-segments", arg_description_stream.str(), false, DEFAULT_MIN_ARC_SEGMENTS, "int");

    // -c --n-arc-correction
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The number of segments that will be interpolated using a small angle approximation before true sin/cos corrections are applied.  A value less than or equal to 1 will disable this feature.  Default Value: " << DEFAULT_N_ARC_CORRECTIONS;
    TCLAP::ValueArg<int> n_arc_correction_arg("c", "n-arc-correction", arg_description_stream.str(), false, DEFAULT_N_ARC_CORRECTIONS, "int");

    // -s --arc-segments-per-second
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "The number of segments per second.  This will produce a constant number of arcs, clamped between mm-per-arc-segment and min-mm-per-arc-segment.  Can be used to prevent stuttering when printing very quickly.  A value less than or equal to 0 will disable this feature.  Default Value: " << DEFAULT_ARC_SEGMENTS_PER_SEC;
    TCLAP::ValueArg<double> arc_segments_per_sec_arg("s", "arc-segments-per-second", arg_description_stream.str(), false, DEFAULT_MIN_MM_PER_ARC_SEGMENT, "float");

    // -e --mm-max-arc-error
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "This currently is only used in Smoothieware.   The maximum error for line segments that divide arcs.  Set to 0 to disable.  Default Value: " << DEFAULT_MM_MAX_ARC_ERROR;
    TCLAP::ValueArg<double> mm_max_arc_error_arg("e", "mm-max-arc-error", arg_description_stream.str(), false, DEFAULT_MM_MAX_ARC_ERROR, "float");

    // -p --print-firmware-defaults
    arg_description_stream.clear();
    arg_description_stream.str("");
    arg_description_stream << "Prints all available settings and defaults for the provided firmware type and version.  All other parameters will be ignored.";
    TCLAP::SwitchArg print_firmware_defaults_arg("p", "print-firmware-defaults", arg_description_stream.str());

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
    arg_description_stream << "Sets console log level. Possible values: Default Value: " << log_level_string_default;
    TCLAP::ValueArg<std::string> log_level_arg("l", "log-level", arg_description_stream.str(), false, log_level_string_default, &log_levels_constraint);

    // Add all arguments
    cmd.add(source_arg);
    cmd.add(target_arg);
    cmd.add(firmware_type_arg);
    cmd.add(firmware_version_arg);
    cmd.add(g90_arg);
    cmd.add(mm_per_arc_segment_arg);
    cmd.add(min_mm_per_arc_segment_arg);
    cmd.add(min_arc_segments_arg);
    cmd.add(n_arc_correction_arg);
    cmd.add(arc_segments_per_sec_arg);
    cmd.add(log_level_arg);
    cmd.add(min_circle_segments_arg);
    cmd.add(min_arc_segment_mm_arg);
    cmd.add(max_arc_segment_mm_arg);
    cmd.add(print_firmware_defaults_arg);

    // Parse the argv array.
    cmd.parse(argc, argv);

    // Ok!  Now let's see what firmware and version were selected, then we can start adding parameters
    // First, Set the firmware type
    std::string firmware_type_string = firmware_type_arg.getValue();
    for (int i = 0; i < NUM_FIRMWARE_TYPES; i++)
    {
      if (firmware_type_names[i] == firmware_type_string)
      {
        args.firmware_args.firmware_type = static_cast<firmware_types>(i);
      }
    }
    // Now set the version
    // Set the firmware version, and check to make sure that the version supplied is supported.
    std::string firmware_version_string = firmware_version_arg.getValue();
    switch (args.firmware_args.firmware_type)
    {
    case firmware_types::MARLIN_1:
      if (!marlin_1_firmware.is_valid_version(firmware_version_string))
      {
        throw new TCLAP::ArgException("Unknown Version Exception", firmware_version_arg.getName(), "'" + firmware_version_string + "' is not a valid version for " + firmware_type_string + " firmware type.");
      }
      break;
    case firmware_types::MARLIN_2:
      if (!marlin_2_firmware.is_valid_version(firmware_version_string))
      {
        throw new TCLAP::ArgException("Unknown Version Exception", firmware_version_arg.getName(), "'" + firmware_version_string + "' is not a valid version for " + firmware_type_string + " firmware type.");
      }
      break;
    case firmware_types::REPETIER:
      if (!repetier_firmware.is_valid_version(firmware_version_string))
      {
        throw new TCLAP::ArgException("Unknown Version Exception", firmware_version_arg.getName(), "'" + firmware_version_string + "' is not a valid version for " + firmware_type_string + " firmware type.");
      }
      break;
    case firmware_types::PRUSA:
      if (!prusa_firmware.is_valid_version(firmware_version_string))
      {
        throw new TCLAP::ArgException("Unknown Version Exception", firmware_version_arg.getName(), "'" + firmware_version_string + "' is not a valid version for " + firmware_type_string + " firmware type.");
      }
      break;
    case firmware_types::SMOOTHIEWARE:
      if (!smoothieware_firmware.is_valid_version(firmware_version_string))
      {
        throw new TCLAP::ArgException("Unknown Version Exception", firmware_version_arg.getName(), "'" + firmware_version_string + "' is not a valid version for " + firmware_type_string + " firmware type.");
      }
      break;
    }
    args.firmware_args.version = firmware_version_string;

    // now that we have the firmware type and version, we can extract the default arguments and override any settings that are supplied
    switch (args.firmware_args.firmware_type)
    {
    case firmware_types::MARLIN_1:
      marlin_1_firmware.set_arguments(args.firmware_args);
      args.firmware_args = marlin_1_firmware.get_default_arguments_for_current_version();
      break;
    case firmware_types::MARLIN_2:
      marlin_2_firmware.set_arguments(args.firmware_args);
      args.firmware_args = marlin_2_firmware.get_default_arguments_for_current_version();
      break;
    case firmware_types::REPETIER:
      repetier_firmware.set_arguments(args.firmware_args);
      args.firmware_args = repetier_firmware.get_default_arguments_for_current_version();
      break;
    case firmware_types::PRUSA:
      prusa_firmware.set_arguments(args.firmware_args);
      args.firmware_args = prusa_firmware.get_default_arguments_for_current_version();
      break;
    case firmware_types::SMOOTHIEWARE:
      smoothieware_firmware.set_arguments(args.firmware_args);
      args.firmware_args = smoothieware_firmware.get_default_arguments_for_current_version();
      break;
    }

    // see if the user want's to see the default settings
    if (print_firmware_defaults_arg.getValue())
    {
      std::cout << "Showing arguments and defaults for " << firmware_type_string << " ("<< firmware_version_string << ")\n";
      std::cout << "Available argument for firmware: " << get_available_arguments_string(args.firmware_args.get_available_arguments()) << "\n";
      std::cout << "Default " << args.firmware_args.get_argument_description();
      return 0;
    }
    // Get the value parsed by each arg. 
    args.source_path = source_arg.getValue();
    args.target_path = target_arg.getValue();
    if (args.target_path.size() == 0)
    {
      args.target_path = args.source_path;
    }

    // If the arguments are set, apply them.  If not, don't.
    if (mm_per_arc_segment_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("mm_per_arc_segment"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", mm_per_arc_segment_arg.getName(), "The argument does not apply to the " + firmware_type_string +" " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.mm_per_arc_segment = mm_per_arc_segment_arg.getValue();
    }
    if (min_mm_per_arc_segment_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("min_mm_per_arc_segment"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", min_mm_per_arc_segment_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.min_mm_per_arc_segment = min_mm_per_arc_segment_arg.getValue();
    }
    if (min_arc_segments_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("min_arc_segments"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", min_arc_segments_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.min_arc_segments = min_arc_segments_arg.getValue();
    }
    if (arc_segments_per_sec_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("arc_segments_per_sec"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", arc_segments_per_sec_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.arc_segments_per_sec = arc_segments_per_sec_arg.getValue();
    }
    if (g90_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("g90_g91_influences_extruder"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", g90_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.g90_g91_influences_extruder = g90_arg.getValue() == "TRUE";
    }
    if (n_arc_correction_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("n_arc_correction"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", n_arc_correction_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.n_arc_correction = n_arc_correction_arg.getValue();
    }
    if (mm_max_arc_error_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("mm_max_arc_error"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", mm_max_arc_error_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.mm_max_arc_error = mm_max_arc_error_arg.getValue();
    }

    // min_circle_segments
    if (min_circle_segments_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("min_circle_segments"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", min_circle_segments_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.set_min_circle_segments(min_circle_segments_arg.getValue());
    }
    // min_arc_segment_mm
    if (min_arc_segment_mm_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("min_arc_segment_mm"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", min_arc_segment_mm_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.set_min_arc_segment_mm(min_arc_segment_mm_arg.getValue());
    }
    // max_arc_segment_mm
    if (max_arc_segment_mm_arg.isSet())
    {
      // See if this argument is supported
      if (!args.firmware_args.is_argument_used("max_arc_segment_mm"))
      {
        throw new TCLAP::ArgException("Invalid Argument For Firmware", max_arc_segment_mm_arg.getName(), "The argument does not apply to the " + firmware_type_string + " " + firmware_version_arg.getValue() + " firmware.  Only the following parameters are supported: " + get_available_arguments_string(args.firmware_args.get_available_arguments()));
      }
      args.firmware_args.set_max_arc_segment_mm(max_arc_segment_mm_arg.getValue());
    }
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
  log_levels.push_back((int)log_levels::DEBUG);
  logger* p_logger = new logger(log_names, log_levels);
  p_logger->set_log_level_by_value(log_level_value);

  std::stringstream log_messages;
  std::string temp_file_path = "";
  log_messages << std::fixed << std::setprecision(DEFAULT_ARG_DOUBLE_PRECISION);
  if (args.source_path == args.target_path)
  {
    overwrite_source_file = true;
    if (!utilities::get_temp_file_path_for_file(args.source_path, temp_file_path))
    {
      log_messages << "The source and target path are the same, but a temporary file path could not be created.  Is the path empty?";
      p_logger->log(0, log_levels::INFO, log_messages.str());
      log_messages.clear();
      log_messages.str("");
    }

    // create a uuid with a tmp extension for the temporary file

    log_messages << "Source and target path are the same.  The source file will be overwritten.  Temporary file path: " << temp_file_path;
    p_logger->log(0, log_levels::INFO, log_messages.str());
    log_messages.clear();
    log_messages.str("");
  }
  log_messages << "Arguments: \n";
  log_messages << "\tSource File Path             : " << args.source_path << "\n";
  if (overwrite_source_file)
  {
    log_messages << "\tTarget File Path (overwrite) : " << args.target_path << "\n";
    log_messages << "\tTemporary File Path          : " << temp_file_path << "\n";
  }
  else
  {
    log_messages << "\tTarget File File             : " << args.target_path << "\n";
  }

  log_messages << "\tLog Level                    : " << log_level_string << "\n";


  if (overwrite_source_file)
  {
    args.target_path = temp_file_path;
  }

  arc_interpolation interpolator(args);
  log_messages << interpolator.get_firmware_argument_description();
  p_logger->log(0, log_levels::INFO, log_messages.str());

  p_logger->log(0, log_levels::INFO, "Running interpolation...");
  interpolator.process();
  p_logger->log(0, log_levels::INFO, "Interpolation Complete.");

  log_messages.clear();
  log_messages.str("");
  log_messages << "Target file at '" << args.target_path << "' created.";

  if (overwrite_source_file)
  {
    log_messages.clear();
    log_messages.str("");
    log_messages << "Deleting the original source file at '" << args.source_path << "'.";
    p_logger->log(0, log_levels::INFO, log_messages.str());
    log_messages.clear();
    log_messages.str("");
    std::remove(args.source_path.c_str());
    log_messages << "Renaming temporary file at '" << args.target_path << "' to '" << args.source_path << "'.";
    p_logger->log(0, log_levels::INFO, log_messages.str());
    std::rename(args.target_path.c_str(), args.source_path.c_str());
  }

  log_messages.clear();
  log_messages.str("");
  log_messages << "Process completed successfully.";
  p_logger->log(0, log_levels::INFO, log_messages.str());

  return 0;

}


std::string get_available_arguments_string(std::vector<std::string> firmware_arguments)
{
  std::string available_argument_string = "";

  for (std::vector<std::string>::iterator it = firmware_arguments.begin(); it != firmware_arguments.end(); it++)
  {
    if (available_argument_string.size() > 0)
    {
      available_argument_string += ", ";
    }
    available_argument_string += "--" +utilities::replace(*it, "_", "-");
  }
  return available_argument_string;
}
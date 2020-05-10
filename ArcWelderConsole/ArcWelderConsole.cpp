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
int main(int argc, char* argv[])
{
	std::string info = "Arc Welder: Anti-Stutter v0.1.rc1.dev0\nReduces the number of gcodes per second sent to a 3D printer that supports arc commands (G2 G3)\nCopyright(C) 2020 - Brad Hochgesang\n";
	std::cout << info;
	// General Strings
	std::string usage_string = "Usage: ";
	std::string default_value_string = "Default Value: ";
	std::string usage_example_string = "Example Usage: ";
	// Resolution messages
	std::string resolution_parameter = "--resolution-mm";
	std::string resolution_usage = resolution_parameter + " {float}";
	std::string resolution_description = "The resolution in mm of the of the output.  Determines the maximum tool path deviation allowed during conversion.";
	std::string resolution_default_value = "0.05 (in millimeters)";
	std::string resolution_usage_example = resolution_parameter + " 0.1";
	// g90/g91 influences extruder messages
	std::string g90_influences_extruder_parameter = "--g90-influences-extruder";
	std::string g90_influences_extruder_usage = g90_influences_extruder_parameter + " {true/false}";
	std::string g90_influences_extruder_description = "If true, G90 and G91 influence the E axis.  This can affect retraction and extrusion calculations depending on the gcode.";
	std::string g90_influences_extruder_default_value = "false";
	std::string g90_influences_extruder_usage_example = g90_influences_extruder_parameter + " true";
	// show_progress messages
	std::string show_progress_parameter = "--show-progress";
	std::string show_progress_usage = show_progress_parameter + " {true/false}";
	std::string show_progress_description = "Display a periodic progress message every 1 second.";
	std::string show_progress_default_value = "true";
	std::string show_progress_usage_example = show_progress_parameter + " false";
	// log level messages
	std::string log_level_parameter = "--log-level";
	std::string log_level_usage = log_level_parameter + " {NOSET/VERBOSE/DEBUG/INFO/WARNING/ERROR/CRITICAL}";
	std::string log_level_usage_description = "Sets console log level.  DEBUG, VERBOSE, AND NOSET will produce a HUGE amount of output, and will slow processing considerably.  Errors and Exceptions will be logged to stderr.";
	std::string log_level_usage_default_value = "ERROR";
	std::string log_level_usage_example = log_level_parameter + " DEBUG";

	std::stringstream usage_message_stream;
	usage_message_stream << usage_string << argv[0] << " {SOURCE_PATH} {DESTINATION_PATH} {optional arguments}\n";
	usage_message_stream << usage_example_string << argv[0] << "\"c:\\source_file.gcode\" \"c:\\target_file.gcode\" -resolution 0.05 -g90-91-influences-extruder false -log-level DEBUG\n";
	usage_message_stream << "********************\n";
	usage_message_stream << "Argument Description:\n";
	usage_message_stream << "SOURCE_PATH:\n\tThe full path of the gcode file to compress.  This is a required argument\n";
	usage_message_stream << "TARGET_PATH:\n\tThe full path of the compressed gcode output.  Any existing target file will be overwritten!\n";
	usage_message_stream << resolution_usage << "\n\t" << resolution_description << "\n\t" << default_value_string << resolution_default_value << "\n\t" << usage_example_string  << resolution_usage_example << "\n";
	usage_message_stream << g90_influences_extruder_usage << "\n\t" << g90_influences_extruder_description << "\n\t" << default_value_string << g90_influences_extruder_default_value << "\n\t" << usage_example_string << g90_influences_extruder_usage_example << "\n";
	usage_message_stream << show_progress_parameter << "\n\t" << show_progress_description << "\n\t" << default_value_string << show_progress_default_value << "\n\t" << usage_example_string << show_progress_usage_example << "\n";
	usage_message_stream << log_level_usage << "\n\t" << log_level_usage_description << "\n\t" << default_value_string << log_level_usage_default_value << "\n\t" << usage_example_string << log_level_usage_example << "\n";
	
	// Ensure at least 3 parameters, including the cmd.
	if (argc < 3)
	{
		if (argc > 1)
			std::cerr << "Error executing " << argv[0] << ":  Invalid number of arguments.\n";
		else
			std::cout << argv[0] << " - Displaying Help\n";
		std::cout << usage_message_stream.str();
		if (argc > 1)
			return 1;
		return -1;
	}
	// get the source and target path
	std::string source_file_path = argv[1];
	std::string target_file_apth = argv[2];
	double resolution_mm = 0.05;
	bool g90_g91_influences_extruder = false;
	bool show_progress = true;
	int log_level_value = 40;
	// Extract opotional parameters
	std::vector <std::string> sources;
	for (int i = 3; i < argc; ++i) {
		std::string parameter;
		for (unsigned int letter_index = 0; letter_index < strlen(argv[i]); letter_index++)
			parameter += tolower(argv[i][letter_index]);
		
		if (parameter == resolution_parameter) {
			// make sure we have another argument after the flag
			if (i + 1 >= argc)
			{
				std::cerr << "The " << resolution_parameter <<" parameter requires a float value.\n\t" << usage_string << resolution_usage << "\n\t" << usage_example_string << resolution_usage_example << "\n";
				return 1;
			}
			i++; // increment the index to extract the parameter's value
			try
			{
				resolution_mm = std::stod(argv[i]);
			}
			catch (std::invalid_argument) {
				std::cerr << "Unable to convert the " << resolution_parameter << " value '" << argv[i] << "' to a float.\n\t" << usage_string << resolution_usage << "\n\t" << usage_example_string << resolution_usage_example << "\n";
				return 1;
			}
		}
		else if (parameter == g90_influences_extruder_parameter)
		{
			// make sure we have another argument after the flag
			if (i + 1 >= argc)
			{
				std::cerr << "The " << g90_influences_extruder_parameter << " parameter requires a boolean value.\n\t" << usage_string << g90_influences_extruder_usage << "\n\t" << usage_example_string << g90_influences_extruder_usage_example << "\n";
				return 1;
			}
			i++; // increment the index to extract the parameter's value
			std::string g90_g91_influences_extruder_string;
			
			for (unsigned int letter_index = 0; letter_index < strlen(argv[i]); letter_index++)
				g90_g91_influences_extruder_string += toupper(argv[i][letter_index]);

			if (g90_g91_influences_extruder_string != "TRUE" && g90_g91_influences_extruder_string != "FALSE")
			{
				std::cerr << "Unable to convert the " << g90_influences_extruder_parameter << " value '" << argv[i] << "' to a boolean.\n\t" << usage_string << g90_influences_extruder_usage << "\n\t" << usage_example_string << g90_influences_extruder_usage_example << "\n";
				return 1;
			}
			g90_g91_influences_extruder = g90_g91_influences_extruder_string == "TRUE";
		}
		else if (parameter == show_progress_parameter)
		{
			// make sure we have another argument after the flag
			if (i + 1 >= argc)
			{
				std::cerr << "The " << show_progress_parameter << " parameter requires a boolean value.\n\t" << usage_string << show_progress_usage << "\n\t" << usage_example_string << show_progress_usage_example << "\n";
				return 1;
			}
			i++; // increment the index to extract the parameter's value
			std::string show_progress_string;
			for (unsigned int letter_index = 0; letter_index < strlen(argv[i]); letter_index++)
				show_progress_string += toupper(argv[i][letter_index]);
			if (show_progress_string != "TRUE" && show_progress_string != "FALSE")
			{
				std::cerr << "Unable to convert the " << show_progress_parameter << " value '" << argv[i] << "' to a boolean.\n\t" << usage_string << show_progress_usage << "\n\t" << usage_example_string << show_progress_usage_example << "\n";
				return 1;
			}
			show_progress = show_progress_string == "TRUE";
		}
		else if (parameter == log_level_parameter)
		{
			// make sure we have another argument after the flag
			if (i + 1 >= argc)
			{
				std::cerr << "The " << log_level_parameter << " parameter requires a valid log level string.\n\t" << usage_string << log_level_usage << "\n\t" << usage_example_string << log_level_usage_example << "\n";
				return 1;
			}
			i++; // increment the index to extract the parameter's value
			std::string log_level_name;
			for (unsigned int letter_index = 0; letter_index < strlen(argv[i]); letter_index++)
				log_level_name += toupper(argv[i][letter_index]);

			// Ensure the log level name is valid
			log_level_value = -1;
			for (unsigned int log_name_index = 0; log_name_index < log_level_names.size(); log_name_index++)
			{
				if (log_level_name == log_level_names[log_name_index])
				{
					log_level_value = log_level_values[log_name_index];
					break;
				}
			}
			if (log_level_value == -1)
			{
				std::cerr << "Unable to convert the " << log_level_parameter << " value '" << argv[i] << "' to a valid log level string.\n\t" << usage_string << log_level_usage << "\n\t" << usage_example_string << log_level_usage_example << "\n";
				return 1;
			}

		}
		else {
			std::cerr << "An unknown parameter '" << parameter << "' was received.\n" << usage_message_stream.str();
			return 1;
		}
	}

	std::vector<std::string> log_names;
	log_names.push_back("arc_welder.gcode_conversion");
	std::vector<int> log_levels;
	log_levels.push_back(log_levels::DEBUG);
	logger* p_logger = new logger(log_names, log_levels);
	p_logger->set_log_level_by_value(log_level_value);
	std::string log_level_name = "No Logging";
	if (log_level_value >= 0)	log_level_names[logger::get_log_level_for_value(log_level_value)];

	std::stringstream log_messages;
	log_messages << "Processing Gcode\n";
	log_messages << "\tSource File Path            : " << source_file_path << "\n";
	log_messages << "\tTarget File File            : " << target_file_apth << "\n";
	log_messages << "\tResolution in MM            : " << resolution_mm << "\n";
	log_messages << "\tG90/G91 Influences Extruder : " << (g90_g91_influences_extruder ? "True" : "False") << "\n";
	log_messages << "\tLog Level                   : " << log_level_name << "\n";
	log_messages << "\tShow Progress Updates       : " << (show_progress ? "True" : "False") << "\n";
	std::cout << log_messages.str();
	arc_welder* p_arc_welder = NULL;
	if (show_progress)
		p_arc_welder = new arc_welder(source_file_path, target_file_apth, p_logger, resolution_mm, g90_g91_influences_extruder, 50, on_progress);
	else
		p_arc_welder = new arc_welder(source_file_path, target_file_apth, p_logger, resolution_mm, g90_g91_influences_extruder, 50);
	
	p_arc_welder->process();

	delete p_arc_welder;
	log_messages.clear();
	log_messages.str("");
	log_messages << "Target file at '" << target_file_apth << "' created.  Exiting.";
	p_logger->log(0, INFO, log_messages.str());
	return 0;
}

static bool on_progress(arc_welder_progress progress)
{
	std::cout << progress.str() << "\n";
	return true;
}

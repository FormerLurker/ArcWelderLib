////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Test Application
//
// This application is only used for ad-hoc testing of the anti-stutter library.
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
#include "ArcWelderTest.h"
#include "logger.h"
#include <iostream>
#include "utilities.h"

int main(int argc, char* argv[])
{
	run_tests(argc, argv);
}

int run_tests(int argc, char* argv[])
{
	_CrtMemState state;
	// This line will take a snapshot
	// of the memory allocated at this point.
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

	//std::string filename = argv[1];
	unsigned int num_runs = 1;
	_CrtMemCheckpoint(&state);

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned int index = 0; index < num_runs; index++)
	{
		std::cout << "Processing test run " << index + 1 << " of " << num_runs << ".\r\n";
		TestAntiStutter(ANTI_STUTTER_TEST);
		//TestParsingCase();
		//TestDoubleToString();
		//TestInverseProcessor();
		//TestCircularBuffer();
		//TestSegmentedLine();
		//TestSegmentedArc();

	}
	auto end = std::chrono::high_resolution_clock::now();
	_CrtMemDumpAllObjectsSince(&state);
	std::chrono::duration<double> diff = end - start;
	std::cout << "Tests completed in " << diff.count() << " seconds";
	//std::cout << "Has Memory Leak = " << has_leak << ".\r\n";
	// Set the debug-heap flag so that memory leaks are reported when
	// the process terminates. Then, exit.
	//printf("Press Any Key to Continue\n");
	//std::getchar();
	return 0;
}

static gcode_position_args get_single_extruder_position_args()
{
	gcode_position_args posArgs = gcode_position_args();
	posArgs.autodetect_position = true;
	posArgs.home_x = 0;
	posArgs.home_x_none = true;
	posArgs.home_y = 0;
	posArgs.home_y_none = true;
	posArgs.home_z = 0;
	posArgs.home_z_none = true;
	posArgs.shared_extruder = true;
	posArgs.zero_based_extruder = true;
	posArgs.set_num_extruders(1);
	posArgs.retraction_lengths[0] = .8;
	posArgs.z_lift_heights[0] = .6;
	posArgs.x_firmware_offsets[0] = 0;
	posArgs.y_firmware_offsets[0] = 1;
	posArgs.default_extruder = 0;
	posArgs.priming_height = 0.4;
	posArgs.minimum_layer_height = 0.05;
	posArgs.height_increment = 0.5;
	posArgs.g90_influences_extruder = false;
	posArgs.xyz_axis_default_mode = "absolute";
	posArgs.e_axis_default_mode = "absolute";
	posArgs.units_default = "millimeters";
	posArgs.location_detection_commands = std::vector<std::string>();
	posArgs.is_bound_ = true;
	posArgs.is_circular_bed = false;
	posArgs.snapshot_x_min = 0;
	posArgs.snapshot_x_max = 250;
	posArgs.snapshot_y_min = 0;
	posArgs.snapshot_y_max = 210;
	posArgs.snapshot_z_min = 0;
	posArgs.snapshot_z_max = 200;
	posArgs.x_min = 0;
	posArgs.x_max = 250;
	posArgs.y_min = -3;
	posArgs.y_max = 210;
	posArgs.z_min = 0;
	posArgs.z_max = 200;
	return posArgs;
}

static gcode_position_args get_5_shared_extruder_position_args()
{
	gcode_position_args posArgs = gcode_position_args();
	posArgs.autodetect_position = true;
	posArgs.home_x = 0;
	posArgs.home_x_none = true;
	posArgs.home_y = 0;
	posArgs.home_y_none = true;
	posArgs.home_z = 0;
	posArgs.home_z_none = true;
	posArgs.shared_extruder = true;
	posArgs.zero_based_extruder = true;
	posArgs.set_num_extruders(5);
	posArgs.retraction_lengths[0] = .2;
	posArgs.retraction_lengths[1] = .4;
	posArgs.retraction_lengths[2] = .6;
	posArgs.retraction_lengths[3] = .8;
	posArgs.retraction_lengths[4] = 1;
	posArgs.z_lift_heights[0] = 1;
	posArgs.z_lift_heights[1] = .8;
	posArgs.z_lift_heights[2] = .6;
	posArgs.z_lift_heights[3] = .4;
	posArgs.z_lift_heights[4] = .2;
	posArgs.x_firmware_offsets[0] = 0;
	posArgs.y_firmware_offsets[0] = 1;
	posArgs.x_firmware_offsets[1] = 2;
	posArgs.y_firmware_offsets[1] = 3;
	posArgs.x_firmware_offsets[2] = 4;
	posArgs.y_firmware_offsets[2] = 5;
	posArgs.x_firmware_offsets[3] = 6;
	posArgs.y_firmware_offsets[3] = 7;
	posArgs.x_firmware_offsets[4] = 8;
	posArgs.y_firmware_offsets[4] = 9;
	posArgs.default_extruder = 0;
	posArgs.priming_height = 0.4;
	posArgs.minimum_layer_height = 0.05;
	posArgs.g90_influences_extruder = false;
	posArgs.xyz_axis_default_mode = "absolute";
	posArgs.e_axis_default_mode = "absolute";
	posArgs.units_default = "millimeters";
	posArgs.location_detection_commands = std::vector<std::string>();
	posArgs.is_bound_ = true;
	posArgs.is_circular_bed = false;
	posArgs.snapshot_x_min = 0;
	posArgs.snapshot_x_max = 250;
	posArgs.snapshot_y_min = 0;
	posArgs.snapshot_y_max = 210;
	posArgs.snapshot_z_min = 0;
	posArgs.snapshot_z_max = 200;
	posArgs.x_min = 0;
	posArgs.x_max = 250;
	posArgs.y_min = -3;
	posArgs.y_max = 210;
	posArgs.z_min = 0;
	posArgs.z_max = 200;
	return posArgs;
}

static gcode_position_args get_5_extruder_position_args()
{
	gcode_position_args posArgs = gcode_position_args();
	posArgs.autodetect_position = true;
	posArgs.home_x = 0;
	posArgs.home_x_none = true;
	posArgs.home_y = 0;
	posArgs.home_y_none = true;
	posArgs.home_z = 0;
	posArgs.home_z_none = true;
	posArgs.shared_extruder = false;
	posArgs.zero_based_extruder = true;
	posArgs.set_num_extruders(5);
	posArgs.retraction_lengths[0] = .2;
	posArgs.retraction_lengths[1] = .4;
	posArgs.retraction_lengths[2] = .6;
	posArgs.retraction_lengths[3] = .8;
	posArgs.retraction_lengths[4] = 1;
	posArgs.z_lift_heights[0] = 1;
	posArgs.z_lift_heights[1] = .8;
	posArgs.z_lift_heights[2] = .6;
	posArgs.z_lift_heights[3] = .4;
	posArgs.z_lift_heights[4] = .2;
	posArgs.x_firmware_offsets[0] = 0;
	posArgs.y_firmware_offsets[0] = 0;
	posArgs.x_firmware_offsets[1] = 5;
	posArgs.y_firmware_offsets[1] = 0;
	posArgs.x_firmware_offsets[2] = 0;
	posArgs.y_firmware_offsets[2] = 0;
	posArgs.x_firmware_offsets[3] = 0;
	posArgs.y_firmware_offsets[3] = 0;
	posArgs.x_firmware_offsets[4] = 0;
	posArgs.y_firmware_offsets[4] = 0;
	posArgs.default_extruder = 0;
	posArgs.priming_height = 0.4;
	posArgs.minimum_layer_height = 0.05;
	posArgs.g90_influences_extruder = false;
	posArgs.xyz_axis_default_mode = "absolute";
	posArgs.e_axis_default_mode = "absolute";
	posArgs.units_default = "millimeters";
	posArgs.location_detection_commands = std::vector<std::string>();
	posArgs.is_bound_ = true;
	posArgs.is_circular_bed = false;
	posArgs.snapshot_x_min = 0;
	posArgs.snapshot_x_max = 250;
	posArgs.snapshot_y_min = 0;
	posArgs.snapshot_y_max = 210;
	posArgs.snapshot_z_min = 0;
	posArgs.snapshot_z_max = 200;
	posArgs.x_min = 0;
	posArgs.x_max = 250;
	posArgs.y_min = -3;
	posArgs.y_max = 210;
	posArgs.z_min = 0;
	posArgs.z_max = 200;
	return posArgs;
}

static void TestAntiStutter(std::string filePath)
{
	double max_resolution = DEFAULT_RESOLUTION_MM;
	double max_radius_mm = DEFAULT_MAX_RADIUS_MM;
	std::vector<std::string> logger_names;
	logger_names.push_back("arc_welder.gcode_conversion");
	std::vector<int> logger_levels;
	//logger_levels.push_back(log_levels::DEBUG);
	logger_levels.push_back(log_levels::INFO);
	logger* p_logger = new logger(logger_names, logger_levels);
	p_logger->set_log_level(VERBOSE);
	//arc_welder arc_welder_obj(BENCHY_0_5_MM_NO_WIPE, "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\test_output.gcode", p_logger, max_resolution, false, 50, static_cast<progress_callback>(on_progress));
	//arc_welder arc_welder_obj(SIX_SPEED_TEST, "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\test_output.gcode", p_logger, max_resolution, false, 50, on_progress);
	arc_welder arc_welder_obj(BENCHY_L1_DIFFICULT, "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\test_output.gcode", p_logger, max_resolution, max_radius_mm, false, 50, on_progress);
	//BENCHY_LAYER_1GCODE
	//SMALL_TEST
	//FACE_SHIELD
	//BENCHY_LAYER_1_NO_WIPE
	//BENCHY_0_5_MM_NO_WIPE
	//BENCHY_CURA_RELATIVE_E_NOWIPE
	//BENCHY_GYROID_ABSOLUTE_E_NOWIPE
	//BENCHY_GYROID_RELATIVE_E_NOWIPE
	//BENCHY_STACK_RELATIVE
	//BENCHY_STACK_ABSOLUTE
	//FRACTAL
	//SUPER_HUGE_TEST
	//TORTURE_TEST
	//ORCHID_POD
	//DIFFICULT_CURVES
	//ISSUE_PRICKLYPEAR_LAYER_0_114
	//BARBARIAN
	// BENCHY_L1_DIFFICULT
	arc_welder_results results = arc_welder_obj.process();
	p_logger->log(0, INFO, results.progress.detail_str());
	p_logger->log(0, INFO, "Processing Complete.");
	delete p_logger;
}

static bool on_progress(arc_welder_progress progress)
{
	std::cout << progress.str() << "\r\n";
	return true;
}

void TestDoubleToString()
{
	char buffer[100];

	
	for (int index = 0; index < 1000; index++)
	{
		double r = (double)rand() / RAND_MAX;
		r = -1000000.0 + r * (1000000.0 - -1000000.0);
		std::cout << std::fixed << std::setprecision(10) << "Number: " << r << std::endl;
		utilities::to_string(r, 5, buffer);
		std::cout << buffer << std::endl;
	}
	
}

static void TestParsingCase()
{
	gcode_parser parser;
	//parsed_command command = parser.parse_gcode("  G0 X1 y2 ; test", true);
	parsed_command command2 = parser.parse_gcode(" M73 P0 R93", true);
	//parsed_command command2 = parser.parse_gcode("M204 P2000 R1500 T2000 ; sets acceleration (P, T) and retract acceleration (R), mm/sec^2", true);
	parsed_command command3 = parser.parse_gcode("G0 X1 y2; test", true);
	

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Test Application
//
// This application is only used for ad-hoc testing of the anti-stutter library.
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
#include "ArcWelderTest.h"
#include "logger.h"
#include <iostream>

int main(int argc, char* argv[])
{
	run_tests(argc, argv);
}

int run_tests(int argc, char* argv[])
{

	_CrtMemState state1, state2, state3;
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
	_CrtMemCheckpoint(&state1);

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
		/*
		if (!TestProblemDoubles())
		{
			std::cout << "Test Failed!" << std::endl;
		}

		if (!TestIntToStringRandom(-1000000, 1000000, 1000000))
		{
			std::cout << "Test Failed!" << std::endl;
		}

		
		if (!TestDoubleToStringRandom(-0.5, 0.5, 1000000))
		{
			std::cout << "Test Failed!" << std::endl;
		}
		if (!TestDoubleToStringRandom(-100, 100, 1000000))
		{
			std::cout << "Test Failed!" << std::endl;
		}
		if (!TestDoubleToStringRandom(-1, 1, 1000000))
		{
			std::cout << "Test Failed!" << std::endl;
		}
		if (!TestDoubleToStringRandom(-1000000, 1000000, 1000000))
		{
			std::cout << "Test Failed!" << std::endl;
		}		
		*/
	
		 

	}
	auto end = std::chrono::high_resolution_clock::now();
	_CrtMemCheckpoint(&state2);
	if (_CrtMemDifference(&state3, &state1, &state2)) {
		_CrtMemDumpStatistics(&state3);
	}
	//_CrtMemDumpAllObjectsSince(&state);
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
	std::vector<std::string> logger_names;
	logger_names.push_back("arc_welder.gcode_conversion");
	std::vector<int> logger_levels;
	logger_levels.push_back((int)log_levels::NOSET);
	logger_levels.push_back((int)log_levels::VERBOSE);
	logger_levels.push_back((int)log_levels::DEBUG);
	logger_levels.push_back((int)log_levels::INFO);
	logger_levels.push_back((int)log_levels::WARNING);
	//logger_levels.push_back((int)(log_levels::ERROR));
	logger_levels.push_back((int)log_levels::CRITICAL);
	logger* p_logger = new logger(logger_names, logger_levels);
	p_logger->set_log_level(log_levels::INFO);
	
	//FIRMWARE_COMPENSATION_TEST_1
	//BENCHY_MIN_RADIUS_TEST
	//BENCHY_DIFFICULT
	//BENCHY_LAYER_1GCODE
	//SMALL_TEST
	//FACE_SHIELD
	//BENCHY_LAYER_1_NO_WIPE
	//BENCHY_0_5_MM_NO_WIPE
	//BENCHY_CURA_RELATIVE_E_NOWIPE
	//BENCHY_GYROID_ABSOLUTE_E_NOWIPEd
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
	// SPIRAL_TEST
	// SPIRAL_VASE_TEST_FUNNEL
	std::string source_path = SUPER_HUGE_TEST;
	std::string target_path = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\test_output.gcode";
	arc_welder_args args(source_path, target_path, p_logger);
	args.box_encoding = args.box_encoding = utilities::box_drawing::ASCII;
	args.callback = on_progress;
  // override any arguments here;
	args.allow_travel_arcs = true;
	args.allow_3d_arcs = true;
	args.max_radius_mm = 9999;
	args.resolution_mm = 0.05;
	args.extrusion_rate_variance_percent = 1000;
	arc_welder arc_welder_obj(args);
		
	arc_welder_results results = arc_welder_obj.process();
	p_logger->log(0, log_levels::INFO, results.progress.detail_str());
	p_logger->log(0, log_levels::INFO, "Processing Complete.");
	delete p_logger;
}

bool on_progress(arc_welder_progress progress, logger * p_logger, int logger_type)
{
	p_logger->log(logger_type, log_levels::INFO, progress.str());
	return true;
}

static void TestParsingCase()
{
	gcode_parser parser;
	//parsed_command command = parser.parse_gcode("  G0 X1 y2 ; test", true);
	parsed_command command2 = parser.parse_gcode(" M73 P0 R93", true);
	//parsed_command command2 = parser.parse_gcode("M204 P2000 R1500 T2000 ; sets acceleration (P, T) and retract acceleration (R), mm/sec^2", true);
	parsed_command command3 = parser.parse_gcode("G0 X1 y2; test", true);
}

bool TestIntToStringRandom(int low, int high, int num_runs)
{
	bool all_success = true;
	for (int index = 0; index < num_runs; index++)
	{
		int value = utilities::rand_range(low, high);
		unsigned char precision = utilities::rand_range(static_cast<unsigned char>(0), static_cast<unsigned char>(6));
		if (!CompareDoubleToStringResult(static_cast<double>(value), precision))
		{
			all_success = false;
		}
	}
	return all_success;
}


bool TestDoubleToStringRandom(double low, double high, int num_runs)
{
	bool all_success = true;
	for (int index = 0; index < num_runs; index++)
	{
		double value = utilities::rand_range(low, high);
		unsigned char precision = utilities::rand_range(static_cast<unsigned char>(0), static_cast<unsigned char>(6));
		if (!CompareDoubleToStringResult(value, precision))
		{
			all_success = false;
		}
	}
	return all_success;
}

bool CompareDoubleToStringResult(double value, unsigned char precision)
{
	
	std::ostringstream stream;
	stream << std::fixed;
	stream << std::setprecision(precision) << value;
	//std::cout << std::fixed << "Testing: " << std::setprecision(12) << value << " precision: " << std::setprecision(0) << static_cast <int> (precision);
	std::string test_string = utilities::dtos(value, precision);
	if (test_string != stream.str())
	{
		std::cout << std::fixed << "Failed to convert: " << std::setprecision(24) << value << " Precision:" << std::setprecision(0) << static_cast <int> (precision) << " String:" << test_string << " Stream:" << stream.str() << std::endl;
		return false;
	}
	//std::cout << std::endl;
	return true;
	
}

bool TestProblemDoubles()
{
	bool result = true;
	result = result && CompareDoubleToStringResult(-0.000030518509475996325, 4);
	result = result && CompareDoubleToStringResult(0.500000000000000000000000, static_cast<unsigned int>(2));
	result = result && CompareDoubleToStringResult(9.9999999999999, static_cast<unsigned int>(2));
	result = result && CompareDoubleToStringResult(9.9950, static_cast<unsigned int>(2));
	result = result && CompareDoubleToStringResult(39.6, static_cast<unsigned int>(3));

	result = result && CompareDoubleToStringResult(39.600000000000001421085472, static_cast<unsigned int>(3));
	result = result && CompareDoubleToStringResult(40.228999999999999204192136, static_cast<unsigned int>(3));
	
	
	return result;
}
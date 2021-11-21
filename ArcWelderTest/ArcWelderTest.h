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
#pragma once
#include <stdlib.h>
#include <crtdbg.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include "gcode_position.h"
#include "gcode_parser.h"
#include <sstream>
#include "arc_welder.h"
#include "array_list.h"
#include "logger.h"
#include <exception>

int run_tests(int argc, char* argv[]);
static gcode_position_args get_single_extruder_position_args();
static gcode_position_args get_5_shared_extruder_position_args();
static gcode_position_args get_5_extruder_position_args();
static void TestAntiStutter(std::string filePath);
bool on_progress(arc_welder_progress progress, logger* p_logger, int logger_type);
static void TestParsingCase();
bool CompareDoubleToStringResult(double value, unsigned char precision);
bool TestIntToStringRandom(int low, int high, int num_runs);
bool TestDoubleToStringRandom(double low, double high, int num_runs);
bool TestProblemDoubles();

static std::string ANTI_STUTTER_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\5x5_cylinder_2000Fn_0.2mm_PLA_MK2.5MMU2_4m.gcode";
static std::string BENCHY_GCODE = "C:\\Users\\Brad\\Documents\\3DPrinter\\Calibration\\Benchy\\3DBenchy_0.2mm_PLA_MK2.5MMU2.gcode";
static std::string BENCHY_CURA_RELATIVE_E_NOWIPE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\3DBenchy_CuraRelative_Gyroid_0.2mm.gcode";
static std::string BENCHY_GYROID_RELATIVE_E_NOWIPE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\3DBenchy_0.2mm_gyroid_relative_e_NoWipe.gcode";
static std::string BENCHY_GYROID_ABSOLUTE_E_NOWIPE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\3DBenchy_Absolute_Gyroid_0.2mm.gcode";
static std::string BENCHY_0_5_MM_NO_WIPE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Benchy_0.5mm_NoWipe.gcode";
static std::string BENCHY_LAYER_1GCODE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\benchy_l1.gcode";
static std::string BENCHY_LAYER_1_NO_WIPE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\benchy_L1_NoWipe.gcode";
static std::string BENCHY_STACK_RELATIVE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyStack_Relative.gcode";
static std::string BENCHY_STACK_ABSOLUTE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyStack_Absolute.gcode";
static std::string CAM_RING_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\5milCamRing_0.2mm_PLA_MK2.5MMU2_16m.gcode";
static std::string FRACTAL = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Mandelbrot.gcode";
static std::string DIFFICULT_CURVES = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\DifficultCurves.gcode";
static std::string FACE_SHIELD = "C:\\Users\\Brad\\Documents\\3DPrinter\\corona_virus\\2X_Visor_Frame_0.35mm_PLA_1h25m.gcode";
static std::string SMALL_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\small_test.gcode";
static std::string SUPER_HUGE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\super_huge_file.gcode";
//static std::string TORTURE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\stereographic_projection_0.2mm_PLA_MK2.5MMU2_2h49m.gcode";
static std::string ORCHID_POD = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Pla_OrchidPot.gcode";
static std::string BENCHY_DIFFICULT = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyArc_Difficult.gcode";
static std::string BENCHY_L1_DIFFICULT = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyArc_L1_Difficult.gcode";


static std::string SIX_SPEED_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\6_speed_test.gcode";
// Issues
static std::string ISSUE_MIMUPREFERIDA = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\MIMUPREFERIDA\\TESTSTUTTER.gcode";
static std::string BARBARIAN = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\PricklyPear\\Barbarian.gcode";
static std::string BAD_ARC = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\PricklyPear\\bad_arc.gcode";
static std::string ISSUE_PRICKLYPEAR_LAYER_0_114 = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\PricklyPear\\Layers0_114.gcode";
// Sanity tests
static std::string COLINEAR_TEST_1 = "C:\\Users\\Brad\\Documents\\AntiStutter\\Sanity Checks\\G2_colinear_test.gcode";
static std::string SPIRAL_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\smoothietest\\SPIRAL_TEST.gcode";
static std::string SPIRAL_TRAVEL_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\smoothietest\\SPIRAL_TRAVEL_TEST.gcode";
static std::string TravelWipeTest = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\smoothietest\\TravelWipeTest.gcode";
static std::string SPIRAL_TEST_PRECISION = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\smoothietest\\SPIRAL_TEST_precision.gcode";
static std::string SPIRAL_VASE_TEST_DOUBLE_SPIRAL = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\SpiralVaseTest\\SpiralVaseTest_DOUBLE_SPIRAL.gcode"; 
static std::string SPIRAL_VASE_TEST_CYLINDER = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\SpiralVaseTest\\SpiralVaseTest_Cylinder.gcode";
static std::string SPIRAL_VASE_TEST_SINGLE_LAYER_CYLINDER = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\SpiralVaseTest\\SpiralVaseSingleLayer_Cylinder.gcode";
static std::string SPIRAL_VASE_TEST_PI_BOWL = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\SpiralVaseTest\\SpiralVaseTest_PiBowl.gcode";

static std::string FIRMWARE_COMPENSATION_TEST_1 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\cylinder_tests\\cylinder_test_0.1_5.0_0.1.gcode";
static std::string BENCHY_MIN_RADIUS_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyMinRadiusTest.gcode";
static std::string ISSUE_93 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Issues\\93\\FailingGCode.gcode";
static std::string ISSUE_99 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Issues\\99\\FailingGCode.gcode";
static std::string ISSUE_134 = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\134\\BirdHouse [PETG] [brim]+Infill [20%,cubic]+Noz [0.6]+LH [0.2]+LW-[0.6]+Temps [240+70]+50.0mms+Support [normal (56)]+Coast-[False].gcode" ;
static std::string ISSUE_170 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Issues\\170\\lampenring.gcode";
static std::string ISSUE_184 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Issues\\184\\Dome_60_1.2mm_max_test15_0.2mm_PETG_MK3S_2h1m.gcode";

static std::string CONE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\ConeTest.gcode";
static std::string CONE_TEST_VASE = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\ConeTestVase.gcode";
static std::string UNICODE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BenchyMinRadiusTest_with_unicode.gcode";

static std::string BAD_ARC_DIRECTIONS = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BadArcDirections.gcode";
static std::string BAD_ARC_DIRECTIONS_2 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\BadArcDirections2.gcode";
static std::string WIPER_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\wiper_test.gcode";
static std::string SLOW_COUPLER = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Rob_Coupler.gcode";
static std::string ISSUE_34 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\spacer.gcode";
static std::string DIFFICULT_ARCS_ISSUE_34 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\DifficultArcs\\issue_34.gcode";

static std::string TORTURE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\performance\\torture_test.gcode";
static std::string CURA_PLUGIN_ISSUE_18 = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Issues\\CuraPlugin\\18\\Unwelded.gcode";
static std::string METALTEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\MetalAltered.gcode";
static std::string TEMP_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\temp.gcode";





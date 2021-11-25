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
#include <string>
#include <vector>
int run_arc_straightener(int argc, char* argv[]);
static void check_firmware_version_for_type(std::string firmware_type_string, std::string firmware_version, std::string firmware_version_arg_name);
static int get_firmware_type_from_string(std::string firmware_type);
static void print_firmware_defaults(std::string firmware_type_string, std::string firmware_version_string, std::string firmware_version_arg_name);

/*
static void TestInverseProcessor(std::string source_path, std::string target_path);
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
static std::string TORTURE_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\stereographic_projection_0.2mm_PLA_MK2.5MMU2_2h49m.gcode";
static std::string ORCHID_POD = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\Pla_OrchidPot.gcode";
static std::string ARC_GYROID_BENCHY_DIFFICULT = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\AS_BenchyArc_Difficult.gcode";
static std::string SIX_SPEED_TEST = "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\SixSpeedTest_AS.gcode";


// Issues
static std::string ISSUE_MIMUPREFERIDA = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\MIMUPREFERIDA\\TESTSTUTTER.gcode";
static std::string ISSUE_PRICKLYPEAR = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\PricklyPear\\Barbarian.gcode";
static std::string ISSUE_PRICKLYPEAR_LAYER_0_114 = "C:\\Users\\Brad\\Documents\\AntiStutter\\Issues\\PricklyPear\\Layers0_114.gcode";
*/
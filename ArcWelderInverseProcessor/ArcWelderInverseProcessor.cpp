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

#include <iostream>
#include <sstream>
#include "inverse_processor.h"
#include "ArcWelderInverseProcessor.h"

int main(int argc, char* argv[])
{
	std::string info = "Arc Welder: Inverse Processor v0.1.rc1.dev0\nConverts G2/G3 commands to G1/G2 commands.\nCopyright(C) 2020 - Brad Hochgesang\n";
	std::cout << info;
	TestInverseProcessor(SIX_SPEED_TEST, "C:\\Users\\Brad\\Documents\\3DPrinter\\AntiStutter\\test_output.gcode");
}

static void TestInverseProcessor(std::string source_path, std::string target_path)
{
	inverse_processor processor(source_path, target_path, false, 50);
	processor.process();
}
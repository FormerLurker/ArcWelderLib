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
#include "firmware.h"
#include <cstring>
#include <fstream>
#include "gcode_position.h"

#define DEFAULT_GCODE_BUFFER_SIZE 50
struct arc_interpolation_args
{
	arc_interpolation_args()
	{
		
		source_path = "";
		target_path = "";
	}
	/// <summary>
	/// Firmware arguments.  Not all options will apply to all firmware types.
	/// </summary>
	firmware_arguments	firmware_args;
	/// <summary>
	/// Required: the path to the source file containing G2/G3 commands.
	/// </summary>
	std::string source_path;
	/// <summary>
	/// Optional: the path to the target file.  If left blank the source file will be overwritten by the target.
	/// </summary>
	std::string target_path;
	
};

class arc_interpolation
{																																																							
	
	public:
		arc_interpolation();
		arc_interpolation(arc_interpolation_args args);
		virtual ~arc_interpolation();
		void process();
		/// <summary>
		/// Outputs a string description of the firmware arguments.
		/// </summary>
		/// <returns></returns>
		std::string get_firmware_arguments_description(std::string separator = "", std::string argument_prefix = "", std::string replacement_string = "", std::string replacement_value = "") const;
	private:
			arc_interpolation_args args_;
			gcode_position_args get_args_(bool g90_g91_influences_extruder, int buffer_size);
			std::string source_path_;
			std::string target_path_;
			gcode_position* p_source_position_;
			std::ofstream output_file_;
			int lines_processed_ = 0;
			firmware* p_current_firmware_;
			int num_arc_commands_;
  
};


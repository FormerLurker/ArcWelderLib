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
#pragma once

#include <string>
#include "gcode_position.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
#define M_PI       3.14159265358979323846f   // pi
enum AxisEnum { X_AXIS = 0, Y_AXIS= 1, Z_AXIS = 2, E_AXIS = 3, X_HEAD = 4, Y_HEAD = 5 };
// Arc interpretation settings:
#define MM_PER_ARC_SEGMENT 2.0f // REQUIRED - The enforced maximum length of an arc segment
#define MIN_MM_PER_ARC_SEGMENT 0.2f /* OPTIONAL - the enforced minimum length of an interpolated segment.  Must be smaller than
	MM_PER_ARC_SEGMENT if defined.  Only has an effect if MIN_ARC_SEGMENTS or ARC_SEGMENTS_PER_SEC is defined */
	// If both MIN_ARC_SEGMENTS and ARC_SEGMENTS_PER_SEC is defined, the minimum calculated segment length is used.
#define MIN_ARC_SEGMENTS 20 // OPTIONAL - The enforced minimum segments in a full circle of the same radius.
#define ARC_SEGMENTS_PER_SEC 40 // OPTIONAL - Use feedrate to choose segment length.
#define N_ARC_CORRECTION 25 // OPTIONAL - The number of interpolated segments that will be generated without a floating point correction
//#define ARC_EXTRUSION_CORRECTION // If defined, we should apply correction to the extrusion length based on the
								   // difference in true arc length.  The correctly is extremely small, and may not be worth the cpu cycles

class inverse_processor {
public:
	inverse_processor(std::string source_path, std::string target_path, bool g90_g91_influences_extruder, int buffer_size);
	virtual ~inverse_processor();
	void process();
	std::string mc_arc(float* position, float* target, float* offset, uint8_t axis_0, uint8_t axis_1,
		uint8_t axis_linear, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder, bool output_relative);
private:
	gcode_position_args get_args_(bool g90_g91_influences_extruder, int buffer_size);
	std::string source_path_;
	std::string target_path_;
	gcode_position* p_source_position_;
	
	float arc_max_radius_threshold;
	//float arc_min_radius_threshold;
	float total_e_adjustment;
	int trig_calc_count = 0;
};




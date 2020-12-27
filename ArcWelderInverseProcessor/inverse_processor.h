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
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
#define M_PI       3.14159265358979323846   // pi
enum AxisEnum { X_AXIS = 0, Y_AXIS= 1, Z_AXIS = 2, E_AXIS = 3, X_HEAD = 4, Y_HEAD = 5 };
// Arc interpretation settings:
#define DEFAULT_MM_PER_ARC_SEGMENT 1.0 // REQUIRED - The enforced maximum length of an arc segment
#define DEFAULT_MIN_MM_PER_ARC_SEGMENT 0 /* OPTIONAL - the enforced minimum length of an interpolated segment.  Must be smaller than
	MM_PER_ARC_SEGMENT.  Only has an effect if MIN_ARC_SEGMENTS > 0 or ARC_SEGMENTS_PER_SEC > 0 */
	// If both MIN_ARC_SEGMENTS and ARC_SEGMENTS_PER_SEC is defined, the minimum calculated segment length is used.
#define DEFAULT_MIN_ARC_SEGMENTS 0 // OPTIONAL - The enforced minimum segments in a full circle of the same radius.
#define DEFAULT_ARC_SEGMENTS_PER_SEC 0 // OPTIONAL - Use feedrate to choose segment length.
// approximation will not be used for the first segment.  Subsequent segments will be corrected following DEFAULT_N_ARC_CORRECTION.
#define DEFAULT_N_ARC_CORRECTIONS 24

struct ConfigurationStore {
	ConfigurationStore() {
		mm_per_arc_segment = DEFAULT_MM_PER_ARC_SEGMENT;
		min_mm_per_arc_segment = DEFAULT_MIN_MM_PER_ARC_SEGMENT;
		min_arc_segments = DEFAULT_MIN_ARC_SEGMENTS;
		arc_segments_per_sec = DEFAULT_ARC_SEGMENTS_PER_SEC;
		n_arc_correction = DEFAULT_N_ARC_CORRECTIONS;
	}
	float mm_per_arc_segment; // This value is ALWAYS used.
	float min_mm_per_arc_segment;  // if less than or equal to 0, this is disabled
	int min_arc_segments; // If less than or equal to zero, this is disabled
	double arc_segments_per_sec; // If less than or equal to zero, this is disabled
	int n_arc_correction;
	
};
class inverse_processor {
public:
	inverse_processor(std::string source_path, std::string target_path, bool g90_g91_influences_extruder, int buffer_size, ConfigurationStore cs = ConfigurationStore());
	virtual ~inverse_processor();
	void process();
	void mc_arc(float* position, float* target, float* offset, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder);
	
private:
	ConfigurationStore cs_;
	gcode_position_args get_args_(bool g90_g91_influences_extruder, int buffer_size);
	std::string source_path_;
	std::string target_path_;
	gcode_position* p_source_position_;
	std::ofstream output_file_;
	bool output_relative_;
	float arc_max_radius_threshold;
	//float arc_min_radius_threshold;
	float total_e_adjustment;
	int trig_calc_count = 0;
	int lines_processed_ = 0;
	void clamp_to_software_endstops(float* target);

	void plan_buffer_line(float x, float y, bool has_z, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target=NULL);
	
};






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


#include "marlin_1.h"
#include "utilities.h"
marlin_1::marlin_1(firmware_arguments args) : firmware(args) {
	feedrate_mm_s = 0;
	current_position = new float[MARLIN_XYZE];
	apply_arguments();
};

marlin_1::~marlin_1()
{
	delete current_position;
}

void marlin_1::apply_arguments()
{
	static const std::vector<std::string> marlin_1_firmware_version_names{
		 "1.1.9.1"
	};
	set_versions(marlin_1_firmware_version_names, "1.1.9.1");
	marlin_1_version_ = (marlin_1::marlin_1_firmware_versions)version_index_;
	std::vector<std::string> used_arguments;
	/* Add case statement if we ever add any additional firmware versions
	switch (marlin_1_version_)
	{
	default:*/
	plan_arc_ = &marlin_1::plan_arc_1_1_9_1;
	used_arguments = { FIRMWARE_ARGUMENT_MM_PER_ARC_SEGMENT, FIRMWARE_ARGUMENT_N_ARC_CORRECTION, FIRMWARE_ARGUMENT_G90_G91_INFLUENCES_EXTRUDER };
	//break;
  //}

	args_.set_used_arguments(used_arguments);
}

firmware_arguments marlin_1::get_default_arguments_for_current_version() const
{
	// Start off with the current args so they are set up correctly for this firmware type and version
	firmware_arguments default_args = args_;

	// firmware defaults
	default_args.g90_g91_influences_extruder = false;
	// Add the switch in here in case we want to add more versions.
	//switch (marlin_1_version_)
	//{
	//default:
	  // Active Settings
	default_args.mm_per_arc_segment = 1.0f;
	default_args.n_arc_correction = 25;
	//break;
  //}

	return default_args;
}

std::string marlin_1::interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise)
{
	// Clear the current list of gcodes
	gcodes_.clear();

	// Setup the current position
	current_position[X_AXIS] = static_cast<float>(position_.x);
	current_position[Y_AXIS] = static_cast<float>(position_.y);
	current_position[Z_AXIS] = static_cast<float>(position_.z);
	current_position[E_AXIS] = static_cast<float>(position_.e);
	float marlin_target[MARLIN_XYZE];
	marlin_target[X_AXIS] = static_cast<float>(target.x);
	marlin_target[Y_AXIS] = static_cast<float>(target.y);
	marlin_target[Z_AXIS] = static_cast<float>(target.z);
	marlin_target[E_AXIS] = static_cast<float>(target.e);
	float marlin_offset[2];
	marlin_offset[0] = static_cast<float>(i);
	marlin_offset[1] = static_cast<float>(j);
	// TODO:  handle R form!!

	// Set the feedrate
	feedrate_mm_s = static_cast<float>(target.f);
	uint8_t marlin_isclockwise = is_clockwise ? 1 : 0;

	(this->*plan_arc_)(marlin_target, marlin_offset, marlin_isclockwise);

	return gcodes_;
}

/// <summary>
/// This function was adapted from the 1.1.9.1 release of Marlin firmware, which can be found at the following link:
/// https://github.com/MarlinFirmware/Marlin/blob/1314b31d97bba8cd74c6625c47176d4692f57790/Marlin/Marlin_main.cpp
/// Copyright Notice found on that page:
/// 
/// 
/// Marlin 3D Printer Firmware
/// Copyright (C) 2016, 2017 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
/// 
/// Based on Sprinter and grbl.
/// Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
/// 
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
/// </summary>
/// <param name="cart">The target position</param>
/// <param name="offset">The I and J offset</param>
/// <param name="clockwise">Is the motion clockwise or counterclockwise</param>
void marlin_1::plan_arc_1_1_9_1(const float(&cart)[MARLIN_XYZE], // Destination position
	const float(&offset)[2], // Center of rotation relative to current_position
	const bool clockwise      // Clockwise?
)
{
	// cnc workspace planes variables -- Note:  This is NOT implemented, but is added for completeness in case it is in the future.
	int active_extruder = 0;
	AxisEnum p_axis, q_axis, l_axis;
	p_axis = X_AXIS, q_axis = Y_AXIS, l_axis = Z_AXIS;


	// Radius vector from center to current location
	float r_P = -offset[0], r_Q = -offset[1];

	const float radius = utilities::hypotf(r_P, r_Q),
		center_P = current_position[p_axis] - r_P,
		center_Q = current_position[q_axis] - r_Q,
		rt_X = cart[p_axis] - center_P,
		rt_Y = cart[q_axis] - center_Q,
		linear_travel = cart[l_axis] - current_position[l_axis],
		extruder_travel = cart[E_CART] - current_position[E_CART];

	// CCW angle of rotation between position and target from the circle center. Only one atan2() trig computation required.
	float angular_travel = (float)utilities::atan2((double)r_P * rt_Y - (double)r_Q * rt_X, (double)r_P * rt_X + (double)r_Q * rt_Y);
	if (angular_travel < 0) angular_travel += utilities::radiansf(360.0f);
	if (clockwise) angular_travel -= utilities::radiansf(360.0f);

	// Make a circle if the angular rotation is 0 and the target is current position
	if (angular_travel == 0 && current_position[p_axis] == cart[p_axis] && current_position[q_axis] == cart[q_axis])
		angular_travel = utilities::radiansf(360.0f);

	const float flat_mm = radius * angular_travel,
		mm_of_travel = linear_travel ? utilities::hypotf(flat_mm, linear_travel) : utilities::absf(flat_mm);
	if (mm_of_travel < 0.001f) return;

	uint16_t segments = (uint16_t)utilities::floorf(mm_of_travel / (float)(args_.mm_per_arc_segment));
	NOLESS(segments, 1);

	/**
	 * Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
	 * and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
	 *     r_T = [cos(phi) -sin(phi);
	 *            sin(phi)  cos(phi)] * r ;
	 *
	 * For arc generation, the center of the circle is the axis of rotation and the radius vector is
	 * defined from the circle center to the initial position. Each line segment is formed by successive
	 * vector rotations. This requires only two cos() and sin() computations to form the rotation
	 * matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
	 * all double numbers are single precision on the Arduino. (True double precision will not have
	 * round off issues for CNC applications.) Single precision error can accumulate to be greater than
	 * tool precision in some cases. Therefore, arc path correction is implemented.
	 *
	 * Small angle approximation may be used to reduce computation overhead further. This approximation
	 * holds for everything, but very small circles and large MM_PER_ARC_SEGMENT values. In other words,
	 * theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
	 * to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
	 * numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
	 * issue for CNC machines with the single precision Arduino calculations.
	 *
	 * This approximation also allows plan_arc to immediately insert a line segment into the planner
	 * without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
	 * a correction, the planner should have caught up to the lag caused by the initial plan_arc overhead.
	 * This is important when there are successive arc motions.
	 */
	 // Vector rotation matrix values
	float raw[MARLIN_XYZE];
	const float theta_per_segment = angular_travel / segments,
		linear_per_segment = linear_travel / segments,
		extruder_per_segment = extruder_travel / segments,
		sin_T = theta_per_segment,
		cos_T = 1 - 0.5f * utilities::sqf(theta_per_segment); // Small angle approximation

	// Initialize the linear axis
	raw[l_axis] = current_position[l_axis];

	// Initialize the extruder axis
	raw[E_CART] = current_position[E_CART];

	const float fr_mm_s = MMS_SCALED(feedrate_mm_s);

	int8_t arc_recalc_count = 0;
	if (args_.n_arc_correction > 1)
	{
		arc_recalc_count = args_.n_arc_correction;
	}


	for (uint16_t i = 1; i < segments; i++) // Iterate (segments-1) times
	{

		if (args_.n_arc_correction > 1 && --arc_recalc_count)
		{
			// Apply vector rotation matrix to previous r_P / 1
			const float r_new_Y = r_P * sin_T + r_Q * cos_T;
			r_P = r_P * cos_T - r_Q * sin_T;
			r_Q = r_new_Y;
		}
		else
		{
			if (args_.n_arc_correction > 1)
			{
				arc_recalc_count = args_.n_arc_correction;
			}

			// Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
			// Compute exact location by applying transformation matrix from initial radius vector(=-offset).
			// To reduce stuttering, the sin and cos could be computed at different times.
			// For now, compute both at the same time.
			const float cos_Ti = (float)utilities::cos(i * (double)theta_per_segment), sin_Ti = (float)utilities::sin(i * (double)theta_per_segment);
			r_P = -offset[0] * cos_Ti + offset[1] * sin_Ti;
			r_Q = -offset[0] * sin_Ti - offset[1] * cos_Ti;
		}

		// Update raw location
		raw[p_axis] = center_P + r_P;
		raw[q_axis] = center_Q + r_Q;
		raw[l_axis] += linear_per_segment;
		raw[E_CART] += extruder_per_segment;

		clamp_to_software_endstops(raw);


		if (!buffer_line_kinematic(raw, feedrate_mm_s, active_extruder))
			break;
	}

	buffer_line_kinematic(cart, feedrate_mm_s, active_extruder);

	COPY(current_position, cart);
}

void marlin_1::NOLESS(uint16_t &x, uint16_t y)
{
	if (x < y)
		x = y;
}

float marlin_1::MMS_SCALED(float x)
{
	// No scaling
	return x;
}

void marlin_1::COPY(float target[MARLIN_XYZE], const float(&source)[MARLIN_XYZE])
{
	// This is a slow copy, but speed isn't much of an issue here.
	for (int i = 0; i < MARLIN_XYZE; i++)
	{
		target[i] = source[i];
	}
}


void marlin_1::clamp_to_software_endstops(const float(&raw)[MARLIN_XYZE])
{
	// Do nothing, just added to keep mc_arc identical to the firmware version
	return;
}

//void marlin::buffer_line_kinematic(float x, float y, float z, const float& e, float feed_rate, uint8_t extruder, const float* gcode_target)
bool marlin_1::buffer_line_kinematic(const float(&cart)[MARLIN_XYZE], double fr_mm_s, int active_extruder)
{

	// create the target position
	firmware_position target;
	target.x = cart[AxisEnum::X_AXIS];
	target.y = cart[AxisEnum::Y_AXIS];
	target.z = cart[AxisEnum::Z_AXIS];
	target.e = cart[AxisEnum::E_AXIS];
	target.f = fr_mm_s;
	if (gcodes_.size() > 0)
	{
		gcodes_ += "\n";
	}
	// Generate the gcode
	gcodes_ += g1_command(target);

	// update the current position
	set_current_position(target);
	return true;
}

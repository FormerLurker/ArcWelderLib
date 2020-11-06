////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arc Welder: Anti-Stutter Library
//
// Compresses many G0/G1 commands into G2/G3(arc) commands where possible, ensuring the tool paths stay within the specified resolution.
// This reduces file size and the number of gcodes per second.
//
// Uses the 'Gcode Processor Library' for gcode parsing, position processing, logging, and other various functionality.
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

#include "segmented_arc.h"
#include "utilities.h"
#include "segmented_shape.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <cmath>

segmented_arc::segmented_arc() : segmented_shape(DEFAULT_MIN_SEGMENTS, DEFAULT_MAX_SEGMENTS, DEFAULT_RESOLUTION_MM, ARC_LENGTH_PERCENT_TOLERANCE_DEFAULT)
{
	
}

segmented_arc::segmented_arc(
	int min_segments, 
	int max_segments, 
	double resolution_mm, 
	double path_tolerance_percent, 
	double max_radius_mm) : segmented_shape(min_segments, max_segments, resolution_mm, path_tolerance_percent)
{
	if (max_radius_mm > DEFAULT_MAX_RADIUS_MM) max_radius_mm_ = DEFAULT_MAX_RADIUS_MM;
	else max_radius_mm_ = max_radius_mm;
}

segmented_arc::~segmented_arc()
{
}

point segmented_arc::pop_front(double e_relative)
{
	e_relative_ -= e_relative;
	if (points_.count() == get_min_segments())
	{
		set_is_shape(false);
	}
	return points_.pop_front();
}
point segmented_arc::pop_back(double e_relative)
{
	e_relative_ -= e_relative;
	return points_.pop_back();
	if (points_.count() == get_min_segments())
	{
		set_is_shape(false);
	}
}
double segmented_arc::get_max_radius() const
{
	return max_radius_mm_;
}
bool segmented_arc::is_shape() const
{
/*
	if (is_shape_)
	{
		arc a;
		return arc::try_create_arc(arc_circle_, points_, original_shape_length_, resolution_mm_, a);;
	} */
	return is_shape_;
}

bool segmented_arc::try_add_point(point p, double e_relative)
{
	
	bool point_added = false;
	// if we don't have enough segnemts to check the shape, just add
	if (points_.count() > get_max_segments() - 1)
	{
		// Too many points, we can't add more
		return false;
	}
	double distance = 0;
	if (points_.count() > 0)
	{
		point p1 = points_[points_.count() - 1];
		distance = utilities::get_cartesian_distance(p1.x, p1.y, p.x, p.y);
		if (!utilities::is_equal(p1.z, p.z))
		{
			// Arcs require that z is equal for all points
			//std::cout << " failed - z change.\n";

			return false;
		}

		if (utilities::is_zero(distance))
		{
			// there must be some distance between the points
			// to make an arc.
			//std::cout << " failed - no distance change.\n";
			return false;
		}
		
	}
	
	if (points_.count() < get_min_segments() - 1)
	{
		point_added = true;
		points_.push_back(p);
		original_shape_length_ += distance;
		if (points_.count() == get_min_segments())
		{
			arc a;
			if (!arc::try_create_arc(arc_circle_, points_, original_shape_length_, a, resolution_mm_, path_tolerance_percent_))
			{
				point_added = false;
				points_.pop_back();
				original_shape_length_ -= distance;
			}
		}
		

	}
	else
	{
		// if we're here, we need to see if the new point can be included in the shape
		point_added = try_add_point_internal_(p, distance);
	}
	if (point_added)
	{
		
		if (points_.count() > 1)
		{
			// Only add the relative distance to the second point on up.
			e_relative_ += e_relative;
		}
		//std::cout << " success - " << points_.count() << " points.\n";
	}
	else if (points_.count() < get_min_segments() && points_.count() > 1)
	{
		// If we haven't added a point, and we have exactly min_segments_,
		// pull off the initial arc point and try again
		point old_initial_point = points_.pop_front();
		// We have to remove the distance and e relative value
		// accumulated between the old arc start point and the new
		point new_initial_point = points_[0];
		original_shape_length_ -= utilities::get_cartesian_distance(old_initial_point.x, old_initial_point.y, new_initial_point.x, new_initial_point.y);
		e_relative_ -= new_initial_point.e_relative;
		//std::cout << " failed - removing start point and retrying current point.\n";
		return try_add_point(p, e_relative);
	}

	return point_added;
}

bool segmented_arc::try_add_point_internal_(point p, double pd)
{
	// If we don't have enough points (at least min_segments) return false
	if (points_.count() < get_min_segments() - 1)
		return false;
	
	// Create a test circle
	circle test_circle;
	bool circle_created;
	// Find a point in the middle of our list for p2
	int mid_point_index = ((points_.count() - 2) / 2)+1;
	circle_created = circle::try_create_circle(points_[0], points_[mid_point_index], p, max_radius_mm_, test_circle);
	
	if (circle_created)
	{

		// If we got a circle, make sure all of the points fit within the tolerance.
		bool circle_fits_points;

		// the circle is new..  we have to test it now, which is expensive :(
		points_.push_back(p);
		double previous_shape_length = original_shape_length_;
		original_shape_length_ += pd;
		
		circle_fits_points = does_circle_fit_points_(test_circle);
		
		if (circle_fits_points)
		{
			arc_circle_ = test_circle;
		}
		else
		{
			points_.pop_back();
			original_shape_length_ = previous_shape_length;
		}
		
		// Only set is_shape if it goes from false to true
		if (!is_shape())
			set_is_shape(circle_fits_points);
		
		return circle_fits_points;
	}
	
	//std::cout << " failed - could not create a circle from the points.\n";
	return false;
	
}

bool segmented_arc::does_arc_fit_points(circle& c) const
{
	arc a;
	return arc::try_create_arc(c, points_, original_shape_length_, a, resolution_mm_, path_tolerance_percent_);
	/*double distance_from_center;
	double difference_from_radius;
	for (int index = 0; index < points_.count() - 1; index++)
	{
		// Make sure the length from the center of our circle to the test point is 
			// at or below our max distance.
		point cur_point(points_[index]);
		double x_rel = cur_point.x - c.center.x;
		double y_rel = cur_point.y - c.center.y;
		bool clockwise = a.angle_radians < 0;

		bool not_in_sector = (
			! (-a.start_point.x * y_rel + a.start_point.y * x_rel > 0)
			&& (-a.end_point.x * y_rel + a.end_point.y * x_rel > 0)
		) == clockwise;
		if (not_in_sector)
		{
			return false;
		}
		

	}			*/
	// Radius compare
	/*
	double r_axis_x = -(a.center.x - a.start_point.x);
	double r_axis_y = -(a.center.y - a.start_point.y);
	float center_axis_x = a.start_point.x - r_axis_x;
	float center_axis_y = a.start_point.y - r_axis_y;
	float rt_x = a.end_point.x - center_axis_x;
	float rt_y = a.end_point.y - center_axis_y;
	float angular_travel_total = std::atan2(r_axis_x * rt_y - r_axis_y * rt_x, r_axis_x * rt_x + r_axis_y * rt_y);
	if (a.angle_radians>0) { angular_travel_total -= 2 * PI_DOUBLE; }
	double test_radius = std::abs(a.radius * angular_travel_total);
	if (utilities::is_zero(test_radius - original_shape_length_, resolution_mm_))
	{
		return true;
	}
	return false;
		*/
}

bool segmented_arc::is_point_on_arc(const arc& a, const point& p) const
{
	double distance_from_center;
	double difference_from_radius;
	

	for (int index = 0; index < points_.count() - 1; index++)
	{
		// Make sure the length from the center of our circle to the test point is 
		// at or below our max distance.
		distance_from_center = utilities::get_cartesian_distance(points_[index].x, points_[index].y, a.center.x, a.center.y);
		double difference_from_radius = std::abs(distance_from_center - a.radius);
		if (utilities::greater_than(difference_from_radius, resolution_mm_))
		{
			//std::cout << " failed - end points do not lie on circle.\n";
			return false;
		}
		// see if the point is within the cone
		
	}
	return true;
}


bool segmented_arc::does_circle_fit_points_(circle& c) const
{
	// We know point 1 must fit (we used it to create the circle).  Check the other points
	// Note:  We have not added the current point, but that's fine since it is guaranteed to fit too.
	// If this works, it will be added.

	double distance_from_center;
	double difference_from_radius;
	
	// Check the endpoints to make sure they fit the current circle
	for (int index = 1; index < points_.count(); index++)
	{
		// Make sure the length from the center of our circle to the test point is 
		// at or below our max distance.
		distance_from_center = utilities::get_cartesian_distance(points_[index].x, points_[index].y, c.center.x, c.center.y);
		double difference_from_radius = std::abs(distance_from_center - c.radius);
		if (utilities::greater_than(difference_from_radius, resolution_mm_))
		{
			//std::cout << " failed - end points do not lie on circle.\n";
			return false;
		}
	}
	
	// Check the point perpendicular from the segment to the circle's center, if any such point exists
	for (int index = 0; index < points_.count() - 1; index++)
	{
		point point_to_test;
		if (segment::get_closest_perpendicular_point(points_[index], points_[index + 1], c.center, point_to_test))
		{
			distance_from_center = utilities::get_cartesian_distance(point_to_test.x, point_to_test.y, c.center.x, c.center.y);
			difference_from_radius = std::abs(distance_from_center - c.radius);
			// Test allowing more play for the midpoints.
			if (utilities::greater_than(difference_from_radius, resolution_mm_))
			{
				return false;
			}
		}
		
	}
	
	// get the current arc and compare the total length to the original length
	//arc a;
	//return arc::try_create_arc(c, points_, original_shape_length_, resolution_mm_, a))
	
	return does_arc_fit_points(c);
	
}

bool segmented_arc::try_get_arc(arc & target_arc)																								 
{
	//int mid_point_index = ((points_.count() - 2) / 2) + 1;
	//return arc::try_create_arc(arc_circle_, points_[0], points_[mid_point_index], points_[points_.count() - 1], original_shape_length_, resolution_mm_, target_arc);
	return arc::try_create_arc(arc_circle_ ,points_, original_shape_length_, target_arc, resolution_mm_, path_tolerance_percent_);
}

bool segmented_arc::try_get_arc_(const circle& c, arc &target_arc)
{
	//int mid_point_index = ((points_.count() - 1) / 2) + 1;
	//return arc::try_create_arc(c, points_[0], points_[mid_point_index], endpoint, original_shape_length_ + additional_distance, resolution_mm_, target_arc);
	return arc::try_create_arc(c, points_, original_shape_length_, target_arc, resolution_mm_, path_tolerance_percent_);
}

std::string segmented_arc::get_shape_gcode_absolute(double e, double f)
{
	bool has_e = e_relative_ != 0;
	return get_shape_gcode_(has_e, e, f);
}
std::string segmented_arc::get_shape_gcode_relative(double f)
{
	bool has_e = e_relative_ != 0;
	return get_shape_gcode_(has_e, e_relative_, f);
}

std::string segmented_arc::get_shape_gcode_(bool has_e, double e, double f) const
{
	
	char buf[20];
	std::string gcode;
	arc c;

	arc::try_create_arc(arc_circle_, points_, original_shape_length_, c, resolution_mm_, path_tolerance_percent_);
	double i = c.center.x - c.start_point.x;
	double j = c.center.y - c.start_point.y;
	// Here is where the performance part kicks in (these are expensive calls) that makes things a bit ugly.
	// there are a few cases we need to take into consideration before choosing our sprintf string
	// create the XYZ portion
	
	if (c.angle_radians < 0)
	{
		gcode = "G2";
	}
	else
	{
		gcode = "G3";
	
	}
	// Add X, Y, I and J
	gcode += " X";
	gcode += utilities::to_string(c.end_point.x, xyz_precision_, buf, false);

	gcode += " Y";
	gcode += utilities::to_string(c.end_point.y, xyz_precision_, buf, false);

	gcode += " I";
	gcode += utilities::to_string(i, xyz_precision_, buf, false);

	gcode += " J";
	gcode += utilities::to_string(j, xyz_precision_, buf, false);
	
	// Add E if it appears
	if (has_e)
	{
		gcode += " E";
		gcode += utilities::to_string(e, e_precision_, buf, false);
	}

	// Add F if it appears
	// Never add F, it should NEVER change!

	return gcode;

}


#include "repiter_arc.h"


/*
// Arc function taken from grbl
// The arc is approximated by generating a huge number of tiny, linear segments. The length of each
// segment is configured in settings.mm_per_arc_segment.
void repiter_arc::arc(float* position, float* target, float* offset, float radius, uint8_t isclockwise) {
  //   int acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
  //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
  float center_axis0 = position[X_AXIS] + offset[X_AXIS];
  float center_axis1 = position[Y_AXIS] + offset[Y_AXIS];
  //float linear_travel = 0; //target[axis_linear] - position[axis_linear];
  float extruder_travel = (Printer::destinationSteps[E_AXIS] - Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS];
  float r_axis0 = -offset[0]; // Radius vector from center to current location
  float r_axis1 = -offset[1];
  float rt_axis0 = target[0] - center_axis0;
  float rt_axis1 = target[1] - center_axis1;
  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel = atan2(r_axis0 * rt_axis1 - r_axis1 * rt_axis0, r_axis0 * rt_axis0 + r_axis1 * rt_axis1);
  if ((!isclockwise && angular_travel <= 0.00001) || (isclockwise && angular_travel < -0.000001)) {
    angular_travel += 2.0f * M_PI;
  }
  if (isclockwise) {
    angular_travel -= 2.0f * M_PI;
  }

  float millimeters_of_travel = fabs(angular_travel) * radius; //hypot(angular_travel*radius, fabs(linear_travel));
  if (millimeters_of_travel < 0.001f) {
    return; // treat as succes because there is nothing to do;
  }
  //uint16_t segments = (radius>=BIG_ARC_RADIUS ? floor(millimeters_of_travel/MM_PER_ARC_SEGMENT_BIG) : floor(millimeters_of_travel/MM_PER_ARC_SEGMENT));
  // Increase segment size if printing faster then computation speed allows
  uint16_t segments = (Printer::feedrate > 60.0f ? floor(millimeters_of_travel / RMath::min(static_cast<float>(MM_PER_ARC_SEGMENT_BIG), Printer::feedrate * 0.01666f * static_cast<float>(MM_PER_ARC_SEGMENT))) : floor(millimeters_of_travel / static_cast<float>(MM_PER_ARC_SEGMENT)));
  if (segments == 0)
    segments = 1;

  float theta_per_segment = angular_travel / segments;
  //float linear_per_segment = linear_travel/segments;
  float extruder_per_segment = extruder_travel / segments;

  
  // Vector rotation matrix values
  float cos_T = 1 - 0.5 * theta_per_segment * theta_per_segment; // Small angle approximation
  float sin_T = theta_per_segment;

  float arc_target[4];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count = 0;

  // Initialize the linear axis
  //arc_target[axis_linear] = position[axis_linear];

  // Initialize the extruder axis
  arc_target[E_AXIS] = Printer::currentPositionSteps[E_AXIS] * Printer::invAxisStepsPerMM[E_AXIS];

  for (i = 1; i < segments; i++) {
    // Increment (segments-1)

    if ((count & 3) == 0) {
      //GCode::readFromSerial();
      Commands::checkForPeriodicalActions(false);
      UI_MEDIUM; // do check encoder
    }

    if (count < N_ARC_CORRECTION) { //25 pieces
        // Apply vector rotation matrix
      r_axisi = r_axis0 * sin_T + r_axis1 * cos_T;
      r_axis0 = r_axis0 * cos_T - r_axis1 * sin_T;
      r_axis1 = r_axisi;
      count++;
    }
    else {
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      cos_Ti = cos(i * theta_per_segment);
      sin_Ti = sin(i * theta_per_segment);
      r_axis0 = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      r_axis1 = -offset[0] * sin_Ti - offset[1] * cos_Ti;
      count = 0;
    }

    // Update arc_target location
    arc_target[X_AXIS] = center_axis0 + r_axis0;
    arc_target[Y_AXIS] = center_axis1 + r_axis1;
    //arc_target[axis_linear] += linear_per_segment;
    arc_target[E_AXIS] += extruder_per_segment;
    Printer::moveToReal(arc_target[X_AXIS], arc_target[Y_AXIS], IGNORE_COORDINATE, arc_target[E_AXIS], IGNORE_COORDINATE);
  }
  // Ensure last segment arrives at target location.
  Printer::moveToReal(target[X_AXIS], target[Y_AXIS], IGNORE_COORDINATE, target[E_AXIS], IGNORE_COORDINATE);
}

*/
#pragma once
#include <string>

enum firmware_types { MARLIN_1=0, MARLIN_2=1, REPETIER=2, PRUSA=3, SMOOTHIEWARE=4};
#define NUM_FIRMWARE_TYPES 5
static const std::string firmware_type_names[NUM_FIRMWARE_TYPES] = {
		 "MARLIN_1", "MARLIN_2", "REPETIER", "PRUSA", "SMOOTHIEWARE"
};





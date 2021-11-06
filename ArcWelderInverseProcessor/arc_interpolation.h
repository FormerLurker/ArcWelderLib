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
		std::string get_firmware_argument_description() const;
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


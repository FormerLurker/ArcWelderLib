#pragma once
#include <cstdint>
#include "firmware.h"
#include "utilities.h"
#define repetier_is_close_value 0.001f
#define repetier_is_close(x,y)  ( repetier_is_close_value > utilities::fabs(x-y) )
class repetier :
  public firmware
{
public:
  enum class repetier_firmware_versions { V1_0_4 = 0, V1_0_5};
  
  repetier(firmware_arguments args);
  virtual ~repetier();
  virtual std::string interpolate_arc(firmware_position& target, double i, double j, double r, bool is_clockwise) override;
  virtual firmware_arguments get_default_arguments_for_current_version() const override;
  virtual void apply_arguments() override;
private:
  repetier_firmware_versions repetier_version_;
  std::string gcodes_;
  const static int REPETIER_XYZE = 4;
  enum AxisEnum { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3};
  /// <summary>
  /// A struct representing the prusa configuration store.  Note:  I didn't add the trailing underscore so this variable name will match the original source algorithm name.
  /// </summary>
  typedef void(repetier::* arc_func)(float* position, float* target, float* offset, float radius, uint8_t isclockwise);

  void arc_1_0_4(float* position, float* target, float* offset, float radius, uint8_t isclockwise);
  void arc_1_0_5(float* position, float* target, float* offset, float radius, uint8_t isclockwise);

  arc_func arc_;

  // Note that trailing underscore are sometimes dropped to keep the ported function as close as possible to the original
  float feedrate;
  // Repetier Function Defs
  float min(float x, float y);
  void moveToReal(float x, float y, float z, float e);
};


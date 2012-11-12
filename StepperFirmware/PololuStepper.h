#ifndef _POLOLUSTEPPER_h
#define _POLOLUSTEPPER_h

#include <Arduino.h>

#define FULL_STEP             1
#define HALF_STEP             2
#define QUARTER_STEP          4
#define EIGHTH_STEP           8
#define SIXTEENTH_STEP        16

#define CLOCKWISE             true
#define COUNTERCLOCKWISE      false

// Simple class to configure and drive a pololu stepper controller.
class PololuStepper {
  private:
    boolean m_reversed;  ///< If true, the motor is backwards so invert the direction pin.
    
  public:
    // Initialize the stepper motor. Must be called before using the stepper.
    // @param steppingMode Stepping mode, can be FULL_STEP, HALF_STEP, QUARTER_STEP, EIGHTH_STEP, or SIXTEENTH_STEP
    // @param driverCurrent Drive current, in units (not mA!)
    // @param reversed If true, reverse the motor direction
    void init(uint8_t steppingMode, uint8_t driverCurrent, boolean reversed);
  
    // Enable the stepper motor driver
    void enable();
    
    // Disable the stepper motor driver
    void disable();
    
    // Reset the stepper motor (?)
    void reset();
    
    // Configure the microstepping mode of the stepper
    // @param mode Can be FULL_STEP, HALF_STEP, QUARTER_STEP, EIGHTH_STEP, or SIXTEENTH_STEP
    void setMicrostepMode(uint8_t mode);
    
    // Set the referce current for the steppermotor
    // @param current Microstepping current, in fake units.
    void setReferenceCurrent(uint8_t current);
    
    // Set the direction that the stepper motor should turn
    // @param direction Can be CLOCKWISE or COUNTERCLOCKWISE
    void setDirection(boolean direction);
    
    // Make one step (or one microstep, depending on the current motor configuration)
    void step();
};

#endif

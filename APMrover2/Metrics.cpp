#include "Rover.h"

/*
  constructor
 */
Metrics::Metrics()
{
}

// update all metrics
void Metrics::update_Metrics(){
    update_error();
    update_ISE();
    update_IAE();
}

// update all errors
void Metrics::update_error(){
    get_error_speed();
    get_error_steering();
}

// update all IAE metrics
void Metrics::update_IAE(){
    _IAE_speed += calc_IAE(_error_speed);
    _IAE_steering += calc_IAE(_error_steering);
}

// update all ISE metrics
void Metrics::update_ISE(){
    _ISE_speed += calc_ISE(_error_speed);
    _ISE_steering += calc_ISE(_error_steering);
}

// calculate IAE
float Metrics::calc_IAE(float error){
    return fabsf(error) * rover.G_Dt;
}

// calculate ISE
float Metrics::calc_ISE(float error){
    return fabsf(error) * fabsf(error) * rover.G_Dt;
}

// get speed error from PID
void Metrics::get_error_speed(){
    _error_speed = rover.g2.attitude_control.get_sailboat_speed_pid().get_error();
}

// get steering error from PID
void Metrics::get_error_steering(){
    _error_steering = rover.g2.attitude_control.get_steering_rate_pid().get_error();
}
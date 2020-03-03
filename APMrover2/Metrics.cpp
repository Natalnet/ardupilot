#include "Rover.h"

/*
  constructor
 */
Metrics::Metrics()
{
}

// update all metrics
void Metrics::update_metrics(){
    update_error();
    update_ISE();
    update_IAE();
    update_ITAE();
    update_ITSE();
    update_IAEW();
    update_IADC();
    update_IAE_IADC();
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

// update all ITAE metrics
void Metrics::update_ITAE(){
    _ITAE_speed += calc_ITAE(_error_speed);
    _ITAE_steering += calc_ITAE(_error_steering);
}

// update all ITSE metrics
void Metrics::update_ITSE(){
    _ITSE_speed += calc_ITSE(_error_speed);
    _ITSE_steering += calc_ITSE(_error_steering);
}

// update all IAEW metrics
void Metrics::update_IAEW(){
    _IAEW_speed = calc_IAEW(_IAE_speed, rover.battery.get_sail_consumed_wh());
    _IAEW_steering = calc_IAEW(_IAE_steering, rover.battery.get_rudder_consumed_wh());
}

// update all IADC metrics
void Metrics::update_IADC(){
    _IADC_speed += calc_IADC(_diff_throttle);
    _IADC_steering += calc_IADC(_diff_steering);
}

// update all IAE_IADC metrics
void Metrics::update_IAE_IADC(){
    _IAE_IADC_speed = calc_IAE_IADC(_IAE_speed, _IADC_speed);
    _IAE_IADC_steering = calc_IAE_IADC(_IAE_steering, _IADC_steering);
}

// calculate IAE
float Metrics::calc_IAE(float error){
    return fabsf(error) * rover.G_Dt;
}

// calculate ISE
float Metrics::calc_ISE(float error){
    return fabsf(error) * fabsf(error) * rover.G_Dt;
}

// calculate ITAE
float Metrics::calc_ITAE(float error){
    return calc_IAE(error) * _current_time;
}

// calculate ITSE
float Metrics::calc_ITSE(float error){
    return calc_ISE(error) * _current_time;
}

// calculate IAEW
float Metrics::calc_IAEW(float IAE, float current_wh){
    return IAE * current_wh;
}

// calculate IADC
float Metrics::calc_IADC(float diff_actuator){
    return fabsf(diff_actuator) * rover.G_Dt;
}

// calculate IAE_IADC
float Metrics::calc_IAE_IADC(float IAE, float IADC){
    return IAE * IADC;
}

// update all errors (speed and steering)
void Metrics::update_error(){
    get_error_speed();
    get_error_steering();

    // get time since started mission
    _current_time = AP_HAL::millis() - _arm_time;

    float local_wh;
    // get wh since started mission
    if(rover.battery.consumed_wh(local_wh)){
        _current_wh = local_wh - _arm_wh;
    }

    _diff_steering = rover.g2.motors.get_steering() - _last_steering;
    _diff_throttle = rover.g2.motors.get_throttle() - _last_throttle;

    _last_steering = _diff_steering;
    _last_throttle = _diff_throttle;
}

// get speed error from PID
void Metrics::get_error_speed(){
    _error_speed = rover.g2.attitude_control.get_sailboat_speed_pid().get_error();
}

// get steering error from PID
void Metrics::get_error_steering(){
    _error_steering = rover.g2.attitude_control.get_steering_rate_pid().get_error();
}

void Metrics::increment_tack(){
    _number_of_tacks++;
}

void Metrics::reset_metrics(){
    _IAE_speed = 0;
    _ISE_speed = 0;
    _ITAE_speed = 0;
    _ITSE_speed = 0;

    _IAE_steering = 0;
    _ISE_steering = 0;
    _ITAE_steering = 0;
    _ITSE_steering = 0;

    _number_of_tacks = 0;
}
#include "Rover.h"

#define SAILBOAT_AUTO_TACKING_TIMEOUT_MS 5000   // tacks in auto mode timeout if not successfully completed within this many milliseconds
#define SAILBOAT_TACKING_ACCURACY_DEG 10        // tack is considered complete when vehicle is within this many degrees of target tack angle
#define SAILBOAT_NOGO_PAD 10                    // deg, the no go zone is padded by this much when deciding if we should use the Sailboat heading controller
#define TACK_RETRY_TIME_MS 5000                 // Can only try another auto mode tack this many milliseconds after the last is cleared (either competed or timed-out)
/*
To Do List
 - Improve tacking in light winds and bearing away in strong wings
 - consider drag vs lift sailing differences, ie upwind sail is like wing, dead down wind sail is like parachute
 - max speed paramiter and controller, for mapping you may not want to go too fast
 - mavlink sailing messages
 - motor sailing, some boats may also have motor, we need to decide at what point we would be better of just motoring in low wind, or for a tight loiter, or to hit waypoint exactly, or if stuck head to wind, or to reverse...
 - smart decision making, ie tack on windshifts, what to do if stuck head to wind
 - some sailing codes track waves to try and 'surf' and to allow tacking on a flat bit, not sure if there is much gain to be had here
 - add some sort of pitch monitoring to prevent nose diving in heavy weather
 - pitch PID for hydrofoils
 - more advanced sail control, ie twist
 - independent sheeting for main and jib
 - wing type sails with 'elevator' control
 - tack on depth sounder info to stop sailing into shallow water on indirect sailing routes
 - add option to do proper tacks, ie tacking on flat spot in the waves, or only try once at a certain speed, or some better method than just changing the desired heading suddenly
*/

const AP_Param::GroupInfo Sailboat::var_info[] = {

    // @Param: ENABLE
    // @DisplayName: Enable Sailboat
    // @Description: This enables Sailboat functionality
    // @Values: 0:Disable,1:Enable
    // @User: Standard
    // @RebootRequired: True
    AP_GROUPINFO_FLAGS("ENABLE", 1, Sailboat, enable, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: ANGLE_MIN
    // @DisplayName: Sail min angle
    // @Description: Mainsheet tight, angle between centerline and boom
    // @Units: deg
    // @Range: 0 90
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("ANGLE_MIN", 2, Sailboat, sail_angle_min, 0),

    // @Param: ANGLE_MAX
    // @DisplayName: Sail max angle
    // @Description: Mainsheet loose, angle between centerline and boom
    // @Units: deg
    // @Range: 0 90
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("ANGLE_MAX", 3, Sailboat, sail_angle_max, 90),

    // @Param: ANGLE_IDEAL
    // @DisplayName: Sail ideal angle
    // @Description: Ideal angle between sail and apparent wind
    // @Units: deg
    // @Range: 0 90
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("ANGLE_IDEAL", 4, Sailboat, sail_angle_ideal, 25),

    // @Param: HEEL_MAX
    // @DisplayName: Sailing maximum heel angle
    // @Description: When in auto sail trim modes the heel will be limited to this value using PID control
    // @Units: deg
    // @Range: 0 90
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("HEEL_MAX", 5, Sailboat, sail_heel_angle_max, 15),

    // @Param: NO_GO_ANGLE
    // @DisplayName: Sailing no go zone angle
    // @Description: The typical closest angle to the wind the vehicle will sail at. the vehicle will sail at this angle when going upwind
    // @Units: deg
    // @Range: 0 90
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("NO_GO_ANGLE", 6, Sailboat, sail_no_go, 45),

    // @Param: WNDSPD_MIN
    // @DisplayName: Sailboat minimum wind speed to sail in
    // @Description: Sailboat minimum wind speed to continue sail in, at lower wind speeds the sailboat will motor if one is fitted
    // @Units: m/s
    // @Range: 0 5
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("WNDSPD_MIN", 7, Sailboat, sail_windspeed_min, 0),

    // @Param: XTRACK_MAX
    // @DisplayName: Sailing vehicle max cross track error
    // @Description: The sail boat will tack when it reaches this cross track error, defines a corridor of 2 times this value wide, 0 disables
    // @Units: m
    // @Range: 5 25
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("XTRACK_MAX", 8, Sailboat, xtrack_max, 10),

    // @Param: LOIT_RADIUS
    // @DisplayName: Loiter radius
    // @Description: When in sailing modes the vehicle will keep moving within this loiter radius
    // @Units: m
    // @Range: 0 20
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("LOIT_RADIUS", 9, Sailboat, loit_radius, 5),

    // @Param: PROP_CTRL
    // @DisplayName: Sail control type
    // @Description: Type of sail control for the sailboat. 0- standard, 1- fixed angle
    // @Units: int
    // @Range: 0 - 10
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("PROP_CTRL", 10, Sailboat, sail_control_type, 0),

    // @Param: FIXED_ANGLE
    // @DisplayName: Sail fixed angle
    // @Description: Sail angle for fixed sail control.
    // @Units: degrees
    // @Range: 0 90
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("FIXED_ANGLE", 11, Sailboat, sail_fixed_angle, 0.0f),

    // @Param: SPEED_MAX
    // @DisplayName: Sailing max speed
    // @Description: Referece speed for speed control of sailboat.
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("SPEED_MAX", 12, Sailboat, sail_speed_max, 0.0f),

    // @Param: EXTR_STEP
    // @DisplayName: Sailing max speed
    // @Description: Referece speed for speed control of sailboat.
    // @Units: degrees
    // @Range: 0 50
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("EXTR_STEP", 13, Sailboat, sail_extr_step, 5.0f),

    // @Param: EXTR_T
    // @DisplayName: Sailing max speed
    // @Description: Referece speed for speed control of sailboat.
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("EXTR_T", 14, Sailboat, sail_extr_t, 1.0f),

    // @Param: POLAR_T
    // @DisplayName: Sailing max speed
    // @Description: Referece speed for speed control of sailboat.
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("POLAR_T", 15, Sailboat, sail_polar_t, 1.0f),

    // @Param: TACK_TYPE
    // @DisplayName: Sailing tack type
    // @Description: 0 - reactive, 1 - deliberative
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("TACK_TYPE", 16, Sailboat, tack_type, 0.0f),

    // @Param: TACK_TYPE
    // @DisplayName: Sailing tack type
    // @Description: 0 - reactive, 1 - deliberative
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("TACK_DT", 17, Sailboat, tack_d_t, 10.0f),

    // @Param: TACK_TYPE    
    // @DisplayName: Sailing tack type
    // @Description: 0 - reactive, 1 - deliberative
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("TACK_THETAT", 18, Sailboat, tack_theta_t, 60.0f),

    // @Param: WAYP_TYPE
    // @DisplayName: Type of waypoint following
    // @Description: 0 - L1 controller, 1 - heading controller
    // @Units: degrees
    // @Range: 0 10
    // @Increment: 0.1
    // @User: Standard
    AP_GROUPINFO("WAYP_TYPE", 19, Sailboat, waypoint_follow_type, 0.0f),

    AP_GROUPEND
};


/*
  constructor
 */
Sailboat::Sailboat()
{
    AP_Param::setup_object_defaults(this, var_info);
}

// true if sailboat navigation (aka tacking) is enabled
bool Sailboat::tack_enabled() const
{
    // tacking disabled if not a sailboat
    if (!sail_enabled()) {
        return false;
    }

    // tacking disabled if motor is always on
    if (motor_state == UseMotor::USE_MOTOR_ALWAYS) {
        return false;
    }

    // disable tacking if motor is available and wind is below cutoff
    if (motor_assist_low_wind()) {
        return false;
    }

    // otherwise tacking is enabled
    return true;
}

void Sailboat::init()
{
    // sailboat defaults
    if (sail_enabled()) {
        rover.g2.crash_angle.set_default(0);
    }

    if (tack_enabled()) {
        rover.g2.loit_type.set_default(1);
    }

    // initialise motor state to USE_MOTOR_ASSIST
    // this will silently fail if there is no motor attached
    set_motor_state(UseMotor::USE_MOTOR_ASSIST, false);
}

// initialise rc input (channel_mainsail), may be called intermittently
void Sailboat::init_rc_in()
{
    // get auxiliary throttle value
    RC_Channel *rc_ptr = rc().find_channel_for_option(RC_Channel::AUX_FUNC::MAINSAIL);
    if (rc_ptr != nullptr) {
        // use aux as sail input if defined
        channel_mainsail = rc_ptr;
        channel_mainsail->set_angle(100);
        channel_mainsail->set_default_dead_zone(30);
    } else {
        // use throttle channel
        channel_mainsail = rover.channel_throttle;
    }
}

// decode pilot mainsail input and return in steer_out and throttle_out arguments
// mainsail_out is in the range 0 to 100, defaults to 100 (fully relaxed) if no input configured
void Sailboat::get_pilot_desired_mainsail(float &mainsail_out)
{
    // no RC input means mainsail is moved to trim
    if ((rover.failsafe.bits & FAILSAFE_EVENT_THROTTLE) || (channel_mainsail == nullptr)) {
        mainsail_out = 100.0f;
        return;
    }
    mainsail_out = constrain_float(channel_mainsail->get_control_in(), 0.0f, 100.0f);
}

// calculate throttle and mainsail angle required to attain desired speed (in m/s)
// returns true if successful, false if sailboats not enabled
void Sailboat::get_throttle_and_mainsail_out(float desired_speed, float &throttle_out, float &mainsail_out)
{
    rover.g2.metrics.update_metrics();
    if (!sail_enabled()) {
        throttle_out = 0.0f;
        mainsail_out = 0.0f;
        return;
    }

    // run speed controller if motor is forced on or motor assistance is required for low speeds or tacking
    if ((motor_state == UseMotor::USE_MOTOR_ALWAYS) ||
         motor_assist_tack() ||
         motor_assist_low_wind()) {
        // run speed controller - duplicate of calls found in mode::calc_throttle();
        throttle_out = 100.0f * rover.g2.attitude_control.get_throttle_out_speed(desired_speed,
                                                                        rover.g2.motors.limit.throttle_lower,
                                                                        rover.g2.motors.limit.throttle_upper,
                                                                        rover.g.speed_cruise,
                                                                        rover.g.throttle_cruise * 0.01f,
                                                                        rover.G_Dt);
    } else {
        throttle_out = 0.0f;
    }

    //
    // mainsail control
    //
    // if we are motoring or attempting to reverse relax the sail
    if (motor_state == UseMotor::USE_MOTOR_ALWAYS || !is_positive(desired_speed)) {
        mainsail_out = 100.0f;
    } else {
    	switch (sail_control_type) {

    		// standard linear control
    		case (LINEAR): {

    			// + is wind over starboard side, - is wind over port side, but as the sails are sheeted the same on each side it makes no difference so take abs
		        float wind_dir_apparent = fabsf(rover.g2.windvane.get_apparent_wind_direction_rad());
		        wind_dir_apparent = degrees(wind_dir_apparent);

		        // set the main sail to the ideal angle to the wind
		        float mainsail_angle = wind_dir_apparent - sail_angle_ideal;

		        // make sure between allowable range
		        mainsail_angle = constrain_float(mainsail_angle,sail_angle_min, sail_angle_max);

		        // linear interpolate mainsail value (0 to 100) from wind angle mainsail_angle
		        float mainsail_base = linear_interpolate(0.0f, 100.0f, mainsail_angle,sail_angle_min,sail_angle_max);

		        // use PID controller to sheet out
		        const float pid_offset = rover.g2.attitude_control.get_sail_out_from_heel(radians(sail_heel_angle_max), rover.G_Dt) * 100.0f;

		        mainsail_out = constrain_float((mainsail_base + pid_offset), 0.0f ,100.0f);
    			break;
    		}

    		// fixed angle for sail control
    		// PARAMS: SAIL_FIXED_ANGLE
    		case (FIXED): {
    			mainsail_out = linear_interpolate(0.0f, 100.0f, sail_fixed_angle, sail_angle_min, sail_angle_max);
    			break;
    		}

    		//TODO: use a cardioid function to approximate the polar diagram. speed control trying to get the speed given by the polar diagram
    		case (POLAR_DIAGRAM_CARD): {

                // get current time
                float now = AP_HAL::millis();

                // time constant off sailboat
                float T = sail_polar_t*1000;

                // + is wind over starboard side, - is wind over port side, but as the sails are sheeted the same on each side it makes no difference so take abs
                float wind_dir_apparent = fabsf(rover.g2.windvane.get_apparent_wind_direction_rad());
                wind_dir_apparent = degrees(wind_dir_apparent);

                // set the main sail to the ideal angle to the wind
                float mainsail_angle = wind_dir_apparent - sail_angle_ideal;

                // make sure between allowable range
                mainsail_angle = constrain_float(mainsail_angle,sail_angle_min, sail_angle_max);

                // linear interpolate mainsail value (0 to 100) from wind angle mainsail_angle
                float mainsail_base = linear_interpolate(0.0f, 100.0f, mainsail_angle,sail_angle_min,sail_angle_max);

                // use PID controller to sheet out
                const float pid_offset_heel = rover.g2.attitude_control.get_sail_out_from_heel(radians(sail_heel_angle_max), rover.G_Dt) * 100.0f;

                mainsail_out = constrain_float((mainsail_base + pid_offset_heel + _pid_offset_speed), 0.0f ,100.0f);

                // update pid speed
                if ((now - _polar_turn_last_ms) >= T) {

                    float speed;
                    if (!rover.g2.attitude_control.get_forward_speed(speed)) {
                        speed = 0.0f;
                    }

                    // calc first difference of speed and sail angle
                    float du = speed - _speed_last;
                    float ds = _sail_last - _sail_last_last;

                    // use PID controller to achieve max speed
                    const float pid_offset_speed = rover.g2.attitude_control.get_sail_out_from_speed(sail_speed_max, T);

                    // specific control heuristic for sailboat
                    if ((du < 0 && ds < 0) || (du > 0 && ds > 0)){
                        _pid_offset_speed = _pid_offset_speed + fabsf(pid_offset_speed);
                    } else {
                        _pid_offset_speed = _pid_offset_speed + fabsf(pid_offset_speed) * -1.0f;
                    }

                    // constrain control output so it dosent explode (convergence not garanteed)
                    _pid_offset_speed = constrain_float(_pid_offset_speed, -10.0f ,10.0f);

                    //gcs().send_text(MAV_SEVERITY_INFO, "CONTROL SIGNAL ACC = %5.3f", (double)_pid_offset_speed);

                    // update speed of last step
                    _speed_last = speed;

                    // update sail angle of last steps and the latter step
                    _sail_last_last = _sail_last;
                    _sail_last = mainsail_out;

                    // update time of last step
                    _polar_turn_last_ms = now;
                }

    			break;
            }

    		// TODO: use the real polar diagram to give reference speed.
    		case (POLAR_DIAGRAM_REAL): {
    			break;
    		}

    		// No need for polar diagram. perform sucessive changes on sail angle to search for maximum speed.
    		case (EXTREMUM_SEEKING): {

                // get current time
                float now = AP_HAL::millis();
                float T = sail_extr_t*1000;

                // check if its time to change
                if ((now - _extr_turn_last_ms) >= T){

                    float speed;
                    if (!rover.g2.attitude_control.get_forward_speed(speed)) {
                        speed = 0.0f;
                    }

                    // calc first difference of speed and sail angle
                    float du = speed - _speed_last;
                    float ds = _sail_last - _sail_last_last;

                    // translate sail degrees (0 - 90) to output (0 - 100)
                    float _sail_extr_step = linear_interpolate(0.0f, 100.0f, sail_extr_step, sail_angle_min, sail_angle_max);

                    // calc next sail angle step
                    float step = _sail_extr_step * copysign(1.0f, du) * copysign(1.0f, ds);

                    // no heeling control ATM
                    mainsail_out = constrain_float((_sail_last + step), 0.0f ,100.0f);

                    // update speed of last step
                     _speed_last = speed;

                     // update sail angle of last steps and the latter step
                     _sail_last_last = _sail_last;
                     _sail_last = mainsail_out;

                     // update time of last step
                     _extr_turn_last_ms = now;

                } else {

                    // maintain the last sail angle
                    mainsail_out = constrain_float((_sail_last), 0.0f ,100.0f);
                }

    			break;
    		}
    	}
    }
}

// Velocity Made Good, this is the speed we are traveling towards the desired destination
// only for logging at this stage
// https://en.wikipedia.org/wiki/Velocity_made_good
float Sailboat::get_VMG() const
{
    // return zero if we don't have a valid speed
    float speed;
    if (!rover.g2.attitude_control.get_forward_speed(speed)) {
        return 0.0f;
    }

    // return speed if not heading towards a waypoint
    if (!rover.control_mode->is_autopilot_mode()) {
        return speed;
    }

    return (speed * cosf(wrap_PI(radians(rover.g2.wp_nav.wp_bearing_cd() * 0.01f) - rover.ahrs.yaw)));
}

// handle user initiated tack while in acro mode
void Sailboat::handle_tack_request_acro()
{
    if (!tack_enabled() || currently_tacking) {
        return;
    }
    // set tacking heading target to the current angle relative to the true wind but on the new tack
    currently_tacking = true;
    tack_heading_rad = wrap_2PI(rover.ahrs.yaw + 2.0f * wrap_PI((rover.g2.windvane.get_true_wind_direction_rad() - rover.ahrs.yaw)));

    tack_request_ms = AP_HAL::millis();
}

// return target heading in radians when tacking (only used in acro)
float Sailboat::get_tack_heading_rad()
{
    if (fabsf(wrap_PI(tack_heading_rad - rover.ahrs.yaw)) < radians(SAILBOAT_TACKING_ACCURACY_DEG) ||
       ((AP_HAL::millis() - tack_request_ms) > SAILBOAT_AUTO_TACKING_TIMEOUT_MS)) {
        clear_tack();
    }

    return tack_heading_rad;
}

// handle user initiated tack while in autonomous modes (Auto, Guided, RTL, SmartRTL, etc)
void Sailboat::handle_tack_request_auto()
{
    if (!tack_enabled() || currently_tacking) {
        return;
    }

    // record time of request for tack.  This will be processed asynchronously by sailboat_calc_heading
    tack_request_ms = AP_HAL::millis();
}

// clear tacking state variables
void Sailboat::clear_tack()
{
    currently_tacking = false;
    tack_assist = false;
    tack_request_ms = 0;
    tack_clear_ms = AP_HAL::millis();
}

// returns true if boat is currently tacking
bool Sailboat::tacking() const
{
    return tack_enabled() && currently_tacking;
}

// returns true if sailboat should take a indirect navigation route to go upwind
// desired_heading should be in centi-degrees
bool Sailboat::use_indirect_route(float desired_heading_cd) const
{
    if (!tack_enabled()) {
        return false;
    }

    // use sailboat controller until tack is completed
    if (currently_tacking) {
        return true;
    }

    // convert desired heading to radians
    const float desired_heading_rad = radians(desired_heading_cd * 0.01f);

    // check if desired heading is in the no go zone, if it is we can't go direct
    // pad no go zone, this allows use of heading controller rather than L1 when close to the wind
    return fabsf(wrap_PI(rover.g2.windvane.get_true_wind_direction_rad() - desired_heading_rad)) <= radians(sail_no_go + SAILBOAT_NOGO_PAD);
}

// if we can't sail on the desired heading then we should pick the best heading that we can sail on
// this function assumes the caller has already checked sailboat_use_indirect_route(desired_heading_cd) returned true
float Sailboat::calc_heading(float desired_heading_cd)
{
    if (!tack_enabled()) {
        return desired_heading_cd;
    }
    bool should_tack = false;

    // find witch tack we are on
    const AP_WindVane::Sailboat_Tack current_tack = rover.g2.windvane.get_current_tack();

    // convert desired heading to radians
    const float desired_heading_rad = radians(desired_heading_cd * 0.01f);

    // if the desired heading is outside the no go zone there is no need to change it
    // this allows use of heading controller rather than L1 when desired
    // this is used in the 'SAILBOAT_NOGO_PAD' region
    const float true_wind_rad = rover.g2.windvane.get_true_wind_direction_rad();
    if (fabsf(wrap_PI(true_wind_rad - desired_heading_rad)) > radians(sail_no_go) && !currently_tacking) {

        // calculate the tack the new heading would be on
        const float new_heading_apparent_angle = wrap_PI(true_wind_rad - desired_heading_rad);
        AP_WindVane::Sailboat_Tack new_tack;
        if (is_negative(new_heading_apparent_angle)) {
            new_tack = AP_WindVane::Sailboat_Tack::TACK_PORT;
        } else {
            new_tack = AP_WindVane::Sailboat_Tack::TACK_STARBOARD;
        }

        // if the new tack is not the same as the current tack we need might need to tack
        if (new_tack != current_tack) {
            // see if it would be a tack, the front of the boat going through the wind
            // or a gybe, the back of the boat going through the wind
            const float app_wind_rad = rover.g2.windvane.get_apparent_wind_direction_rad();
            if (fabsf(app_wind_rad) + fabsf(new_heading_apparent_angle) < M_PI) {
                should_tack = true;
            }
        }

        if (!should_tack) {
            return desired_heading_cd;
        }
    }

    // check for user requested tack
    uint32_t now = AP_HAL::millis();
    if (tack_request_ms != 0 && !should_tack  && !currently_tacking) {
        // set should_tack flag is user requested tack within last 0.5 sec
        should_tack = ((now - tack_request_ms) < 500);
        tack_request_ms = 0;
    }

    // trigger tack if cross track error larger than xtrack_max parameter
    // this effectively defines a 'corridor' of width 2*xtrack_max that the boat will stay within
    const float cross_track_error = rover.g2.wp_nav.crosstrack_error();
    if ((fabsf(cross_track_error) >= xtrack_max) && !is_zero(xtrack_max) && !should_tack && !currently_tacking) {
        // make sure the new tack will reduce the cross track error
        // if were on starboard tack we are traveling towards the left hand boundary
        if (is_positive(cross_track_error) && (current_tack == AP_WindVane::Sailboat_Tack::TACK_STARBOARD)) {
            should_tack = true;
        }
        // if were on port tack we are traveling towards the right hand boundary
        if (is_negative(cross_track_error) && (current_tack == AP_WindVane::Sailboat_Tack::TACK_PORT)) {
            should_tack = true;
        }
    }

    // calculate left and right no go headings looking upwind, Port tack heading is left no-go, STBD tack is right of no-go
    const float left_no_go_heading_rad = wrap_2PI(true_wind_rad + radians(sail_no_go));
    const float right_no_go_heading_rad = wrap_2PI(true_wind_rad - radians(sail_no_go));

    // if tack triggered, calculate target heading
    if (should_tack && (now - tack_clear_ms) > TACK_RETRY_TIME_MS) {
        // std::vector<Location> tack_points = calc_tack_points(desired_heading_rad);
        gcs().send_text(MAV_SEVERITY_INFO, "Sailboat: Tacking");
        rover.g2.metrics.increment_tack();

        // calculate target heading for the new tack
        switch (current_tack) {
            case AP_WindVane::Sailboat_Tack::TACK_PORT:
                tack_heading_rad = right_no_go_heading_rad;
                break;
            case AP_WindVane::Sailboat_Tack::TACK_STARBOARD:
                tack_heading_rad = left_no_go_heading_rad;
                break;
        }
        currently_tacking = true;
        auto_tack_start_ms = now;
    }

    // if we are tacking we maintain the target heading until the tack completes or times out
    if (currently_tacking) {
        // check if we have reached target
        if (fabsf(wrap_PI(tack_heading_rad - rover.ahrs.yaw)) <= radians(SAILBOAT_TACKING_ACCURACY_DEG)) {
            clear_tack();
        } else if ((now - auto_tack_start_ms) > SAILBOAT_AUTO_TACKING_TIMEOUT_MS) {
            // tack has taken too long
            if ((motor_state == UseMotor::USE_MOTOR_ASSIST) && (now - auto_tack_start_ms) < (3.0f * SAILBOAT_AUTO_TACKING_TIMEOUT_MS)) {
                // if we have throttle available use it for another two time periods to get the tack done
                tack_assist = true;
            } else {
                gcs().send_text(MAV_SEVERITY_INFO, "Sailboat: Tacking timed out");
                clear_tack();
            }
        }
        // return tack target heading
        return degrees(tack_heading_rad) * 100.0f;
    }

    // return the correct heading for our current tack
    if (current_tack == AP_WindVane::Sailboat_Tack::TACK_PORT) {
        return degrees(left_no_go_heading_rad) * 100.0f;
    } else {
        return degrees(right_no_go_heading_rad) * 100.0f;
    }
}

// set state of motor
void Sailboat::set_motor_state(UseMotor state, bool report_failure)
{
    // always allow motor to be disabled
    if (state == UseMotor::USE_MOTOR_NEVER) {
        motor_state = state;
        return;
    }

    // enable assistance or always on if a motor is defined
    if (rover.g2.motors.have_skid_steering() ||
        SRV_Channels::function_assigned(SRV_Channel::k_throttle) ||
        rover.get_frame_type() != rover.g2.motors.frame_type::FRAME_TYPE_UNDEFINED) {
        motor_state = state;
    } else if (report_failure) {
        gcs().send_text(MAV_SEVERITY_WARNING, "Sailboat: failed to enable motor");
    }
}

// true if motor is on to assist with slow tack
bool Sailboat::motor_assist_tack() const
{
    // throttle is assist is disabled
    if (motor_state != UseMotor::USE_MOTOR_ASSIST) {
        return false;
    }

    // assist with a tack because it is taking too long
    return tack_assist;
}

// true if motor should be on to assist with low wind
bool Sailboat::motor_assist_low_wind() const
{
    // motor assist is disabled
    if (motor_state != UseMotor::USE_MOTOR_ASSIST) {
        return false;
    }

    // assist if wind speed is below cutoff
    return (is_positive(sail_windspeed_min) &&
            rover.g2.windvane.wind_speed_enabled() &&
            (rover.g2.windvane.get_true_wind_speed() < sail_windspeed_min));
}

// theta_t and alpha_tw are in degrees
std::vector<Vector2f> Sailboat::calc_deliberative_tack_points_NE(float desired_heading_cd){

    std::vector<Vector2f> local;

    float x0 = 0.0f;
    float y0 = 0.0f;

    Location origin = rover.g2.wp_nav.get_origin();
    Location destination = rover.g2.wp_nav.get_destination();

    float d_t = tack_d_t;
    float theta_t = tack_theta_t;

    Vector2f destination_NE = origin.get_distance_NE(destination);
    Vector2f origin_NE;
    origin_NE.x = x0;
    origin_NE.y = y0;

    float x1 = destination_NE.x;
    float y1 = destination_NE.y;

    Vector2f line_A;
    Vector2f line_B;
    Vector2f line_L1;
    Vector2f line_L2;
    Vector2f line_t1;

    // get angle of attack of apparent wind
    float alpha_aw = wrap_PI(rover.g2.windvane.get_apparent_wind_direction_rad() - radians(desired_heading_cd * 0.01f));

    // find line A
    if (fabsf(x1 - x0) < 0.000000000001) {
        line_A.x = 100000000;
    } else {
        line_A.x = (y1 - y0) / (x1 - x0);
    }

    line_A.y = y0 - (line_A.x * x0);

    float alpha_p = origin.get_bearing_to(destination) * 0.01f;

    float tan_theta_t = tan(radians(theta_t));

    // alpha_aw -> angle of attack of apparent wind; it basically says wich side is better to start tacking
    if (alpha_aw < alpha_p){
        line_B.x = (-line_A.x + tan_theta_t) / (-tan_theta_t * line_A.x - 1);
    } else {
        line_B.x = (-line_A.x - tan_theta_t) / (tan_theta_t * line_A.x - 1);
    }

    line_B.y = y0 - (line_B.x * x0);

    // find distance from origin in line_A that has a distance of d_t from line_B
    float H = d_t/sin(radians(theta_t));

    float a = 1 + pow(line_B.x,2);
    float b = (-2)*x0 + 2*line_B.x*line_B.y - 2*line_B.x*y0;
    float c = pow(x0,2) + pow(y0,2) + pow(line_B.y,2) - 2*y0*line_B.y - pow(H,2);

    float delta = pow(b,2) - 4*a*c;

    Vector2f t1;
    Vector2f t2;

    // find roots
    t1.x = (-b + sqrt(delta))/(2*a);
    t1.y = line_B.x*t1.x + line_B.y;

    t2.x = (-b - sqrt(delta))/(2*a);
    t2.y = line_B.x*t2.x + line_B.y;

    // distance from root points to destination
    float d_t1_p1 = get_distance(t1, destination_NE);
    float d_t2_p1 = get_distance(t2, destination_NE);

    Vector2f t;

    // the first tack point is the one closer to destination
    if(d_t1_p1 < d_t2_p1){
        t = t1;
    } else {
        t = t2;
    }

    // (x_t,y_t) is the first tack point
    local.push_back(t);

    // project p_t in line A (create struct Line (a and b))
    Vector2f p_p = projection(t, line_A);

    // distance between origin and p_p
    float d_p0 = get_distance(origin_NE, p_p);
    d_p0 = d_p0 * 2;

    // find line L1 and L2
    line_L1.y = -line_A.x * t.x + t.y;
    line_L1.x = line_A.x;

    if(line_L1.y > line_A.y) {
        line_L2.y = line_A.y - (line_L1.y - line_A.y);
    } else {
        line_L2.y = line_A.y + (line_A.y - line_L1.y);
    }

    line_L2.x = line_A.x;

    // number of tack points (distance from origin to destination divided by d_p0)
    float n_t = floor(get_distance(origin_NE, destination_NE)/d_p0);

    Vector2f step;
    // step in x and y direction
    step.x = (p_p.x - x0) * 2;
    step.y = (p_p.y - y0) * 2;

    // find first tack point in both lines
    Vector2f p0_L1 = projection(p_p, line_L1);
    Vector2f p0_L2 = projection(p_p, line_L2);

    // run through each line, advancing delta_x,delta_y and find the tack points
    for (int z = 1; z < n_t; z++){
        Vector2f step_tmp;

        step_tmp.x = z * step.x;
        step_tmp.y = z * step.y;

        Vector2f p_tmp;

        // even steps
        if (z % 2 == 0){
            p_tmp.x = p0_L1.x + step_tmp.x;
            p_tmp.y = p0_L1.y + step_tmp.y;
            local.push_back(p_tmp);
        } else {
            p_tmp.x = p0_L2.x + step_tmp.x;
            p_tmp.y = p0_L2.y + step_tmp.y;
            local.push_back(p_tmp);
        }
    }

    // std::vector<Location> tmp = calc_location_from_NE(local);

    // for(std::vector<int>::size_type i = 0; i < tmp.size(); i++){
    //     gcs().send_text(MAV_SEVERITY_INFO, "Tack lat lng %d = (%d, %d)", (int)i, tmp.at(i).lat, tmp.at(i).lng);
    // }

    return local;
}

// get distance between (m) two points
float Sailboat::get_distance(Vector2f origin, const Location &loc2){
    float dlat = (float)(loc2.lat - origin.x);
    float dlng = ((float)(loc2.lng - origin.y)) * loc2.longitude_scale();
    return norm(dlat, dlng) * 0.011131884502145034f;
}

// get distance between (m) two points
float Sailboat::get_distance(Vector2f origin, Vector2f destination){
    float dlat = (float)(destination.x - origin.x);
    float dlng = (float)(destination.y - origin.y);
    return norm(dlat, dlng);
}

// project a point into a line. return the projection point
Vector2f Sailboat::projection(Vector2f point, Vector2f line){
    Vector2f out;
    Vector2f aux;

    if (fabsf(line.x) < 0.0000000001){
        line.x = 0.0000000001;
    }
    if (fabsf(line.y) < 0.0000000001) {
        line.y = 0.0000000001;
    }

    aux.x = -1/line.x;
    aux.y = -aux.x * point.x + point.y;

    out.x = (aux.y - line.y) / (line.x - aux.x);
    out.y = line.x * out.x + line.y;

    return out;
}

std::vector<Location> Sailboat::calc_location_from_NE(std::vector<Vector2f> points_NE){
    std::vector<Location> local;

    for(std::vector<int>::size_type i = 0; i < points_NE.size(); i++){
        Location tmp;

        tmp = rover.g2.wp_nav.get_origin();

        tmp.offset(points_NE.at(i).x, points_NE.at(i).y);

        local.push_back(tmp);
    }

    local.push_back(rover.g2.wp_nav.get_destination());
    local.push_back(rover.g2.wp_nav.get_destination());

    return local;
}

std::vector<Location> Sailboat::calc_tack_points(float desired_heading_cd){
    return calc_location_from_NE(calc_deliberative_tack_points_NE(desired_heading_cd));
}
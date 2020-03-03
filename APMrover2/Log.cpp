#include "Rover.h"

#include <AP_RangeFinder/AP_RangeFinder_Backend.h>

#if LOGGING_ENABLED == ENABLED

// Write an attitude packet
void Rover::Log_Write_Attitude()
{
    float desired_pitch_cd = degrees(g2.attitude_control.get_desired_pitch()) * 100.0f;
    const Vector3f targets(0.0f, desired_pitch_cd, 0.0f);

    logger.Write_Attitude(targets);

#if AP_AHRS_NAVEKF_AVAILABLE
    AP::ahrs_navekf().Log_Write();
    logger.Write_AHRS2();
#endif
    logger.Write_POS();

    // log steering rate controller
    logger.Write_PID(LOG_PIDS_MSG, g2.attitude_control.get_steering_rate_pid().get_pid_info());
    logger.Write_PID(LOG_PIDA_MSG, g2.attitude_control.get_throttle_speed_pid().get_pid_info());

    // log pitch control for balance bots
    if (is_balancebot()) {
        logger.Write_PID(LOG_PIDP_MSG, g2.attitude_control.get_pitch_to_throttle_pid().get_pid_info());
    }

    // log heel to sail control for sailboats
    if (rover.g2.sailboat.sail_enabled()) {
        logger.Write_PID(LOG_PIDR_MSG, g2.attitude_control.get_sailboat_heel_pid().get_pid_info());
        logger.Write_PID(LOG_PIDV_MSG, g2.attitude_control.get_sailboat_speed_pid().get_pid_info());
    }
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
    sitl.Log_Write_SIMSTATE();
#endif
}

// Write a range finder depth message
void Rover::Log_Write_Depth()
{
    // only log depth on boats with working downward facing range finders
    if (!rover.is_boat() || !rangefinder.has_data_orient(ROTATION_PITCH_270)) {
        return;
    }

    // get position
    Location loc;
    if (!ahrs.get_position(loc)) {
        return;
    }

    // check if new sensor reading has arrived
    uint32_t reading_ms = rangefinder.last_reading_ms(ROTATION_PITCH_270);
    if (reading_ms == rangefinder_last_reading_ms) {
        return;
    }
    rangefinder_last_reading_ms = reading_ms;

    logger.Write("DPTH", "TimeUS,Lat,Lng,Depth",
                        "sDUm", "FGG0", "QLLf",
                        AP_HAL::micros64(),
                        loc.lat,
                        loc.lng,
                        (double)(rangefinder.distance_cm_orient(ROTATION_PITCH_270) * 0.01f));
}

// guided mode logging
struct PACKED log_GuidedTarget {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    uint8_t type;
    float pos_target_x;
    float pos_target_y;
    float pos_target_z;
    float vel_target_x;
    float vel_target_y;
    float vel_target_z;
};

// Write a Guided mode target
void Rover::Log_Write_GuidedTarget(uint8_t target_type, const Vector3f& pos_target, const Vector3f& vel_target)
{
    struct log_GuidedTarget pkt = {
        LOG_PACKET_HEADER_INIT(LOG_GUIDEDTARGET_MSG),
        time_us         : AP_HAL::micros64(),
        type            : target_type,
        pos_target_x    : pos_target.x,
        pos_target_y    : pos_target.y,
        pos_target_z    : pos_target.z,
        vel_target_x    : vel_target.x,
        vel_target_y    : vel_target.y,
        vel_target_z    : vel_target.z
    };
    logger.WriteBlock(&pkt, sizeof(pkt));
}

struct PACKED log_Nav_Tuning {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    float wp_distance;
    uint16_t wp_bearing_cd;
    uint16_t nav_bearing_cd;
    uint16_t yaw;
    float xtrack_error;
};

// Write a navigation tuning packet
void Rover::Log_Write_Nav_Tuning()
{
    struct log_Nav_Tuning pkt = {
        LOG_PACKET_HEADER_INIT(LOG_NTUN_MSG),
        time_us             : AP_HAL::micros64(),
        wp_distance         : control_mode->get_distance_to_destination(),
        wp_bearing_cd       : (uint16_t)wrap_360_cd(control_mode->wp_bearing() * 100),
        nav_bearing_cd      : (uint16_t)wrap_360_cd(control_mode->nav_bearing() * 100),
        yaw                 : (uint16_t)ahrs.yaw_sensor,
        xtrack_error        : control_mode->crosstrack_error()
    };
    logger.WriteBlock(&pkt, sizeof(pkt));
}

void Rover::Log_Write_Sail()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    // get wind direction
    float wind_dir_abs = logger.quiet_nanf();
    float wind_dir_rel = logger.quiet_nanf();
    float wind_speed_true = logger.quiet_nanf();
    float wind_speed_apparent = logger.quiet_nanf();
    if (rover.g2.windvane.enabled()) {
        wind_dir_abs = degrees(g2.windvane.get_true_wind_direction_rad());
        wind_dir_rel = degrees(g2.windvane.get_apparent_wind_direction_rad());
        wind_speed_true = g2.windvane.get_true_wind_speed();
        wind_speed_apparent = g2.windvane.get_apparent_wind_speed();
    }
    logger.Write("SAIL", "TimeUS,WindDirTrue,WindDirApp,WindSpdTrue,WindSpdApp,SailOut,VMG",
                        "shhnn%n", "F000000", "Qffffff",
                        AP_HAL::micros64(),
                        (double)wind_dir_abs,
                        (double)wind_dir_rel,
                        (double)wind_speed_true,
                        (double)wind_speed_apparent,
                        (double)g2.motors.get_mainsail(),
                        (double)g2.sailboat.get_VMG());
}

void Rover::Log_Write_Metrics1()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    logger.Write("MTC1", "TimeUS,IAE,ISE,ITAE,ITSE,IAEW,IADC,IAEIADC",
                        "smmmmmmm", "F0000000", "Qfffffff",
                        AP_HAL::micros64(),
                        (double)rover.g2.metrics.get_IAE_steering(),
                        (double)rover.g2.metrics.get_ISE_steering(),
                        (double)rover.g2.metrics.get_ITAE_steering(),
                        (double)rover.g2.metrics.get_ITSE_steering(),
                        (double)rover.g2.metrics.get_IAEW_steering(),
                        (double)rover.g2.metrics.get_IADC_steering(),
                        (double)rover.g2.metrics.get_IAE_IADC_steering());
}

void Rover::Log_Write_Metrics2()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    logger.Write("MTC2", "TimeUS,IAE,ISE,ITAE,ITSE,IAEW,IADC,IAEIADC",
                        "smmmmmmm", "F0000000", "Qfffffff",
                        AP_HAL::micros64(),
                        (double)rover.g2.metrics.get_IAE_speed(),
                        (double)rover.g2.metrics.get_ISE_speed(),
                        (double)rover.g2.metrics.get_ITAE_speed(),
                        (double)rover.g2.metrics.get_ITSE_speed(),
                        (double)rover.g2.metrics.get_IAEW_speed(),
                        (double)rover.g2.metrics.get_IADC_speed(),
                        (double)rover.g2.metrics.get_IAE_IADC_speed());
}

void Rover::Log_Write_Metrics3()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    logger.Write("MTC3", "TimeUS,NumberTacks",
                        "sm", "F0", "Qf",
                        AP_HAL::micros64(),
                        (double)g2.metrics.get_number_of_tacks());
}

void Rover::Log_Write_External_Current()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    logger.Write("ECUR", "TimeUS,Samp,Smah,Swh,Ramp,Rmah,Rwh",
                        "smmmmmm", "F000000", "Qffffff",
                        AP_HAL::micros64(),
                        (double)rover.g2.sail_current,
                        (double)rover.battery.get_sail_consumed_mah(),
                        (double)rover.battery.get_sail_consumed_wh(),
                        (double)rover.g2.rudder_current,
                        (double)rover.battery.get_rudder_consumed_mah(),
                        (double)rover.battery.get_rudder_consumed_wh());
}

void Rover::Log_Write_Actuators_Status()
{
    // only log sail if present
    if (!rover.g2.sailboat.sail_enabled()) {
        return;
    }

    logger.Write("ACT1", "TimeUS,SailAng,SailMot,RudAng,RudMot",
                        "smmmm", "F0000", "Qffff",
                        AP_HAL::micros64(),
                        (double)g2.sail_angle,
                        (double)g2.sail_motor,
                        (double)g2.rudder_angle,
                        (double)g2.rudder_motor);
}

struct PACKED log_Steering {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    int16_t steering_in;
    float steering_out;
    float desired_lat_accel;
    float lat_accel;
    float desired_turn_rate;
    float turn_rate;
};

struct PACKED log_Startup {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    uint8_t startup_type;
    uint16_t command_total;
};

void Rover::Log_Write_Startup(uint8_t type)
{
    struct log_Startup pkt = {
        LOG_PACKET_HEADER_INIT(LOG_STARTUP_MSG),
        time_us         : AP_HAL::micros64(),
        startup_type    : type,
        command_total   : mode_auto.mission.num_commands()
    };
    logger.WriteBlock(&pkt, sizeof(pkt));
}

// Write a steering packet
void Rover::Log_Write_Steering()
{
    float lat_accel = logger.quiet_nanf();
    g2.attitude_control.get_lat_accel(lat_accel);
    struct log_Steering pkt = {
        LOG_PACKET_HEADER_INIT(LOG_STEERING_MSG),
        time_us        : AP_HAL::micros64(),
        steering_in        : channel_steer->get_control_in(),
        steering_out       : g2.motors.get_steering(),
        desired_lat_accel  : control_mode->get_desired_lat_accel(),
        lat_accel          : lat_accel,
        desired_turn_rate  : degrees(g2.attitude_control.get_desired_turn_rate()),
        turn_rate          : degrees(ahrs.get_yaw_rate_earth())
    };
    logger.WriteBlock(&pkt, sizeof(pkt));
}

struct PACKED log_Throttle {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    int16_t throttle_in;
    float throttle_out;
    float desired_speed;
    float speed;
    float accel_y;
};

// Write a throttle control packet
void Rover::Log_Write_Throttle()
{
    const Vector3f accel = ins.get_accel();
    float speed = logger.quiet_nanf();
    g2.attitude_control.get_forward_speed(speed);
    struct log_Throttle pkt = {
        LOG_PACKET_HEADER_INIT(LOG_THR_MSG),
        time_us         : AP_HAL::micros64(),
        throttle_in     : channel_throttle->get_control_in(),
        throttle_out    : g2.motors.get_throttle(),
        desired_speed   : g2.attitude_control.get_desired_speed(),
        speed           : speed,
        accel_y         : accel.y
    };
    logger.WriteBlock(&pkt, sizeof(pkt));
}

void Rover::Log_Write_RC(void)
{
    logger.Write_RCIN();
    logger.Write_RCOUT();
    if (rssi.enabled()) {
        logger.Write_RSSI();
    }
}

void Rover::Log_Write_Vehicle_Startup_Messages()
{
    // only 200(?) bytes are guaranteed by AP_Logger
    Log_Write_Startup(TYPE_GROUNDSTART_MSG);
    logger.Write_Mode(control_mode->mode_number(), control_mode_reason);
    ahrs.Log_Write_Home_And_Origin();
    gps.Write_AP_Logger_Log_Startup_messages();
}

void Rover::Log_Write_Metrics(){
    // reset metrics (when it gets number 2)
    if(mode_auto.mission.get_current_nav_index() == 1 && g2.metrics.isClearMetrics){
        g2.metrics.reset_metrics();
        g2.metrics.isClearMetrics = false;
        gcs().send_text(MAV_SEVERITY_WARNING, "CLEARED METRICS!!!");
    }

    if(rover.mode_auto.mission.get_current_nav_index() != 1){
        g2.metrics.isClearMetrics = true;
    }

    Log_Write_Metrics1();
    Log_Write_Metrics2();
    Log_Write_Metrics3();
    Log_Write_External_Current();
    Log_Write_Actuators_Status();
}

// type and unit information can be found in
// libraries/AP_Logger/Logstructure.h; search for "log_Units" for
// units and "Format characters" for field type information
const LogStructure Rover::log_structure[] = {
    LOG_COMMON_STRUCTURES,
    { LOG_STARTUP_MSG, sizeof(log_Startup),
      "STRT", "QBH",        "TimeUS,SType,CTot", "s--", "F--" },
    { LOG_THR_MSG, sizeof(log_Throttle),
      "THR", "Qhffff", "TimeUS,ThrIn,ThrOut,DesSpeed,Speed,AccY", "s--nno", "F--000" },
    { LOG_NTUN_MSG, sizeof(log_Nav_Tuning),
      "NTUN", "QfHHHf", "TimeUS,WpDist,WpBrg,DesYaw,Yaw,XTrack", "smdddm", "F0BBB0" },
    { LOG_STEERING_MSG, sizeof(log_Steering),
      "STER", "Qhfffff",   "TimeUS,SteerIn,SteerOut,DesLatAcc,LatAcc,DesTurnRate,TurnRate", "s--ookk", "F--0000" },
    { LOG_GUIDEDTARGET_MSG, sizeof(log_GuidedTarget),
      "GUID",  "QBffffff",    "TimeUS,Type,pX,pY,pZ,vX,vY,vZ", "s-mmmnnn", "F-000000" },
};

void Rover::log_init(void)
{
    logger.Init(log_structure, ARRAY_SIZE(log_structure));
}

#else  // LOGGING_ENABLED

// dummy functions
void Rover::Log_Write_Attitude() {}
void Rover::Log_Write_Depth() {}
void Rover::Log_Write_GuidedTarget(uint8_t target_type, const Vector3f& pos_target, const Vector3f& vel_target) {}
void Rover::Log_Write_Nav_Tuning() {}
void Rover::Log_Write_Sail() {}
void Rover::Log_Write_Startup(uint8_t type) {}
void Rover::Log_Write_Throttle() {}
void Rover::Log_Write_RC(void) {}
void Rover::Log_Write_Steering() {}
void Rover::Log_Write_Vehicle_Startup_Messages() {}
void Rover::Log_Write_Metrics() {}
void Rover::Log_Write_Metrics1() {}
void Rover::Log_Write_Metrics2() {}
void Rover::Log_Write_Metrics3() {}
void Rover::Log_Write_External_Current() {}
void Rover::Log_Write_Actuators_Status() {}

#endif  // LOGGING_ENABLED

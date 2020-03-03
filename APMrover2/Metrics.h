
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
    Movement metrics for sailboat
*/
class Metrics{
public:

    // constructor
    Metrics();

    // update all metrics
    void update_metrics();

    // update all errors
    void update_error();

    // update all IAE metrics
    void update_IAE();

    // update all ISE metrics
    void update_ISE();

    // update all ITAE metrics
    void update_ITAE();

    // update all ITSE metrics
    void update_ITSE();

    // update all IAEW metrics
    void update_IAEW();

    // update all IADC metrics
    void update_IADC();

    // update all IAE_IADC metrics
    void update_IAE_IADC();

    // calculate IAE
    float calc_IAE(float error);

    // calculate ISE
    float calc_ISE(float error);

    // calculate ITAE
    float calc_ITAE(float error);    

    // calculate ITSE
    float calc_ITSE(float error);

    // calculate IAEW (int{abs(e) dt}*int{P dt}))
    float calc_IAEW(float IAE, float consumed_wh);   

    // calculate IADC
    float calc_IADC(float diff_actuator);  

    // calculate IAE_IADC
    float calc_IAE_IADC(float IAE, float IADC);  

    // get speed error from PID
    void get_error_speed();

    // get steering error from PID
    void get_error_steering();

    // get start time of mission
    void save_arm_time(float arm_time) {_arm_time = arm_time;}

    // get start wh of mission
    void save_arm_wh(float arm_wh) {_arm_wh = arm_wh;}


    float get_IAE_steering() {return _IAE_steering;}
    float get_ISE_steering() {return _ISE_steering;}
    float get_ITAE_steering() {return _ITAE_steering;}
    float get_ITSE_steering() {return _ITSE_steering;}
    float get_IAEW_steering() {return _IAEW_steering;}
    float get_IADC_steering() {return _IADC_steering;}
    float get_IAE_IADC_steering() {return _IAE_IADC_steering;}

    float get_IAE_speed() {return _IAE_speed;}
    float get_ISE_speed() {return _ISE_speed;}
    float get_ITAE_speed() {return _ITAE_speed;}
    float get_ITSE_speed() {return _ITSE_speed;}
    float get_IAEW_speed() {return _IAEW_speed;}
    float get_IADC_speed() {return _IADC_speed;}
    float get_IAE_IADC_speed() {return _IAE_IADC_speed;}    

    void reset_metrics();

    void increment_tack();

    float get_number_of_tacks() {return _number_of_tacks;}

    bool isClearMetrics = true;

private:

    // feedback control errors
    float _error_steering;
    float _error_speed;

    // time of mission start
    float _arm_time;

    // wh of mission start
    float _arm_wh;

    // time since started mission
    float _current_time;

    // wh since started mission
    float _current_wh;

    // difference off actuators
    float _diff_steering;
    float _diff_throttle;

    // last value of actuators
    float _last_steering;
    float _last_throttle;

    // IAE variables
    float _IAE_speed;
    float _IAE_steering;

    // ISE variables
    float _ISE_speed;
    float _ISE_steering;

    // ITAE variables
    float _ITAE_speed;
    float _ITAE_steering;

    // ITSE variables
    float _ITSE_speed;
    float _ITSE_steering;

    // IAEW variables
    float _IAEW_speed;
    float _IAEW_steering;

    // IADC variables
    float _IADC_speed;
    float _IADC_steering;

    // IAE_IADC variables
    float _IAE_IADC_speed;
    float _IAE_IADC_steering;

    float _number_of_tacks;
};

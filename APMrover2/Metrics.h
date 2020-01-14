
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
    void update_Metrics();

    // update all errors
    void update_error();

    // update all IAE metrics
    void update_IAE();

    // update all ISE metrics
    void update_ISE();

    // calculate IAE
    float calc_IAE(float error);

    // calculate ISE
    float calc_ISE(float error);

    // get speed error from PID
    void get_error_speed();

    // get steering error from PID
    void get_error_steering();

    // get start time of mission
    void save_arm_time(float arm_time) {_arm_time = arm_time;}

private:

    // feedback control errors
    float _error_steering;
    float _error_speed;

    // IAE variables
    float _IAE_speed;
    float _IAE_steering;

    // ISE variables
    float _ISE_speed;
    float _ISE_steering;

    // time of start mission
    float _arm_time;
};

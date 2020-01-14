
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

    void update_Metrics();

    void update_error();

    // calculate IAE metric. need error info
    void update_IAE();

    // calculate ISE metric. need error info
    void update_ISE();

    float calc_IAE(float error);

    float calc_ISE(float error);

    void get_error_speed();

    void get_error_steering();

    void save_arm_time(float arm_time) {_arm_time = arm_time;}

private:

    float _IAE_speed;
    float _ISE_speed;

    float _IAE_steering;
    float _ISE_steering;

    float _error_steering;
    float _error_speed;

    float _arm_time;
};

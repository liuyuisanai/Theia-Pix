/****************************************************************************
 *
 *   Copyright (c) 2013 PX4 Development Team. All rights reserved.
 *   Author: @author Anton Babushkin <anton.babushkin@me.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file mc_pos_control_params.c
 * Multicopter position controller parameters.
 *
 * @author Anton Babushkin <anton.babushkin@me.com>
 */

#include <systemlib/param/param.h>

/**
 * Camera pitch correction step
 *
 * @min 0.0
 * @max 1.0
 * @group camera
 * */
PARAM_DEFINE_FLOAT(CAM_PITCH_STEP, 0.05f);

/**
 * Camera yaw correction step
 *
 * @min 0.0
 * @max 1.0
 * @group camera
 * */
PARAM_DEFINE_FLOAT(CAM_YAW_STEP, 5.0f);

/**
 * Allowed landing speed
 *
 * @min max unlimited
 * @group Multicopter Position Control
 * */
PARAM_DEFINE_FLOAT(MPC_ALLOWED_LAND, 5.0f);

/**
 * Minimum thrust
 *
 * Minimum vertical thrust. It's recommended to set it > 0 to avoid free fall with zero thrust.
 *
 * @min 0.0
 * @max 1.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_THR_MIN, 0.1f);

/**
 * Maximum thrust
 *
 * Limit max allowed thrust.
 *
 * @min 0.0
 * @max 1.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_THR_MAX, 1.0f);

/**
 * Proportional gain for vertical position error
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_P, 1.0f);

/**
 * Proportional gain for vertical velocity error
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_VEL_P, 0.1f);

/**
 * Integral gain for vertical velocity error
 *
 * Non zero value allows hovering thrust estimation on stabilized or autonomous takeoff.
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_VEL_I, 0.02f);

/**
 * Differential gain for vertical velocity error
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_VEL_D, 0.0f);

/**
 * Maximum vertical velocity
 *
 * Maximum vertical velocity in AUTO mode and endpoint for stabilized modes (ALTCTRL, POSCTRL).
 *
 * @unit m/s
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_VEL_MAX, 5.0f);

/**
 * Vertical velocity feed forward
 *
 * Feed forward weight for altitude control in stabilized modes (ALTCTRL, POSCTRL). 0 will give slow responce and no overshot, 1 - fast responce and big overshot.
 *
 * @min 0.0
 * @max 1.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_Z_FF, 0.5f);

/**
 * Proportional gain for horizontal position error
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_P, 1.0f);

/**
 * Proportional gain for horizontal velocity error
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_VEL_P, 0.1f);

/**
 * Integral gain for horizontal velocity error
 *
 * Non-zero value allows to resist wind.
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_VEL_I, 0.02f);

/**
 * Differential gain for horizontal velocity error. Small values help reduce fast oscillations. If value is too big oscillations will appear again.
 *
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_VEL_D, 0.01f);

/**
 * Maximum horizontal velocity
 *
 * Maximum horizontal velocity in AUTO mode and endpoint for position stabilized mode (POSCTRL).
 *
 * @unit m/s
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_VEL_MAX, 5.0f);

/**
 * Horizontal velocity feed forward
 *
 * Feed forward weight for position control in position control mode (POSCTRL). 0 will give slow responce and no overshot, 1 - fast responce and big overshot.
 *
 * @min 0.0
 * @max 1.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_XY_FF, 0.5f);

/**
 * Maximum tilt angle in air
 *
 * Limits maximum tilt in AUTO and POSCTRL modes during flight.
 *
 * @unit deg
 * @min 0.0
 * @max 90.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_TILTMAX_AIR, 45.0f);

/**
 * Maximum tilt during landing
 *
 * Limits maximum tilt angle on landing.
 *
 * @unit deg
 * @min 0.0
 * @max 90.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_TILTMAX_LND, 15.0f);

/**
 * Landing descend rate
 *
 * @unit m/s
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_LAND_SPD, 1.0f);

/**
 * Takeoff ascend rate
 *
 * @unit m/s
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(MPC_TAKEOFF_SPD, 1.0f);

/**
 * Follow mode velocity feed-forward
 *
 * Target velocity feed-forward in follow mode.
 *
 * @min 0.0
 * @max 1.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(FOL_VEL_FF, 1.0f);

/**
 * Follow mode initial altitude offset
 *
 * Altitude offset (target_alt - copter_alt) on copter arming, for altitude estimates synchronization.
 *
 * @unit meters
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(FOL_TALT_OFF, 0.0f);

/**
 * Follow mode max yaw offset
 *
 * Maximum yaw offset in FOLLOW mode that can be set by stick.
 *
 * @unit deg
 * @min 0.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(FOL_YAW_OFF_MAX, 45.0f);

/**
 * Use altitude of target
 *
 * 0 (disabled): ignore altitude reported by target, assuming that altitude is constant, home_alt + FOL_ALT_OFF.
 * 1 (enabled): use altitude reported by target to control camera pitch and (if FOL_RPT_ALT == 1) own altitude.
 *
 * @min 0
 * @max 1
 * @group Multicopter Position Control
 */
PARAM_DEFINE_INT32(FOL_USE_ALT, 1);

/**
 * Repeat target altitude
 *
 * 0 (disabled): hold constant altitude.
 * 1 (enabled): repeat target altitude changes, i.e. hold constant altitude difference.
 *
 * @min 0
 * @max 1
 * @group Multicopter Position Control
 */
PARAM_DEFINE_INT32(FOL_RPT_ALT, 0);

/**
 * Low pass filter for target velocity
 *
 * Time constant for low pass filter to smooth velocity jumps for aggressive targets.
 *
 * @unit s
 * @min 0.0
 * @max 10.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(FOL_LPF, 1.0f);

/**
 * Maximum camera pitch
 *
 * Camera pitch (degrees) for control signal of 1 (assuming that camera pitch is 0 when control signal is 0).
 *
 * @unit deg
 * @min 0.0
 * @max 180.0
 * @group Multicopter Position Control
 */
PARAM_DEFINE_FLOAT(CAM_P_MAX, 90.0f);

/**
 * Yaw dead zone radius, yaw wont work if target in XY plane will be closer than
 * dead zone radius
 * @unit meters
 * @min 0.0
 * @max 50.0
 *
**/
PARAM_DEFINE_FLOAT(A_YAW_DEAD_Z_R, 2.0f);


/**
 * Yaw gardient zone radius, speed of yaw in this zone will be determined
 * where between GRAD ZONE and DEAD ZONE on XY plane target is positioned
 * @unit meters
 * @min 0.0
 * @max 50.0
 *
**/
PARAM_DEFINE_FLOAT(A_YAW_GRAD_Z_R, 2.0f);

/**
 * Cutoff frequency of LPF filter on camera pitch
 * If set negative, then only LPF filter will be used.
 * If set to 0, then LPF won't be used (just delta correction).
 * If set positive, then LPF + delta correction will be used
 */
PARAM_DEFINE_FLOAT(MPC_PITCH_LPF, 0.0f);

#pragma once

#include <ev3dev.h>

#include <chrono>
#include <thread>
#include <boost/log/trivial.hpp>
#include <boost/math/special_functions.hpp>

using namespace std::chrono;

#include <iostream>

class Motors
{
public:
    Motors() : armMotor(ev3dev::OUTPUT_A), turntableMotor(ev3dev::OUTPUT_B)/*, scannerMotor(ev3dev::OUTPUT_C)*/ {}

    void turn_table(int turns, bool doOvershoot = true)
    {
        int sign = turns >= 0 ? 1 : -1;
        int degrees = turntableCountsPerTurn * turns;
        if (doOvershoot)
        {
            turntableMotor.set_position_sp(degrees + sign * turntableMotorOvershoot).run_to_rel_pos();
            block_motor(turntableMotor);
            turntableMotor.set_position_sp(-sign * turntableMotorOvershoot).run_to_rel_pos();
            block_motor(turntableMotor);
        }
        else
        {
            turntableMotor.set_position_sp(degrees).run_to_rel_pos();
            block_motor(turntableMotor);
        }
    }

    void init()
    {
        BOOST_LOG_TRIVIAL(info) << "Initializing Motors";

        BOOST_LOG_TRIVIAL(debug) << "Initializing Arm";
        armMotor.set_stop_action(ev3dev::motor::stop_action_hold);
        armMotor.set_speed_sp(-armMotorSpeed).set_time_sp(5000).run_timed();
        block_motor(armMotor, true);
        armMotor.set_position(-10);
        armMotor.set_position_sp(0);
        armMotor.run_to_abs_pos();
        block_motor(armMotor);

        // BOOST_LOG_TRIVIAL(debug) << "Initializing Scanner";
        // armMotor.set_stop_action(ev3dev::motor::stop_action_hold);
        // scannerMotor.set_speed_sp(scannerMotorSpeed / 5).set_time_sp(5000).run_timed();
        // block_motor(scannerMotor, true);
        // scannerMotor.set_speed_sp(scannerMotorSpeed);
        // scannerMotor.set_position(20);
        // scannerMotor.set_position_sp(0);
        // scannerMotor.run_to_abs_pos();
        // block_motor(scannerMotor);

        BOOST_LOG_TRIVIAL(debug) << "Initializing Turntable";
        turntableMotor.set_stop_action(ev3dev::motor::stop_action_hold);
        turntableMotor.set_speed_sp(turntableMotorSpeed);
        turn_table(1, false);
        turn_table(-1, false);
        turn_table(1, true);
        turn_table(-1, true);

        std::this_thread::sleep_for(seconds(1));
        BOOST_LOG_TRIVIAL(info) << "Done";
    }

    void clean_exit()
    {
        release_motor(armMotor);
        // release_motor(scannerMotor);
        release_motor(turntableMotor);
    }

private:
    ev3dev::motor armMotor;
    const int armMotorSpeed = armMotor.max_speed() * 25 / 100;

    ev3dev::motor turntableMotor;
    const int turntableMotorSpeed = turntableMotor.max_speed() * 50 / 100;
    const int turntableMotorOvershoot = 50;
    const int turntableCountsPerTurn = 270;

    // ev3dev::motor scannerMotor;
    // const int scannerMotorSpeed = scannerMotor.max_speed() * 50 / 100;
    // const int scannerPositionHold = -200;
    // const int scannerPositionCenter = -750;
    // const int scannerPositionEdge = -625;
    // const int scannerPositionCorner = -575;

    void release_motor(ev3dev::motor motor)
    {
        motor.set_stop_action(ev3dev::motor::stop_action_coast);
        motor.stop();
    }

    void block_motor(ev3dev::motor motor, bool onStallled = false, int stopped_timeout = 0, int running_timeout = 100)
    {
        auto start = high_resolution_clock::now();

        while (running_timeout > 0 || ms_ellapsed_since(start) < milliseconds(running_timeout))
        {
            std::this_thread::sleep_for(milliseconds(10));
            auto state = motor.state();
            if (state.count(ev3dev::motor::state_running))
                break;
            BOOST_LOG_TRIVIAL(trace) << "Not running";
        }
        while (stopped_timeout > 0 || ms_ellapsed_since(start) < milliseconds(stopped_timeout))
        {
            std::this_thread::sleep_for(milliseconds(10));
            auto state = motor.state();

            if (!state.count(ev3dev::motor::state_running) || (onStallled && state.count(ev3dev::motor::state_stalled)))
                break;

            // Prints the state
            // std::ostringstream stream;
            // std::copy(state.begin(), state.end(), std::ostream_iterator<std::string>(stream, ","));
            // BOOST_LOG_TRIVIAL(trace) << "State: " << stream.str();
        }
    }

    milliseconds ms_ellapsed_since(system_clock::time_point time)
    {
        return duration_cast<milliseconds>(time - high_resolution_clock::now());
    }
};

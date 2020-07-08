#include <ev3dev.h>

#include <chrono>
#include <thread>
#include <boost/log/trivial.hpp>
#include <boost/math/special_functions.hpp>

#include "motors.hpp"

namespace motors
{
    using namespace std::chrono;
    namespace
    {
        ev3dev::motor armMotor(ev3dev::OUTPUT_A);
        const int armMotorSpeed = armMotor.max_speed() * 25 / 100;

        ev3dev::motor turntableMotor(ev3dev::OUTPUT_B);
        const int turntableMotorSpeed = turntableMotor.max_speed() * 75 / 100;
        constexpr int turntableMotorOvershoot = 50;
        constexpr int turntableCountsPerTurn = 270;

        milliseconds ms_ellapsed_since(system_clock::time_point time)
        {
            return duration_cast<milliseconds>(high_resolution_clock::now() - time);
        }

        void releaseMotor(ev3dev::motor motor)
        {
            motor.set_stop_action(ev3dev::motor::stop_action_coast);
            motor.set_speed_sp(0);
            motor.stop();
        }

        void blockMotor(ev3dev::motor motor, int stopped_timeout = 0, int running_timeout = 50)
        {
            auto start = high_resolution_clock::now();

            while (running_timeout <= 0 || ms_ellapsed_since(start) < milliseconds(running_timeout))
            {
                std::this_thread::sleep_for(milliseconds(10));
                auto state = motor.state();
                if (state.count(ev3dev::motor::state_running))
                    break;
                // BOOST_LOG_TRIVIAL(trace) << "Not running";
            }
            while (stopped_timeout <= 0 || ms_ellapsed_since(start) < milliseconds(stopped_timeout))
            {
                std::this_thread::sleep_for(milliseconds(10));
                auto state = motor.state();

                if (!state.count(ev3dev::motor::state_running) || state.count(ev3dev::motor::state_stalled))
                    break;
            }
        }
    } // namespace

    void init()
    {
        BOOST_LOG_TRIVIAL(info) << "Initializing Motors";

        BOOST_LOG_TRIVIAL(debug) << "Initializing Arm";
        armMotor.set_stop_action(ev3dev::motor::stop_action_hold);
        armMotor.set_speed_sp(-armMotorSpeed).set_time_sp(5000).run_timed();
        blockMotor(armMotor);
        armMotor.set_position(-20);
        armMotor.set_position_sp(0);
        armMotor.run_to_abs_pos();
        blockMotor(armMotor);

        BOOST_LOG_TRIVIAL(debug) << "Initializing Turntable";
        turntableMotor.set_stop_action(ev3dev::motor::stop_action_hold);
        turntableMotor.set_speed_sp(turntableMotorSpeed);
        turnTable(1, false);
        turnTable(-1, false);

        std::this_thread::sleep_for(seconds(1));
        BOOST_LOG_TRIVIAL(info) << "Done";
    }

    void cleanExit()
    {
        releaseMotor(armMotor);
        releaseMotor(turntableMotor);
    }

    void turnTable(int turns, bool doOvershoot)
    {
        int sign = turns >= 0 ? 1 : -1;
        int degrees = turntableCountsPerTurn * turns;
        if (doOvershoot)
        {
            turntableMotor.set_position_sp(degrees + sign * turntableMotorOvershoot).run_to_rel_pos();
            blockMotor(turntableMotor);
            turntableMotor.set_position_sp(-sign * turntableMotorOvershoot).run_to_rel_pos();
            blockMotor(turntableMotor);
        }
        else
        {
            turntableMotor.set_position_sp(degrees).run_to_rel_pos();
            blockMotor(turntableMotor);
        }
    }

    void moveArm(ArmPosition position){
        
        if(position == ArmPosition::UP)
            armMotor.set_position_sp(0);
        else if (position == ArmPosition::DOWN)
            armMotor.set_position_sp(100);
        else if (position == ArmPosition::SETTLE)
            armMotor.set_position_sp(75);
        else if (position == ArmPosition::FLIP)
            armMotor.set_position_sp(180);

        armMotor.run_to_abs_pos();
        blockMotor(armMotor);
    }

    void rotateCube(int turns){
        moveArm(ArmPosition::UP);
        turnTable(turns, false);
    }

    void rotateFace(int turns){
        moveArm(ArmPosition::DOWN);
        turnTable(turns, true);
    }

    void flipCube(int flips){
        for (int i = 0; i < flips; i++)
        {
            moveArm(ArmPosition::FLIP);
            moveArm(ArmPosition::SETTLE);
        }
        moveArm(ArmPosition::DOWN);
    }
}; // namespace motors

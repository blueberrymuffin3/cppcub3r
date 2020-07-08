#include <ev3dev.h>
#include <kociemba.h>

#include <thread>
#include <csignal>
#include <boost/log/trivial.hpp>

#include "motors.hpp"
#include "detection.hpp"
#include "solver.hpp"

static void cleanExit(int status = 0)
{

    ev3dev::led::green_left.set_brightness(ev3dev::led::green_left.max_brightness());
    ev3dev::led::red_left.set_brightness(0);
    ev3dev::led::green_right.set_brightness(ev3dev::led::green_left.max_brightness());
    ev3dev::led::red_right.set_brightness(0);

    motors::cleanExit();

    exit(status);
}

static void signalHandler(int signum)
{
    BOOST_LOG_TRIVIAL(error)
        << "Interrupt signal (" << signum << ") received.";

    cleanExit(signum);
}

void scanAndSolve()
{

    std::string cubestring = detection::scanCube();
    BOOST_LOG_TRIVIAL(info) << "Cubestring: " << cubestring;
    const char *solution = solve(cubestring.data());
    BOOST_LOG_TRIVIAL(info) << "Solution: " << solution;

    solver::runSolver(solution);

    motors::rotateCube(4);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void _main()
{
    motors::init();
    detection::init();
    while (true)
    {
        ev3dev::led::red_right.set_brightness(255);
        BOOST_LOG_TRIVIAL(info) << "Press enter to start";
        while (!ev3dev::button::enter.pressed())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        ev3dev::led::red_right.set_brightness(0);

        scanAndSolve();
    }
}

int main()
{
    signal(SIGINT, signalHandler);
    try
    {
        _main();
    }
    catch (const std::exception &e)
    {
        BOOST_LOG_TRIVIAL(error) << e.what() << '\n';
    }
    cleanExit();
}

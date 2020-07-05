#include <ev3dev.h>

#include <csignal>
#include <boost/log/trivial.hpp>

#include "motors.hpp"

Motors motors;

static void clean_exit(int status = 0)
{

    ev3dev::led::green_left.set_brightness(ev3dev::led::green_left.max_brightness());
    ev3dev::led::red_left.set_brightness(0);
    ev3dev::led::green_right.set_brightness(ev3dev::led::green_left.max_brightness());
    ev3dev::led::red_right.set_brightness(0);

    motors.clean_exit();

    exit(status);
}

static void signalHandler(int signum)
{
    BOOST_LOG_TRIVIAL(error)
        << "Interrupt signal (" << signum << ") received.";

    clean_exit(signum);
}

int main()
{
    signal(SIGINT, signalHandler);

    motors.init();

    clean_exit();
}

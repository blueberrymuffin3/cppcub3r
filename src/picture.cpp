#include <csignal>

#include <boost/log/trivial.hpp>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

static void signalHandler(int signum)
{
    BOOST_LOG_TRIVIAL(error)
        << "Interrupt signal (" << signum << ") received.";
    exit(signum);
}

constexpr bool useWebcam = true;

int main()
{
    signal(SIGINT, signalHandler);

    BOOST_LOG_TRIVIAL(info) << "Taking Photo...";

    cv::Mat frame;
    cv::VideoCapture camera;
    camera.open(0, cv::CAP_ANY);
    int frameNumber = 0;
    
    camera.read(frame);

    BOOST_LOG_TRIVIAL(info) << "Saving Photo to \"capture.jpeg\"...";
    cv::imwrite("capture.png", frame);

    BOOST_LOG_TRIVIAL(info) << "Done!";
}

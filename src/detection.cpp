#include <ev3dev.h>
#include <kociemba.h>

#include <csignal>

#include <boost/log/trivial.hpp>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

constexpr int tile_x_size = 80;
constexpr int tile_x_start = 120;
constexpr int tile_x_offset = 60;
constexpr int tile_x_count = 3;

constexpr int tile_y_size = 80;
constexpr int tile_y_start = 50;
constexpr int tile_y_offset = 60;
constexpr int tile_y_count = 3;

static void signalHandler(int signum)
{
    BOOST_LOG_TRIVIAL(error)
        << "Interrupt signal (" << signum << ") received.";
    exit(signum);
}

std::vector<cv::Scalar> scanSide(cv::Mat _frame)
{
    cv::Mat frame(_frame);
    _frame.copyTo(frame);

    // BOOST_LOG_TRIVIAL(info) << "Converting Colors...";
    // cv::cvtColor(frame, frame, cv::COLOR_RGB2Lab);
    BOOST_LOG_TRIVIAL(info) << "Averaging Colors...";

    std::vector<cv::Scalar> colors(tile_x_count * tile_y_count);

    for (int j = 0; j < tile_y_count; j++)
    {
        for (int i = 0; i < tile_x_count; i++)
        {
            const int x = tile_x_size + (tile_x_offset + tile_x_size) * i;
            const int y = tile_y_size + (tile_y_offset + tile_y_size) * j;
            const int index = i + j * tile_x_count;

            cv::Mat submat(frame, cv::Rect2i(x, y, tile_x_size, tile_y_size));

            colors[index] = cv::mean(submat);
        }
    }

    BOOST_LOG_TRIVIAL(info) << "Done";

    std::ostringstream data;

    data << "[";
    std::copy(colors.begin(), colors.end() - 1,
              std::ostream_iterator<cv::Scalar>(data, ", "));
    data << "]";

    BOOST_LOG_TRIVIAL(info) << data.str();

    return colors;
}

constexpr bool useWebcam = true;

int main()
{
    signal(SIGINT, signalHandler);

    BOOST_LOG_TRIVIAL(info) << "Benchmark Solve...";
    BOOST_LOG_TRIVIAL(info) << solve("DRLUUBFBRBLURRLRUBLRDDFDLFUFUFFDBRDUBRUFLLFDDBFLUBLRBD");
    BOOST_LOG_TRIVIAL(info) << "DONE";

    if (useWebcam)
    {
        BOOST_LOG_TRIVIAL(info) << "Streaming from webcam...";
        cv::Mat frame;
        cv::VideoCapture camera;
        camera.open(0, cv::CAP_ANY);
        while (true)
        {
            BOOST_LOG_TRIVIAL(info) << "Getting frame from webcam...";
            camera.read(frame);

            scanSide(frame);
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Loading 10 Images...";
        cv::Mat frame;
        std::string prefix = "capture-";
        std::string suffix = ".jpeg";
        std::string filename;
        for (int i = 0; i < 10; i++)
        {
            filename = prefix + std::to_string(i) + suffix;
            BOOST_LOG_TRIVIAL(info) << "Loading frame... (" << filename << ")";
            frame = cv::imread(filename);

            scanSide(frame);
        }
    }
}

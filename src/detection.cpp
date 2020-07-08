#include <ev3dev.h>
#include <kociemba.h>

#include <iostream>
#include <thread>

#include <boost/log/trivial.hpp>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "motors.hpp"

#include "detection.hpp"

namespace detection
{
    namespace
    {
        cv::VideoCapture camera;
        cv::Mat frame;
        cv::Mat disp;
        int fileCounter = 0;

        std::vector<cv::Point2i> sections = {
            cv::Point2i(183, 161),
            cv::Point2i(328, 166),
            cv::Point2i(476, 162),

            cv::Point2i(162, 285),
            cv::Point2i(330, 298),
            cv::Point2i(503, 287),
        };

        cv::Point2i sectionOffset(10, 10);

        // indexMeanMap[outputSectionIndex][i] = {scanIndex, sectionIndex}
        std::vector<std::vector<std::vector<int>>> indexMeanMap = {
            {{2, 5}, {3, 3}},
            {{1, 2}, {2, 4}, {3, 0}},
            {{1, 5}, {2, 3}},
            {{0, 0}, {2, 2}, {3, 4}},
            {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
            {{0, 2}, {1, 4}, {2, 0}},
            {{0, 3}, {3, 5}},
            {{0, 4}, {1, 0}, {3, 2}},
            {{0, 5}, {1, 3}},
        };

        std::vector<cv::Scalar>
        scanPart()
        {
            BOOST_LOG_TRIVIAL(debug) << "Getting frame from webcam...";

            // clears out any buffers with multiple reads (hopefully)
            for (int i = 0; i < 5; i++)
            {
                camera.read(frame);
                frame.copyTo(disp);
            }

            BOOST_LOG_TRIVIAL(debug) << "Converting Colors...";
            cv::cvtColor(frame, frame, cv::COLOR_BGR2Lab);
            BOOST_LOG_TRIVIAL(debug) << "Averaging Colors...";
            std::vector<cv::Scalar> colors(sections.size());

            for (int i = 0; i < sections.size(); i++)
            {
                cv::Rect2i rect(sections[i] - sectionOffset, sections[i] + sectionOffset);
                cv::Mat submat(frame, rect);
                colors[i] = cv::mean(submat);

                cv::rectangle(disp, rect, colors[i], -1);
                cv::rectangle(disp, rect, cv::Scalar::all(0), 1);
            }

            cv::imwrite("cv-" + std::to_string(fileCounter) + ".png", disp);
            fileCounter++;

            return colors;
        }

        std::vector<cv::Scalar> scanSide()
        {
            BOOST_LOG_TRIVIAL(info) << "Scanning Side...";
            motors::moveArm(motors::ArmPosition::UP);

            std::vector<std::vector<cv::Scalar>> scannedColors(4);

            for (int i = 0; i < 4; i++)
            {
                scannedColors[i] = scanPart();
                if(i != 3) motors::rotateCube(1);
            }

            std::vector<cv::Scalar> colors(9);

            for (int i = 0; i < colors.size(); i++)
            {
                cv::Scalar color(0, 0, 0);
                std::vector<std::vector<int>> map = indexMeanMap[i];
                int items = map.size();

                for (int j = 0; j < items; j++)
                {
                    std::vector<int> indecies = map[j];
                    color += scannedColors[indecies[0]][indecies[1]];
                }

                for (int j = 0; j < 3; j++)
                {
                    color[j] /= items;
                }

                colors[i] = color;
            }

            BOOST_LOG_TRIVIAL(info) << "Done";

            std::ostringstream data;

            data << "[";
            std::copy(colors.begin(), colors.end(), std::ostream_iterator<cv::Scalar>(data, ", "));
            data << "]";

            BOOST_LOG_TRIVIAL(debug) << data.str();

            return colors;
        }

        std::vector<std::vector<int>> rotateMap = {
            {0, 1, 2,
             3, 4, 5,
             6, 7, 8},

            {6, 3, 0,
             7, 4, 1,
             8, 5, 2},

            {8, 7, 6,
             5, 4, 3,
             2, 1, 0},

            {2, 5, 8,
             1, 4, 7,
             0, 3, 6},
        };

        std::vector<cv::Scalar> rotateSide(std::vector<cv::Scalar> side, int n)
        {
            n %= 4;
            if (n < 0)
                n += 4;

            std::vector<cv::Scalar> rotated(9);

            for (int i = 0; i < rotated.size(); i++)
            {
                rotated[i] = side[rotateMap[n][i]];
            }

            return rotated;
        }
    } // namespace

    std::string scanCube()
    {
        std::vector<char> faceCharsLUT = {'U', 'R', 'F', 'D', 'L', 'B', '?'};
        std::ostringstream cubestring;
        std::vector<std::vector<cv::Scalar>> faces(6);
        // white, red, green, yellow, orange, blue

        faces[0] = scanSide();
        motors::rotateCube(1);
        motors::flipCube(1);
        faces[2] = scanSide();
        motors::rotateCube(2);
        motors::flipCube(1);
        faces[1] = rotateSide(scanSide(), 3);
        motors::rotateCube(1);
        motors::flipCube(1);
        faces[5] = rotateSide(scanSide(), 3);
        motors::rotateCube(1);
        motors::flipCube(1);
        faces[4] = rotateSide(scanSide(), 3);
        motors::flipCube(1);
        faces[3] = rotateSide(scanSide(), 3);
        motors::rotateCube(1);
        motors::flipCube(2);
        motors::rotateCube(1);

        for (int facei = 0; facei < 6; facei++)
        {
            for (int tilei = 0; tilei < 9; tilei++)
            {
                int closest = 6;

                if (tilei == 4)
                {
                    closest = facei;
                }
                else
                {
                    cv::Scalar color = faces[facei][tilei];
                    int closestDist = INT_MAX;
                    for (int faceChecki = 0; faceChecki < 6; faceChecki++)
                    {
                        int dist = 0;
                        cv::Scalar checkColor = faces[faceChecki][4]; // 4 is the center tile
                        for (int channeli = 0; channeli < 3; channeli++)
                        {
                            dist += std::abs(color[channeli] - checkColor[channeli]);
                        }

                        if (dist < closestDist)
                        {
                            closestDist = dist;
                            closest = faceChecki;
                        }
                    }
                }

                cubestring << faceCharsLUT[closest];
            }
        }

        return cubestring.str();
    }

    void init()
    {
        BOOST_LOG_TRIVIAL(info) << "Initializing Camera...";
        camera.open(0, cv::CAP_ANY);
        camera.read(frame);
    }

} // namespace detection

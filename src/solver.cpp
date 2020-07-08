#include "solver.hpp"

#include <sstream>
#include <boost/log/trivial.hpp>

#include "motors.hpp"

namespace solver
{
    namespace
    {
        enum Side
        {
            U,
            F,
            R,
            B,
            L,
            D
        };

        struct Vector3i
        {
            int x = 0;
            int y = 0;
            int z = 0;

            Vector3i(int x, int y, int z)
            {
                this->x = x;
                this->y = y;
                this->z = z;
            }

            Vector3i(Side side)
            {
                if (side == Side::U)
                    y = 1;
                else if (side == Side::F)
                    z = -1;
                else if (side == Side::R)
                    x = 1;
                else if (side == Side::B)
                    z = 1;
                else if (side == Side::L)
                    x = -1;
                else if (side == Side::D)
                    y = -1;
            }

            int dot(Vector3i other)
            {
                return x * other.x + y * other.y + z * other.z;
            }

            Vector3i cross(Vector3i o)
            {
                return Vector3i(
                    y * o.z - z * o.y,
                    z * o.x - x * o.z,
                    x * o.y - y * o.x);
            }

            Vector3i operator-()
            {
                return Vector3i(-x, -y, -z);
            }

            bool operator==(Vector3i o)
            {
                return x == o.x && y == o.y && z == o.z;
            }
        };

        struct Orientation
        {
            Vector3i up = Side::U;
            Vector3i front = Side::F;

            // TODO: Verify math
            void flipToSide(Side _side)
            {
                Vector3i side(_side);
                if (up.dot(side) == 1) // Flip cube 180°
                {
                    motors::flipCube(2);
                    up = -up;
                    front = -front;
                }
                else if (up.dot(side) == 0) // Flip cube 90°
                {
                    int trippleProduct = up.dot(front.cross(side));
                    if (front == -side)
                    {
                        // We're already on the right side
                    }
                    else if (front == side) // Rotate cube 180°
                    {
                        motors::rotateCube(2);
                        front = -front;
                    }
                    else if (trippleProduct == -1) // Rotate cube -90°
                    {
                        motors::rotateCube(-1);
                        front = up.cross(front);
                    }
                    else // Rotate cube +90°
                    {
                        motors::rotateCube(1);
                        front = front.cross(up);
                    }

                    motors::flipCube(1);
                    front = -up;
                    up = -side;
                }
            }
        }; // namespace orientation

        struct Move
        {
            int amount;
            Side side;

            Move(std::string string)
            {
                if (string.length() != 1 && string.length() != 2)
                    throw "String is wrong length \"" + string + "\"";

                char _side = string[0];
                char _amount = string.length() == 1 ? '1' : string[1];

                if (_side == 'U')
                    side = Side::U;
                else if (_side == 'F')
                    side = Side::F;
                else if (_side == 'R')
                    side = Side::R;
                else if (_side == 'B')
                    side = Side::B;
                else if (_side == 'L')
                    side = Side::L;
                else if (_side == 'D')
                    side = Side::D;
                else
                    throw "Unknown side " + std::to_string(_side);

                if (_amount == '1')
                    amount = 1;
                else if (_amount == '\'')
                    amount = -1;
                else if (_amount == '2')
                    amount = 2;
                else
                    throw "Unknown amount " + std::to_string(_amount);
            }
        };
    } // namespace

    void runSolver(std::string _cubestring)
    {
        BOOST_LOG_TRIVIAL(info) << "Solving Cube...";
        if (_cubestring.length() == 0)
            return;

        std::stringstream cubestring(_cubestring);
        std::string _move;

        Orientation orientation;
        while (std::getline(cubestring, _move, ' '))
        {
            BOOST_LOG_TRIVIAL(debug) << _move;
            Move move(_move);
            orientation.flipToSide(move.side);
            motors::rotateFace(-move.amount);
        }
    }
} // namespace solver

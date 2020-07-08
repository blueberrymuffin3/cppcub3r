#pragma once

namespace motors
{
    enum ArmPosition
    {
        UP,
        DOWN,
        SETTLE,
        FLIP
    };

    void init();
    void cleanExit();
    void turnTable(int turns, bool doOvershoot);
    void moveArm(ArmPosition position);
    void rotateCube(int turns);
    void rotateFace(int turns);
    void flipCube(int flips);
}; // namespace motors

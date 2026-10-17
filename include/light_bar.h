#pragma once

#include <opencv2/core.hpp>

struct LightBar
{
    cv::RotatedRect rect;
    double length;
    double width;
    double angle;
};
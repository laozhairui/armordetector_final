
#pragma once

#include <opencv2/core.hpp>

#include <cstddef>

struct ArmorPair
{
    std::size_t left_index = 0;
    std::size_t right_index = 0;
    cv::RotatedRect rect;
    double score = 0.0;
};


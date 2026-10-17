#pragma once

#include <light_bar.h>

#include <opencv2/core.hpp>

#include <vector>

class LightBarDetector
{
public:
    std::vector<LightBar> detect(
        const std::vector<std::vector<cv::Point>>& contours
    );
};

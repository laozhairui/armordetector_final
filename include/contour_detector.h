#pragma once
#include <opencv2/core.hpp>
#include <vector>

class ContourDetector
{
public:
    std::vector<std::vector<cv::Point>> detect(
        const cv::Mat& mask
    );
};
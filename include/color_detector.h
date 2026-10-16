#pragma once

#include <opencv2/core.hpp>

#include <string>

class ColorDetector
{
public:
    ColorDetector();

    cv::Mat detect(const cv::Mat& frame, const std::string& color);
    
};
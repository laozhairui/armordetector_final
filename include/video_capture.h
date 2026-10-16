#pragma once

#include <opencv2/opencv.hpp>

#include <string>

class VideoCapture
{
public:
    explicit VideoCapture(const std::string& source);

    bool is_opened() const;

    bool read_frame(cv::Mat& frame);

private:
    cv::VideoCapture m_capture;
};
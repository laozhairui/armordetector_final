#include <contour_detector.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

std::vector<std::vector<cv::Point>> ContourDetector::detect(
    const cv::Mat& mask
)
{
    std::vector<std::vector<cv::Point>> contours;
    if (mask.empty())
    {
        return contours;
    }

    cv::Mat used_mask = mask.clone();
    cv::findContours(
        used_mask,
        contours,
        cv::RETR_EXTERNAL,
        cv::CHAIN_APPROX_SIMPLE
    );

    return contours;
}
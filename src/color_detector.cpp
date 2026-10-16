#include <color_detector.h>

#include <opencv2/imgproc.hpp>

ColorDetector::ColorDetector()
{
}

cv::Mat ColorDetector::detect(
    const cv::Mat& frame,
    const std::string& color
)
{
    cv::Mat hsv;
    cv::Mat mask;

    cv::cvtColor(
        frame,
        hsv,
        cv::COLOR_BGR2HSV
    );

    if (color == "red")
    {
        cv::Mat mask1;
        cv::Mat mask2;

        cv::inRange(
            hsv,
            cv::Scalar(0, 65, 45),
            cv::Scalar(12, 255, 255),
            mask1
        );

        cv::inRange(
            hsv,
            cv::Scalar(165, 65, 45),
            cv::Scalar(179, 255, 255),
            mask2
        );

        cv::bitwise_or(
            mask1,
            mask2,
            mask
        );
    }
    else
    {
        cv::inRange(
            hsv,
            cv::Scalar(88, 65, 45),
            cv::Scalar(138, 255, 255),
            mask
        );
    }

    cv::Mat close_kernel =
        cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(3, 3)
        );

    cv::morphologyEx(
        mask,
        mask,
        cv::MORPH_CLOSE,
        close_kernel
    );

    return mask;
}
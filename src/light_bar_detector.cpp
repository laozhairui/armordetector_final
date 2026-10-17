
#include <light_bar_detector.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include <opencv2/imgproc.hpp>

namespace
{
    constexpr double kMinContourArea = 3.0;
    constexpr double kMaxContourArea = 1000.0;
    constexpr double kMinLightLength = 5.0;
    constexpr double kMinLightWidth = 1.0;
    constexpr double kMinAspectRatio = 1.5;
    constexpr double kMaxAspectRatio = 8.0;
    constexpr double kMinFillRatio = 0.22;
    constexpr double kMinSolidity = 0.50;
    constexpr double kMaxVerticalDeviation = 38.0;

    double normalize_angle(double angle)
    {
        while (angle < 0.0)
        {
            angle += 180.0;
        }

        while (angle >= 180.0)
        {
            angle -= 180.0;
        }

        return angle;
    }

    double get_long_axis_angle(const cv::RotatedRect& rect)
    {
        double angle = rect.angle;

        if (rect.size.width < rect.size.height)
        {
            angle += 90.0;
        }

        return normalize_angle(angle);
    }

    double get_vertical_deviation(double angle)
    {
        angle = normalize_angle(angle);

        return std::abs(angle - 90.0);
    }

    bool is_valid_light_bar(
        const std::vector<cv::Point>& contour,
        LightBar& detected_light_bar
    )
    {
        if (contour.size() < 5)
        {
            return false;
        }

        double contour_area = cv::contourArea(contour);

        if (contour_area < kMinContourArea ||
            contour_area > kMaxContourArea)
        {
            return false;
        }

        cv::RotatedRect rect = cv::minAreaRect(contour);

        double length = std::max(
            rect.size.width,
            rect.size.height
        );

        double width = std::min(
            rect.size.width,
            rect.size.height
        );

        if (length < kMinLightLength ||
            width < kMinLightWidth)
        {
            return false;
        }

        double aspect_ratio = length / width;

        if (aspect_ratio < kMinAspectRatio ||
            aspect_ratio > kMaxAspectRatio)
        {
            return false;
        }

        double rect_area = rect.size.width * rect.size.height;

        if (rect_area <= 0.0)
        {
            return false;
        }

        double fill_ratio = contour_area / rect_area;

        if (fill_ratio < kMinFillRatio)
        {
            return false;
        }

        std::vector<cv::Point> hull;
        cv::convexHull(contour, hull);

        double hull_area = cv::contourArea(hull);

        if (hull_area <= 0.0)
        {
            return false;
        }

        double solidity = contour_area / hull_area;

        if (solidity < kMinSolidity)
        {
            return false;
        }

        double angle = get_long_axis_angle(rect);

        if (get_vertical_deviation(angle) >
            kMaxVerticalDeviation)
        {
            return false;
        }

        detected_light_bar.rect = rect;
        detected_light_bar.length = length;
        detected_light_bar.width = width;
        detected_light_bar.angle = angle;

        return true;
    }
}

std::vector<LightBar> LightBarDetector::detect(
    const std::vector<std::vector<cv::Point>>& contours
)
{
    std::vector<LightBar> light_bar;
    light_bar.reserve(contours.size());

    for (const auto& contour : contours)
    {
        LightBar detected_light_bar{};

        if (is_valid_light_bar(contour, detected_light_bar))
        {
            light_bar.push_back(detected_light_bar);
        }
    }

    std::sort(
        light_bar.begin(),
        light_bar.end(),
        [](const LightBar& left, const LightBar& right)
        {
            return left.rect.center.x < right.rect.center.x;
        }
    );

    return light_bar;
}

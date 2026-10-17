#include <armor_pair_detector.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace
{
    constexpr double kMinLengthRatio = 0.50;
    constexpr double kMinWidthRatio = 0.40;
    constexpr double kMaxAngleDifference = 22.0;
    constexpr double kMinHorizontalGapRatio = 0.65;
    constexpr double kMaxHorizontalGapRatio = 4.2;
    constexpr double kMinVerticalGap = 5.0;
    constexpr double kVerticalGapFactor = 0.70;
    constexpr double kMinAbsoluteDistance = 4.0;
    constexpr double kMinMaxDistance = 20.0;
    constexpr double kMaxCenterDistanceFactor = 5.0;
    constexpr double kMinAdaptiveGap = 18.0;
    constexpr double kAdaptiveGapFactor = 4.0;
    constexpr double kMaxAbsoluteGap = 125.0;
    constexpr double kBlockingInterpolationMin = 0.10;
    constexpr double kBlockingInterpolationMax = 0.90;

    struct PairCandidate
    {
        ArmorPair pair;
        double score = 0.0;
    };

    double angle_difference(
        double first,
        double second
    )
    {
        double difference = std::abs(first - second);

        while (difference >= 180.0)
        {
            difference -= 180.0;
        }

        return std::min(difference, 180.0 - difference);
    }

    bool has_blocking_bar(
        const std::vector<LightBar>& light_bar,
        std::size_t left_index,
        std::size_t right_index
    )
    {
        const LightBar& left = light_bar[left_index];
        const LightBar& right = light_bar[right_index];

        double dx = right.rect.center.x - left.rect.center.x;

        if (dx <= 0.0)
        {
            return true;
        }

        for (std::size_t i = 0; i < light_bar.size(); ++i)
        {
            if (i == left_index || i == right_index)
            {
                continue;
            }

            const LightBar& middle = light_bar[i];

            double t =
                (middle.rect.center.x - left.rect.center.x) / dx;

            if (t <= kBlockingInterpolationMin ||
                t >= kBlockingInterpolationMax)
            {
                continue;
            }

            double expected_y =
                left.rect.center.y +
                t * (right.rect.center.y - left.rect.center.y);

            double y_distance =
                std::abs(middle.rect.center.y - expected_y);

            double allowed_distance = std::max(
                5.0,
                std::min(left.length, right.length) * 0.50
            );

            if (y_distance <= allowed_distance)
            {
                return true;
            }
        }

        return false;
    }

    bool build_pair_candidate(
        const std::vector<LightBar>& light_bar,
        std::size_t left_index,
        std::size_t right_index,
        PairCandidate& candidate
    )
    {
        const LightBar& left = light_bar[left_index];
        const LightBar& right = light_bar[right_index];

        if (left.rect.center.x >= right.rect.center.x)
        {
            return false;
        }

        double average_length = (left.length + right.length) * 0.5;
        double average_width = (left.width + right.width) * 0.5;

        if (average_length <= 0.0 || average_width <= 0.0)
        {
            return false;
        }

        double length_ratio =
            std::min(left.length, right.length) /
            std::max(left.length, right.length);

        if (length_ratio < kMinLengthRatio)
        {
            return false;
        }

        double width_ratio =
            std::min(left.width, right.width) /
            std::max(left.width, right.width);

        if (width_ratio < kMinWidthRatio)
        {
            return false;
        }

        double angle_diff = angle_difference(
            left.angle,
            right.angle
        );

        if (angle_diff > kMaxAngleDifference)
        {
            return false;
        }

        double dx = right.rect.center.x - left.rect.center.x;
        double dy = right.rect.center.y - left.rect.center.y;

        double horizontal_gap_ratio = dx / average_length;

        if (horizontal_gap_ratio < kMinHorizontalGapRatio ||
            horizontal_gap_ratio > kMaxHorizontalGapRatio)
        {
            return false;
        }

        double adaptive_gap_limit = std::min(
            kMaxAbsoluteGap,
            std::max(
                kMinAdaptiveGap,
                average_length * kAdaptiveGapFactor
            )
        );

        if (dx > adaptive_gap_limit)
        {
            return false;
        }

        double vertical_difference = std::abs(dy);
        double allowed_vertical_difference = std::max(
            kMinVerticalGap,
            average_length * kVerticalGapFactor
        );

        if (vertical_difference > allowed_vertical_difference)
        {
            return false;
        }

        double center_distance = std::sqrt(dx * dx + dy * dy);

        double min_center_distance = std::max(
            kMinAbsoluteDistance,
            average_length * 0.55
        );

        double max_center_distance = std::max(
            kMinMaxDistance,
            average_length * kMaxCenterDistanceFactor
        );

        if (center_distance < min_center_distance ||
            center_distance > max_center_distance)
        {
            return false;
        }

        double center_line_angle =
            std::atan2(dy, dx) * 180.0 / CV_PI;

        double max_center_line_angle = std::min(
            30.0,
            18.0 + 20.0 / std::max(average_length, 6.0)
        );

        if (std::abs(center_line_angle) > max_center_line_angle)
        {
            return false;
        }

        if (has_blocking_bar(light_bar, left_index, right_index))
        {
            return false;
        }

        double normalized_gap = center_distance / average_length;
        double vertical_penalty = vertical_difference / average_length;
        double length_penalty = 1.0 - length_ratio;
        double width_penalty = 1.0 - width_ratio;
        double angle_penalty = angle_diff / kMaxAngleDifference;
        double gap_penalty = std::abs(normalized_gap - 2.2) / 3.0;
        double center_angle_penalty =
            std::abs(center_line_angle) / 30.0;

        double score =
            angle_penalty * 24.0 +
            vertical_penalty * 22.0 +
            length_penalty * 24.0 +
            width_penalty * 14.0 +
            gap_penalty * 10.0 +
            center_angle_penalty * 8.0;

        ArmorPair pair;
        pair.left_index = left_index;
        pair.right_index = right_index;
        pair.score = score;
        pair.rect = cv::RotatedRect(
            cv::Point2f(
                (left.rect.center.x + right.rect.center.x) * 0.5F,
                (left.rect.center.y + right.rect.center.y) * 0.5F
            ),
            cv::Size2f(
                static_cast<float>(center_distance),
                static_cast<float>(average_length)
            ),
            static_cast<float>(center_line_angle)
        );

        candidate.pair = pair;
        candidate.score = score;

        return true;
    }
}

std::vector<ArmorPair> ArmorPairDetector::detect(
    const std::vector<LightBar>& light_bar
) const
{
    std::vector<PairCandidate> pair_candidate;

    for (std::size_t i = 0; i < light_bar.size(); ++i)
    {
        for (std::size_t j = i + 1; j < light_bar.size(); ++j)
        {
            PairCandidate candidate;

            if (build_pair_candidate(light_bar, i, j, candidate))
            {
                pair_candidate.push_back(candidate);
            }
        }
    }

    std::sort(
        pair_candidate.begin(),
        pair_candidate.end(),
        [](const PairCandidate& first, const PairCandidate& second)
        {
            return first.score < second.score;
        }
    );

    std::vector<bool> used_light_bar(light_bar.size(), false);
    std::vector<ArmorPair> armor_pair;

    for (const auto& candidate : pair_candidate)
    {
        const ArmorPair& pair = candidate.pair;

        if (used_light_bar[pair.left_index] ||
            used_light_bar[pair.right_index])
        {
            continue;
        }

        armor_pair.push_back(pair);
        used_light_bar[pair.left_index] = true;
        used_light_bar[pair.right_index] = true;
    }

    return armor_pair;
}

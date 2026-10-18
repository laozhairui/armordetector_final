
#include <armor_pair_detector.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
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

    constexpr double kMinTrackDistance = 35.0;
    constexpr double kTrackDistanceFactor = 1.5;
    constexpr int kMaxTrackMissedFrames = 3;
    constexpr int kMaxPairMissedFrames = 3;

    struct PairCandidate
    {
        ArmorPair pair;
        double score = 0.0;
    };

    struct BarMatch
    {
        std::size_t previous_index = 0;
        std::size_t current_index = 0;
        double cost = 0.0;
    };

    double angle_difference(double first, double second)
    {
        double difference = std::abs(first - second);

        while (difference >= 180.0)
        {
            difference -= 180.0;
        }

        return std::min(difference, 180.0 - difference);
    }

    double light_bar_match_cost(
        const LightBar& previous,
        const LightBar& current
    )
    {
        double dx =
            current.rect.center.x - previous.rect.center.x;
        double dy =
            current.rect.center.y - previous.rect.center.y;

        double distance = std::sqrt(dx * dx + dy * dy);

        double distance_limit = std::max(
            kMinTrackDistance,
            previous.length * kTrackDistanceFactor
        );

        if (distance > distance_limit)
        {
            return -1.0;
        }

        double length_scale = std::max(previous.length, 1.0);
        double width_scale = std::max(previous.width, 1.0);

        double length_difference =
            std::abs(current.length - previous.length) /
            length_scale;

        double width_difference =
            std::abs(current.width - previous.width) /
            width_scale;

        double angle_diff = angle_difference(
            previous.angle,
            current.angle
        );

        return distance / distance_limit +
            length_difference * 0.5 +
            width_difference * 0.3 +
            angle_diff / 90.0;
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

        double angle_diff = angle_difference(left.angle, right.angle);

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
)
{
    std::vector<ArmorPair> armor_pair;

    std::vector<std::size_t> current_id(light_bar.size(), 0);
    std::vector<bool> used_current(light_bar.size(), false);
    std::vector<bool> used_previous(m_tracked_light_bar.size(), false);

    std::vector<BarMatch> bar_match;

    for (std::size_t previous_index = 0;
         previous_index < m_tracked_light_bar.size();
         ++previous_index)
    {
        for (std::size_t current_index = 0;
             current_index < light_bar.size();
             ++current_index)
        {
            double cost = light_bar_match_cost(
                m_tracked_light_bar[previous_index].light_bar,
                light_bar[current_index]
            );

            if (cost < 0.0)
            {
                continue;
            }

            bar_match.push_back({
                previous_index,
                current_index,
                cost
            });
        }
    }

    std::sort(
        bar_match.begin(),
        bar_match.end(),
        [](const BarMatch& first, const BarMatch& second)
        {
            return first.cost < second.cost;
        }
    );

    for (const BarMatch& match : bar_match)
    {
        if (used_previous[match.previous_index] ||
            used_current[match.current_index])
        {
            continue;
        }

        used_previous[match.previous_index] = true;
        used_current[match.current_index] = true;

        current_id[match.current_index] =
            m_tracked_light_bar[match.previous_index].id;
    }

    std::vector<TrackedLightBar> next_tracked_light_bar;

    for (std::size_t i = 0; i < m_tracked_light_bar.size(); ++i)
    {
        if (!used_previous[i])
        {
            TrackedLightBar missed = m_tracked_light_bar[i];
            ++missed.missed_frames;

            if (missed.missed_frames <= kMaxTrackMissedFrames)
            {
                next_tracked_light_bar.push_back(missed);
            }
        }
    }

    for (std::size_t i = 0; i < light_bar.size(); ++i)
    {
        if (current_id[i] == 0)
        {
            current_id[i] = m_next_id++;
        }

        TrackedLightBar tracked;
        tracked.light_bar = light_bar[i];
        tracked.id = current_id[i];
        tracked.missed_frames = 0;

        next_tracked_light_bar.push_back(tracked);
    }

    m_tracked_light_bar = std::move(next_tracked_light_bar);

    std::vector<PairCandidate> pair_candidate;

    for (std::size_t i = 0; i < light_bar.size(); ++i)
    {
        for (std::size_t j = i + 1; j < light_bar.size(); ++j)
        {
            std::size_t left_index = i;
            std::size_t right_index = j;

            if (light_bar[left_index].rect.center.x >
                light_bar[right_index].rect.center.x)
            {
                std::swap(left_index, right_index);
            }

            PairCandidate candidate;

            if (build_pair_candidate(
                    light_bar,
                    left_index,
                    right_index,
                    candidate))
            {
                pair_candidate.push_back(candidate);
            }
        }
    }

    std::vector<bool> used_candidate_bar(light_bar.size(), false);
    std::vector<LockedPair> next_locked_pair;

    for (LockedPair locked : m_locked_pair)
    {
        std::size_t matched_candidate = pair_candidate.size();

        for (std::size_t i = 0; i < pair_candidate.size(); ++i)
        {
            const ArmorPair& pair = pair_candidate[i].pair;

            std::size_t left_id = current_id[pair.left_index];
            std::size_t right_id = current_id[pair.right_index];

            bool same_pair =
                (left_id == locked.left_id &&
                 right_id == locked.right_id) ||
                (left_id == locked.right_id &&
                 right_id == locked.left_id);

            if (same_pair)
            {
                matched_candidate = i;
                break;
            }
        }

        if (matched_candidate != pair_candidate.size())
        {
            const ArmorPair& pair =
                pair_candidate[matched_candidate].pair;

            if (!used_candidate_bar[pair.left_index] &&
                !used_candidate_bar[pair.right_index])
            {
                armor_pair.push_back(pair);

                used_candidate_bar[pair.left_index] = true;
                used_candidate_bar[pair.right_index] = true;

                locked.missed_frames = 0;
                next_locked_pair.push_back(locked);
                continue;
            }
        }

        ++locked.missed_frames;

        if (locked.missed_frames <= kMaxPairMissedFrames)
        {
            next_locked_pair.push_back(locked);

            for (std::size_t i = 0; i < light_bar.size(); ++i)
            {
                if (current_id[i] == locked.left_id ||
                    current_id[i] == locked.right_id)
                {
                    used_candidate_bar[i] = true;
                }
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

    for (const PairCandidate& candidate : pair_candidate)
    {
        const ArmorPair& pair = candidate.pair;

        if (used_candidate_bar[pair.left_index] ||
            used_candidate_bar[pair.right_index])
        {
            continue;
        }

        armor_pair.push_back(pair);

        used_candidate_bar[pair.left_index] = true;
        used_candidate_bar[pair.right_index] = true;

        LockedPair locked;
        locked.left_id = current_id[pair.left_index];
        locked.right_id = current_id[pair.right_index];
        locked.missed_frames = 0;

        next_locked_pair.push_back(locked);
    }

    m_locked_pair = std::move(next_locked_pair);

    return armor_pair;
}

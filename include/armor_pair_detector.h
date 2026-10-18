#pragma once

#include <armor_pair.h>
#include <light_bar.h>

#include <cstddef>
#include <vector>

class ArmorPairDetector
{
public:
    std::vector<ArmorPair> detect(
        const std::vector<LightBar>& light_bar
    );

private:
    struct TrackedLightBar
    {
        LightBar light_bar;
        std::size_t id = 0;
        int missed_frames = 0;
    };

    struct LockedPair
    {
        std::size_t left_id = 0;
        std::size_t right_id = 0;
        int missed_frames = 0;
    };

    std::vector<TrackedLightBar> m_tracked_light_bar;
    std::vector<LockedPair> m_locked_pair;
    std::size_t m_next_id = 1;
};

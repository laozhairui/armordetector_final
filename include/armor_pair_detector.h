#pragma once

#include <armor_pair.h>
#include <light_bar.h>

#include <vector>

class ArmorPairDetector
{
public:
    std::vector<ArmorPair> detect(
        const std::vector<LightBar>& light_bar
    ) const;
};

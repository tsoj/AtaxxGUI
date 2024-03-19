#pragma once

#include <../core/engine/settings.hpp>
#include <vector>

struct GuiSettings {
    GuiSettings(const std::string &path);

    std::vector<EngineSettings> engines;
    SearchSettings tc;
};

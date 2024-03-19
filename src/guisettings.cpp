#include "guisettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

GuiSettings::GuiSettings(const std::string &path) {
    nlohmann::ordered_json json;

    // Get settings file
    {
        std::ifstream i(path);
        if (!i.is_open()) {
            throw std::invalid_argument("Could not open settings file " + path);
        }
        try {
            i >> json;
        } catch (nlohmann::json::exception &e) {
            throw e;
        } catch (...) {
            throw std::logic_error("Failure parsing .json");
        }
    }

    auto engine_iter = json.find("engines");

    // Default
    this->tc = SearchSettings::as_time(30000, 30000, 1000, 1000);

    std::vector<std::pair<std::string, std::string>> engine_options;

    for (const auto &[a, b] : json.items()) {
        if (a == "timecontrol") {
            for (const auto &[key, val] : b.items()) {
                if (key == "movetime") {
                    this->tc.type = SearchSettings::Type::Movetime;
                    this->tc.movetime = val.get<int>();
                } else if (key == "nodes") {
                    this->tc.type = SearchSettings::Type::Nodes;
                    this->tc.nodes = val.get<int>();
                } else if (key == "time") {
                    this->tc.type = SearchSettings::Type::Time;
                    this->tc.btime = val.get<int>();
                    this->tc.wtime = val.get<int>();
                } else if (key == "increment" || key == "inc") {
                    this->tc.type = SearchSettings::Type::Time;
                    this->tc.binc = val.get<int>();
                    this->tc.winc = val.get<int>();
                } else if (key == "depth") {
                    this->tc.type = SearchSettings::Type::Depth;
                    this->tc.ply = val.get<int>();
                }
            }
        } else if (a == "options") {
            for (const auto &[key, val] : b.items()) {
                engine_options.emplace_back(key, val);
            }
        }
    }

    for (const auto &engine : *engine_iter) {
        EngineSettings details;

        details.id = this->engines.size();
        details.options = engine_options;
        details.tc = this->tc;

        for (const auto &[a, b] : engine.items()) {
            if (a == "path") {
                details.path = b.get<std::string>();
            } else if (a == "protocol") {
                const auto proto = b.get<std::string>();
                if (proto == "UAI" || proto == "uai") {
                    details.proto = EngineProtocol::UAI;
                } else if (proto == "FSF" || proto == "fsf") {
                    details.proto = EngineProtocol::FSF;
                } else if (proto == "KATAGO" || proto == "KataGo" || proto == "katago") {
                    details.proto = EngineProtocol::KataGo;
                }
            } else if (a == "name") {
                details.name = b.get<std::string>();
            } else if (a == "builtin") {
                details.builtin = b.get<std::string>();
            } else if (a == "arguments") {
                details.arguments = b.get<std::string>();
            } else if (a == "options") {
                for (const auto &[key, val] : b.items()) {
                    const auto iter =
                        std::find_if(details.options.begin(), details.options.end(), [key](const auto &obj) -> bool {
                            return obj.first == key;
                        });

                    // override
                    if (iter != details.options.end()) {
                        details.options.erase(iter);
                    }

                    details.options.emplace_back(key, val.get<std::string>());
                }
            } else if (a == "timecontrol") {
                for (const auto &[key, val] : b.items()) {
                    if (key == "movetime") {
                        details.tc.type = SearchSettings::Type::Movetime;
                        details.tc.movetime = val.get<int>();
                    } else if (key == "nodes") {
                        details.tc.type = SearchSettings::Type::Nodes;
                        details.tc.nodes = val.get<int>();
                    } else if (key == "time") {
                        details.tc.type = SearchSettings::Type::Time;
                        details.tc.btime = val.get<int>();
                        details.tc.wtime = val.get<int>();
                    } else if (key == "increment" || key == "inc") {
                        details.tc.type = SearchSettings::Type::Time;
                        details.tc.binc = val.get<int>();
                        details.tc.winc = val.get<int>();
                    } else if (key == "depth") {
                        details.tc.type = SearchSettings::Type::Depth;
                        details.tc.ply = val.get<int>();
                    }
                }
            }
        }

        // Use engine path as a backup name
        if (details.name.empty()) {
            details.name = details.path;
        }

        this->engines.emplace_back(details);
    }

    // Sanity checks
    for (const auto &engine : this->engines) {
        if (!engine.builtin.empty()) {
            continue;
        }

        if (engine.proto == EngineProtocol::Unknown) {
            throw std::runtime_error("Unrecognised engine protocol");
        }
    }
}
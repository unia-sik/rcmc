#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <vector>

namespace vcd {
    /**
    * @brief Parsed content of the VCD file
    *
    */
    struct content {
        /**
        * @brief [vcd-key] --> [signal-name]
        *
        */
        std::unordered_map<std::string, std::string> signal_names;

        /**
        * @brief [vcd-key] --> [signal-scope]
        *
        */
        std::unordered_map<std::string, std::string> signal_scope;

        /**
        * @brief [cycle] --> [{vcd-key, newValue}]
        *
        */
        std::map<uint64_t, std::vector<std::pair<std::string, std::string>>> signal_values;

        /**
        * @brief Scope structure formated as json
        *
        */
        std::string json_tree;

        /**
        * @brief All comments from the vcd-header
        *
        */
        std::vector<std::string> comments;
        
        uint64_t max_cycle;
    };
}

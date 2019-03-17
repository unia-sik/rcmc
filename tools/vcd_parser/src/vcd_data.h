#pragma once

#include "vcd_content.h"

namespace vcd {
    namespace data {
        void parse_vector(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle);
        void parse_single(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle);
        void parse(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle);
    }
}

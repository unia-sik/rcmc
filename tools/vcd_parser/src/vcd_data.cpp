#include "vcd_data.h"

#include <string>
#include <fstream>
#include "vcd_header.h"

void vcd::data::parse_vector(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle) {
    int index = line.find_first_not_of("01UX", 1);

    if (index == std::string::npos) {
        index = line.length() - 1;
    } else {
        if (line[index] == ' ') {
            index++;
        }
    }

    std::string key = line.substr(index);
    std::string value = line.substr(0, index - 1);

    if (min_cycle <= time && time <= max_cycle) {
        if (scopeFilter == "" || result.signal_scope[key].find(scopeFilter) != std::string::npos) {
            if (signalFilter == "" || result.signal_names[key].find(signalFilter) != std::string::npos) {
                if (valueFilter == "" || value.find(valueFilter) != std::string::npos) {
                    result.signal_values[time].push_back(std::make_pair(key, value));
                }
            }
        }
    }
}

void vcd::data::parse_single(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle) {
    int index = 1;
    
    if (line[index] == ' ') {
        index++;
    }    
    
    std::string key = line.substr(index);
    std::string value = line.substr(0, index);

    if (min_cycle <= time && time <= max_cycle) {
        if (scopeFilter == "" || result.signal_scope[key].find(scopeFilter) != std::string::npos) {
            if (signalFilter == "" || result.signal_names[key].find(signalFilter) != std::string::npos) {
                if (valueFilter == "" || value.find(valueFilter) != std::string::npos) {
                    result.signal_values[time].push_back(std::make_pair(key, value));
                }
            }
        }
    }
}

void vcd::data::parse(std::string& line, vcd::content& result, uint64_t& time, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle) {
    if (line[0] == '#') {
        time = std::stoll(line.substr(1)) / 1000000;
    } else if (line[0] == 'b') {
        parse_vector(line, result, time, scopeFilter, signalFilter, valueFilter, min_cycle, max_cycle);
    } else {
        parse_single(line, result, time, scopeFilter, signalFilter, valueFilter, min_cycle, max_cycle);
    }
}



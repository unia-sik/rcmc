#pragma once
#include <string>
#include <vector>

#include "vcd_content.h"

namespace vcd {
    namespace header {
        std::vector<std::string> split_var_line(std::string& line);
        void parse_var(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack);
        void parse_upscope(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack);
        void parse_scope(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack);
        void parse_comment(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack);
        bool parse(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack);
    }
}

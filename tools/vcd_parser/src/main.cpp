#include <iostream>
#include <fstream>
#include <cmath>

#include "vcd_content.h"
#include "vcd_header.h"
#include "vcd_data.h"


vcd::content parse_vcd(std::fstream& file, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle) {
    vcd::content result;
    std::vector<std::string> scope_stack;

    bool header = true;
    uint64_t time;
    std::string line;

    while (getline(file, line)) {
        if (header) {
            header = vcd::header::parse(line, result, scope_stack);
        } else {
            vcd::data::parse(line, result, time, scopeFilter, signalFilter, valueFilter, min_cycle, max_cycle);
        }
    }

    result.max_cycle = time;
    
    return result;
}

int calc_dimension(vcd::content& cnt) {
    int num_core = 0;

    for (auto& value : cnt.signal_values[0]) {
        if (cnt.signal_scope[value.first].find("reg1.reg1") != std::string::npos && cnt.signal_names[value.first].find("reg0") != std::string::npos) {
            num_core++;
        }

    }

    return std::sqrt(num_core);
}

void build_log(vcd::content& cnt) {
    for (auto& entry : cnt.signal_values) {
        for (auto& value : entry.second) {
            std::cout << entry.first << ": " << cnt.signal_scope[value.first] << cnt.signal_names[value.first] << " <= " << value.second;
            
            std::cout << "'" << value.first << "'\n";
            
        }
    }
}

void build_json(vcd::content& cnt) {
    std::cout << "{\n\"name\": \"root\",\n\"children\":\n[\n";
    std::cout << cnt.json_tree << "\n";
    std::cout << "]}\n";
}

void build_regdump(vcd::content& cnt) {
    int core_dimension = calc_dimension(cnt);

    int last = 2;
    for (auto& entry : cnt.signal_values) {
        if (entry.first >= 3) {
            if (last + 1 != entry.first) {
                for (int i = last + 1; i < entry.first; i++) {
                    std::cout << "\n#" << i - 3 << "\n";
                }
            }
            
            last = entry.first;
            std::cout << "\n#" << entry.first - 3 << "\n";

            for (auto& value : entry.second) {
                unsigned long long conv = 0;
                unsigned long long core_id = 0;

                try {
                    conv = std::stoull(value.second.substr(1), nullptr, 2);
                    core_id = std::stoull(cnt.signal_scope[value.first].substr(1, 1)) * core_dimension + std::stoull(cnt.signal_scope[value.first].substr(5, 1));
                } catch (std::invalid_argument e) {
                } catch (std::out_of_range  e) {
                }

                std::cout << "°" << core_id << " ";
                std::cout << "x" << cnt.signal_names[value.first].substr(3,  cnt.signal_names[value.first].length() - 9) << " ";
                std::cout << std::hex << conv << std::dec << "\n";
            }
        }
    }
    
    for (int i = last + 1; i < cnt.max_cycle + 1; i++) {
        std::cout << "\n#" << i - 3 << "\n";
    }
}

void build_comment(vcd::content& cnt) {
    for (auto& comment : cnt.comments) {
        std::cout << comment << "\n";
    }
}

bool process_file(std::string format, std::string& scopeFilter, std::string& signalFilter, std::string& valueFilter, uint64_t min_cycle, uint64_t max_cycle, std::string filename) {
    std::fstream file;
    file.open(filename);

    if (file.is_open()) {
        vcd::content result = parse_vcd(file, scopeFilter, signalFilter, valueFilter, min_cycle, max_cycle);

        if (format == "log") {
            build_log(result);
            return true;
        } else if (format == "json") {
            build_json(result);
            return true;
        } else if (format == "regdump") {
            build_regdump(result);
            return true;
        } else if (format == "comment") {
            build_comment(result);
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[]) {
    std::string format = "";
    std::string scopeFilter = "";
    std::string signalFilter = "";
    std::string valueFilter = "";
    std::string filename = "";
    uint64_t min_cycle = 0;
    uint64_t max_cycle = UINT64_MAX;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);

        if (arg.find("--format") == 0) {
            format = arg.substr(arg.find("=") + 1);
        } else if (arg.find("--scope") == 0) {
            scopeFilter = arg.substr(arg.find("=") + 1);
        } else if (arg.find("--signal") == 0) {
            signalFilter = arg.substr(arg.find("=") + 1);
        } else if (arg.find("--value") == 0) {
            valueFilter = arg.substr(arg.find("=") + 1);
        } else if (arg.find("--min-cycle") == 0) {
            min_cycle = std::stoull(arg.substr(arg.find("=") + 1));
        } else if (arg.find("--max-cycle") == 0) {
            max_cycle = std::stoull(arg.substr(arg.find("=") + 1));
        } else {
            filename = arg;
        }
    }

    if (!process_file(format, scopeFilter, signalFilter, valueFilter, min_cycle, max_cycle, filename)) {
        std::cout << "Usage: " << argv[0] << " --format=log|regdump|json|comment [--scope=<scopeFilter> --signal=<signalFilter> -- value=<valueFilter>] <file.vcd>" << std::endl;
        return -1;
    }

    return 0;
}


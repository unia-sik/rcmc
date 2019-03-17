#include "vcd_header.h"

std::vector<std::string> vcd::header::split_var_line(std::string& line) {
    std::vector<std::string> result;
    int index = 0;
    int oldIndex = 0;

    while (oldIndex != std::string::npos) {
        index = line.find(" ", index + 1);
        result.push_back(line.substr(oldIndex, index - oldIndex));

        if (index != std::string::npos) {
            oldIndex = index + 1;
        } else {
            oldIndex = index;
        }
    }

    return result;
}

void vcd::header::parse_var(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack) {
    auto entry = split_var_line(line);
    result.signal_names[entry[3]] = entry[4];

    for (auto& scope : scope_stack) {
        result.signal_scope[entry[3]] += scope + ".";
    }

    if (result.json_tree.length() > 0 && result.json_tree[result.json_tree.length() - 1] == '}') {
        result.json_tree += ",";
    }

    result.json_tree += "{\"name\": \"" + entry[4] + "\"}";
}

void vcd::header::parse_upscope(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack) {
    auto entry = split_var_line(line);
    scope_stack.pop_back();

    if (result.json_tree.length() > 0 && result.json_tree[result.json_tree.length() - 1] == ']') {
        result.json_tree += "}\n";
    }

    result.json_tree += "]}";
}

void vcd::header::parse_scope(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack) {
    auto entry = split_var_line(line);
    scope_stack.push_back(entry[2]);

    if (result.json_tree.length() > 0 && result.json_tree[result.json_tree.length() - 1] == '}') {
        result.json_tree += ",";
    }

    result.json_tree += "{\"name\": \"" + entry[2] + "\",\n";
    result.json_tree += "\"children\": [\n";
}

void vcd::header::parse_comment(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack) {
    auto entry = split_var_line(line);
    std::string comment;

    for (auto& scope : scope_stack) {
        comment += scope + ".";
    }    
    
    comment += " ";
    
    for (int i = 1; i < entry.size() - 1; i++) {
        comment += entry[i] + " ";
    }
        
    result.comments.push_back(comment);
}

bool vcd::header::parse(std::string& line, vcd::content& result, std::vector<std::string>& scope_stack) {
    int position = line.find("$var");

    if (position != std::string::npos) {
        parse_var(line, result, scope_stack);
    }

    position = line.find("$upscope");

    if (position != std::string::npos) {
        parse_upscope(line, result, scope_stack);
    }

    position = line.find("$scope");

    if (position != std::string::npos) {
        parse_scope(line, result, scope_stack);
    }
    
    position = line.find("$comment");

    if (position != std::string::npos) {
        parse_comment(line, result, scope_stack);
    }

    position = line.find("$enddefinitions");
    return position == std::string::npos;
}


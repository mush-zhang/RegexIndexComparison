#ifndef UTILS_REG_UTILS_HPP_
#define UTILS_REG_UTILS_HPP_

#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

//The true special/meta chars: '{', '}', '['. ']', '(', ')', '^', '$', '.', '*', '+', '?', '|'
const std::unordered_set<char> k_special_chars{'{', '[', '^', '$', '.', '*', '+', '?', '|'};
const std::unordered_set<char> k_special_classes{'w', 'W', 'a', 'b', 'B', 'd', 'D', 'l', 'p',
                                                 's', 'S', 'u', 'x'};

bool char_escaped(const std::string &line, std::size_t pos) {
    auto temp_pos = pos;
    int num_escaped = 0;
    // count number of escape characters before the char
    while (temp_pos > 0) {
        if (line.at(--temp_pos) != '\\') 
            break;
        num_escaped++;
    }
    return num_escaped % 2 == 1;
}

std::string remove_escape(const std::string & line) {
    // remove escape (trick method. should only remove single \ and turning \\ to \)
    std::string result(line);
    result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
    return result;
}

bool check_special(const std::string & reg_str) {
    // check if the string contains 
    //    1. special chars not escaped; or
    //    2. special class chars that are espcaed

    if (reg_str.empty()) return true;
    for (size_t i = 0; i < reg_str.size(); i++) {
        if ((k_special_chars.find(reg_str.at(i)) != k_special_chars.end() &&
             !char_escaped(reg_str, i)) || 
            (k_special_classes.find(reg_str.at(i)) != k_special_classes.end() &&
             char_escaped(reg_str, i))) {
            return true;
        }
    }
    return false;
}

// std::vector<

#endif // UTILS_REG_UTILS_HPP_
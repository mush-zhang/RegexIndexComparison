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

std::vector<std::string> extract_literals(const std::string & reg_str) {
    std::vector<std::string> result;

    size_t pos = 0;
    size_t prev_pos = 0;
    int pos2 = -1;

    while ((pos = reg_str.find("(", pos)) != std::string::npos) {
        // check if it is escaped
        if (!char_escaped(reg_str, pos)) {
            // if it is not escaped: it is a beginning of a capture group
            result.push_back(remove_escape(reg_str.substr(prev_pos, pos-prev_pos)));

            pos++;
            pos2 = pos;
            while ((pos2 = reg_str.find(")", pos2)) != std::string::npos) {
                if (!char_escaped(reg_str, pos2)) {
                    if (pos2 == reg_str.size() - 1 || reg_str.at(pos2+1) != '+') {
                        // Another rooted plan for the substr in capture group
                        auto sub_btw_brackets = reg_str.substr(pos, pos2-pos);
                        // check if there is special char
                        if (!check_special(sub_btw_brackets)) {
                            result.push_back(remove_escape(sub_btw_brackets));
                        }
                    } 
                    break;
                }
                pos2++;
            } 

            if (pos2 == std::string::npos) {
                auto sub_btw_brackets = reg_str.substr(pos);
                if (!check_special(sub_btw_brackets)) {
                    result.push_back(remove_escape(sub_btw_brackets));
                }
                break;
            } else {
                pos2++; // passed the ")"
                if (pos2 < reg_str.size() && reg_str.at(pos2) == ')' &&
                    (reg_str.at(pos2+1) == '+' || 
                     reg_str.at(pos2+1) == '*' || 
                     reg_str.at(pos2+1) == '?')) {
                    pos2++;
                }
            }
            prev_pos = pos2;
            pos = pos2;
        } else {
            pos++;
        }
    }
    if (pos2 != -1) {
        auto sub_btw_brackets = reg_str.substr(pos2);
        result.push_back(remove_escape(sub_btw_brackets));
    } else {
        result.push_back(remove_escape(reg_str));
    }
    return result;
} 

#endif // UTILS_REG_UTILS_HPP_
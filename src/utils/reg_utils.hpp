#ifndef UTILS_REG_UTILS_HPP_
#define UTILS_REG_UTILS_HPP_

#include <vector>
#include <stack>
#include <string>
#include <unordered_set>
#include <algorithm>

//The true special/meta chars: '{', '}', '[', ']', '(', ')', '^', '$', '.', '*', '+', '?', '|'
static const std::unordered_set<char> k_special_chars{'^', '$', '.', '|'};
static const std::unordered_set<char> k_rep_chars{'*', '+', '?'};
static const std::unordered_set<char> k_parath_l_chars{'{', '[', '('};
static const std::unordered_set<char> k_parath_r_chars{'}', ']', ')'};
static const std::unordered_set<char> k_special_classes{'w', 'W', 'a', 'b', 'B', 'd', 'D', 'l', 'p',
                                                 's', 'S', 'u', 'x', 'z'};

static bool char_escaped(const std::string &line, std::size_t pos) {
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

static std::string remove_escape(const std::string & line) {
    // remove escape (trick method. should only remove single \ and turning \\ to \)
    std::string result(line);
    result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
    return result;
}

static bool check_special(const std::string & reg_str) {
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

static std::vector<std::string> extract_literals(const std::string & reg_str) {
    std::vector<std::string> result;

    std::string curr_result = "";

    std::stack<char> para_stack;

    bool escaped = false;

    int pattern_end_idx = -2;

    for (size_t i = 0; i < reg_str.size(); i++) {
        char c = reg_str.at(i);
        char curr_char = '\n';
        if (c == '\\') {
            if (escaped) {
                curr_char = c;
                escaped = false;
            } else {
                escaped = true;
                continue;
            }
        } else if (k_special_classes.contains(c)) {
            if (!escaped) {
                curr_char = c;
            } else {
                continue;
            }
        } else if (k_special_chars.contains(c)) {
            if (escaped) {
                curr_char = c;
                escaped = false;
            } else {
                continue;
            }
        } else if (k_parath_l_chars.contains(c)) {
            if (escaped) {
                curr_char = c;
                escaped = false;
            } else {
                if (para_stack.empty() && curr_result.size() > 0) {
                    result.emplace_back(curr_result);
                    curr_result = "";
                }
                para_stack.push(c);
                continue;
            }
        } else if (k_parath_r_chars.contains(c)) {
            if (escaped) {
                curr_char = c;
                escaped = false;
            } else {
                if (para_stack.empty()) {
                    std::cerr << "Unmatched parathesis: missing left for ";
                    std::cerr << c << " at pos " << i << std::endl;
                    return std::vector<std::string>();
                } else if ((c == ')' && para_stack.top() == '(') || 
                           (c == ']' && para_stack.top() == '[') || 
                           (c == '}' && para_stack.top() == '{')) {
                    para_stack.pop();
                    if (para_stack.empty()) {
                        pattern_end_idx = i;
                    }
                    continue;
                } else {
                    std::cerr << "Unmatched parathesis: different left " << std::endl;
                    return std::vector<std::string>();
                }
            }
        } else if (k_rep_chars.contains(c)) {
            if (i == pattern_end_idx + 1 || (curr_result.empty() && i > 0)) {
                continue;
            } else {
                std::cerr << "Misplaced repetition char" << std::endl;
                return std::vector<std::string>();
            }
        } else {
            // assumes pattern components in parathesis
            curr_char = c;
        }
        escaped = false;

        if (para_stack.empty()) {
            curr_result += curr_char;
        }
#ifndef NDEBUG
        std::cout << curr_result << std::endl;
#endif  
    }
    if (!curr_result.empty()) {
        result.push_back(curr_result);
    }
    return result;
}

#endif // UTILS_REG_UTILS_HPP_

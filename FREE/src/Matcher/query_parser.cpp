#include "query_parser.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>

using QueryPlanNode = free_matcher::QueryPlanNode;
using AndNode = free_matcher::AndNode;
using OrNode = free_matcher::OrNode;
using NullNode = free_matcher::NullNode;
using LiteralNode = free_matcher::LiteralNode;

//The true special/meta chars: '{', '}', '['. ']', '(', ')', '^', '$', '.', '*', '+', '?', '|'
const std::unordered_set<char> k_special_chars{'{', '[', '^', '$', '.', '*', '+', '?', '|'};
const std::unordered_set<char> k_special_classes{'w', 'W', 'a', 'b', 'B', 'd', 'D', 'l', 'p',
                                                 's', 'S', 'u', 'x'};

enum OP { k_And_, k_Or_ };

// forward declaration
std::unique_ptr<QueryPlanNode> build_rooted_op_plan(std::string & reg_str, OP op);
std::unique_ptr<QueryPlanNode> build_rooted_plan(std::string & reg_str);

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

void remove_escape(std::string & line) {
    // remove escape (trick method. should only remove single \ and turning \\ to \)
    line.erase(std::remove(line.begin(), line.end(), '\\'), line.end());
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

std::unique_ptr<QueryPlanNode> make_const_node(const std::string & token) {
    if (token.empty()) {
        return std::make_unique<NullNode>();
    } else {
        return std::make_unique<LiteralNode>(token);
    }
}

std::unique_ptr<QueryPlanNode> make_op_node(char op, std::unique_ptr<QueryPlanNode> l, 
                                            std::unique_ptr<QueryPlanNode> r) {
    if (op == '&') {
        return std::make_unique<AndNode>(std::move(l), std::move(r));
    } else if (op == '|') {
        return std::make_unique<OrNode>(std::move(l), std::move(r));
    } 
    return nullptr;
}

void make_or_assign_node(std::string & str, char op_char, std::unique_ptr<QueryPlanNode> & curr) {
    if (!curr) {
        curr = build_rooted_plan(str);
    }
    else {
        auto temp = build_rooted_plan(str);
        curr = make_op_node(op_char, std::move(curr), std::move(temp));
    }
}

std::unique_ptr<QueryPlanNode> build_rooted_op_plan(std::string & reg_str, OP op) {

    // std::cout << "build_rooted_op_plan(" << reg_str << "," << op << ")" << std::endl;

    size_t pos = 0;
    size_t prev_pos = 0;
    int pos2 = -1;
    std::unique_ptr<QueryPlanNode> temp_null = nullptr;
    std::unique_ptr<QueryPlanNode> & curr = temp_null;

    std::string l_char, r_char;
    char op_char;
    if (op == k_And_) {
        l_char = "(";
        r_char = ")";
        op_char = '&';
    } else if (op == k_Or_) {
        l_char = "|";
        r_char = "|";
        op_char = '|';        
    }

    while ((pos = reg_str.find(l_char, pos)) != std::string::npos) {
        // check if it is escaped
        if (!char_escaped(reg_str, pos)) {
            // std::cout << "First delim at " << pos << " of " << reg_str << std::endl;
            // if it is not escaped: it is a beginning of a capture group
            std::string curr_str = reg_str.substr(prev_pos, pos-prev_pos);
            if (!curr_str.empty()) {
                make_or_assign_node(curr_str, op_char, curr);
            }

            pos++;
            pos2 = pos;
            // std::cout << "After first Delim, Looing for next node at  " << pos2 << " of " << reg_str << std::endl;

            // should exists a matching bracket in our case
            //  TODO not supporting multi-level brackets currently
            while ((pos2 = reg_str.find(r_char, pos2)) != std::string::npos) {
                if (!char_escaped(reg_str, pos2)) {
                    // std::cout << "Second delim at " << pos2 << " of " << reg_str << std::endl;
                    if (pos2 == reg_str.size() - 1 || 
                        (reg_str.at(pos2+1) != '*' && reg_str.at(pos2+1) != '?')) {
                        // Another rooted plan for the substr in capture group
                        auto sub_btw_brackets = reg_str.substr(pos, pos2-pos);
                        make_or_assign_node(sub_btw_brackets, op_char, curr);
                    } else {
                        curr = make_const_node("");
                    }
                    break;
                }
                pos2++;
            } 

            if (pos2 == std::string::npos) {
                auto sub_btw_brackets = reg_str.substr(pos);
                make_or_assign_node(sub_btw_brackets, op_char, curr);
                break;
            } else {
                pos2++;
                if (pos2 < reg_str.size() && reg_str.at(pos2) == ')' &&
                    (reg_str.at(pos2+1) == '+' || 
                     reg_str.at(pos2+1) == '*' || 
                     reg_str.at(pos2+1) == '?')) {
                    pos2++;
                }
            }
            // std::cout << "After second Delim, looking for next node at " << pos2 << " of " << reg_str << std::endl;

            prev_pos = pos2;
            pos = pos2;
        } else {
            pos++;
        }
    }
    if (pos2 != -1) {
        auto sub_btw_brackets = reg_str.substr(pos2);
        make_or_assign_node(sub_btw_brackets, op_char, curr);
    }

    // if not found, it can be a const or a OR
    return std::move(curr);
}

// Let us define a rooted component be: literal or capture group
std::unique_ptr<QueryPlanNode> build_rooted_plan(std::string & reg_str) {
    // std::cout << "build_rooted_plan(" << reg_str << ")" << std::endl;
    size_t pos = 0;
    size_t prev_pos = 0;
    std::unique_ptr<QueryPlanNode> temp_null = nullptr;
    std::unique_ptr<QueryPlanNode> & curr = temp_null;

    curr = build_rooted_op_plan(reg_str, k_And_);

    // check if it is a OR of constant literals
    if (curr) {
        return std::move(curr);
    }
    // std::cout << " No and found; finding or" << std::endl;
    curr = build_rooted_op_plan(reg_str, k_Or_);

    if (curr) {
        return std::move(curr);
    }
    // std::cout << " No or found; returning const " << reg_str << std::endl;
    // if still null, it is a constant
    if (check_special(reg_str)) {
        return std::move(make_const_node(""));
    }
    remove_escape(reg_str);
    return make_const_node(reg_str);
}

void free_matcher::QueryParser::generate_query_plan(const std::string & reg_str, 
                                                    bool remove_null) {
    // parse the string and pick out the literal component
    std::string curr_str(reg_str);
    k_query_plan_.reset(build_rooted_plan(curr_str).release());
}

void print_plan_helper(const std::string& prefix, 
                       const std::unique_ptr<QueryPlanNode> & node, 
                       bool isLeft) {
    if (node) {
        std::cout << prefix;
        std::cout << (isLeft ? "|--" : "L--");
        // print the value of the node
        std::cout << node->to_string() << std::endl;
        // enter the next tree level - left and right branch
        print_plan_helper(prefix + (isLeft ? "|   " : "    "), node->right_, true);
        print_plan_helper(prefix + (isLeft ? "|   " : "    "), node->left_, false);
    }
}

void free_matcher::QueryParser::print_plan() {
    std::cout << "#################### BEGIN PLAN #######################" << std::endl;
    print_plan_helper("", k_query_plan_, false);
    std::cout << "#################### END PLAN #######################" << std::endl;
}
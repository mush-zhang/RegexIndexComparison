#include <iostream>

#include "query_parser.hpp"
#include "../../utils/utils.hpp"

using QueryPlanNode = free_original_index::QueryPlanNode;
using AndNode = free_original_index::AndNode;
using OrNode = free_original_index::OrNode;
using NullNode = free_original_index::NullNode;
using LiteralNode = free_original_index::LiteralNode;

enum OP { kAnd, kOr };

// forward declaration
std::unique_ptr<QueryPlanNode> build_rooted_op_plan(const std::string & reg_str, OP op);
std::unique_ptr<QueryPlanNode> build_rooted_plan(const std::string & reg_str);

std::unique_ptr<QueryPlanNode> make_const_node(const std::string & token) {
    if (token.empty()) {
        return std::make_unique<NullNode>();
    } else {
        return std::make_unique<LiteralNode>(token);
    }
}

std::unique_ptr<QueryPlanNode> make_op_node(char op, 
        std::unique_ptr<QueryPlanNode> l, std::unique_ptr<QueryPlanNode> r) {
    if (op == '&') {
        return std::make_unique<AndNode>(std::move(l), std::move(r));
    } else if (op == '|') {
        return std::make_unique<OrNode>(std::move(l), std::move(r));
    } 
    return nullptr;
}

void make_or_assign_node(const std::string & str, char op_char, 
        std::unique_ptr<QueryPlanNode> & curr) {
    if (!curr) {
        curr = build_rooted_plan(str);
    }
    else {
        auto temp = build_rooted_plan(str);
        curr = make_op_node(op_char, std::move(curr), std::move(temp));
    }
}

std::unique_ptr<QueryPlanNode> build_rooted_op_plan(const std::string & reg_str, OP op) {
    size_t pos = 0;
    size_t prev_pos = 0;
    int pos2 = -1;
    std::unique_ptr<QueryPlanNode> k_nullptr = nullptr;
    std::unique_ptr<QueryPlanNode> & curr = k_nullptr;

    std::string l_char, r_char;
    char op_char;
    if (op == kAnd) {
        l_char = "(";
        r_char = ")";
        op_char = '&';
    } else if (op == kOr) {
        l_char = "|";
        r_char = "|";
        op_char = '|';        
    }

    while ((pos = reg_str.find(l_char, pos)) != std::string::npos) {
        // check if it is escaped
        if (!char_escaped(reg_str, pos)) {
            // if it is not escaped: it is a beginning of a capture group
            std::string curr_str = reg_str.substr(prev_pos, pos-prev_pos);
            if (!curr_str.empty()) {
                make_or_assign_node(curr_str, op_char, curr);
            }

            pos++;
            pos2 = pos;
            // should exists a matching bracket in our case
            //  TODO not supporting multi-level brackets currently

            // int l_count = 1;
            // while (pos2 < reg_str.size()) {
            //     if (reg_str.at(pos2) == ')' && (!char_escaped(reg_str, pos2))) {
            //         l_count--;
            //     } else if (reg_str.at(pos2) == '(' && (!char_escaped(reg_str, pos2))) {
            //         l_count++;
            //     }
            //     if (l_count == 0) {
            //         break;
            //     }
            //     pos2++;
            // }
            while ((pos2 = reg_str.find(r_char, pos2)) != std::string::npos) {
                if (!char_escaped(reg_str, pos2)) {
                    if (pos2 == reg_str.size() - 1 || 
                        (reg_str.at(pos2+1) != '*' && reg_str.at(pos2+1) != '?')) {
                        // Another rooted plan for the substr in capture group
                        auto sub_btw_brackets = reg_str.substr(pos, pos2-pos);
                        make_or_assign_node(sub_btw_brackets, op_char, curr);
                    } else {
                        make_or_assign_node("", op_char, curr);
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
                pos2++; // passed the ")"
                if (pos2 < reg_str.size() && reg_str.at(pos2-1) == ')' &&
                    (reg_str.at(pos2) == '+' || 
                     reg_str.at(pos2) == '*' || 
                     reg_str.at(pos2) == '?')) {
                    pos2++;
                }
            }
            prev_pos = pos2;
            pos = pos2;
        } else {
            pos++;
        }
    }
    if (pos2 != -1 && pos2 < reg_str.size()) {
        auto sub_btw_brackets = reg_str.substr(pos2);
        make_or_assign_node(sub_btw_brackets, op_char, curr);
    }

    // if not found, it can be a const or a OR
    return std::move(curr);
}

// Let us define a rooted component be: literal or capture group
std::unique_ptr<QueryPlanNode> build_rooted_plan(const std::string & reg_str) {
    size_t pos = 0;
    size_t prev_pos = 0;
    std::unique_ptr<QueryPlanNode> k_nullptr = nullptr;
    std::unique_ptr<QueryPlanNode> & curr = k_nullptr;

    curr = build_rooted_op_plan(reg_str, kAnd);

    // check if it is a OR of constant literals
    if (curr) {
        return std::move(curr);
    }
    curr = build_rooted_op_plan(reg_str, kOr);

    if (curr) {
        return std::move(curr);
    }
    // if still null, it is a constant
    if (check_special(reg_str)) {
        return std::move(make_const_node(""));
    }
    auto new_reg = remove_escape(reg_str);
    return make_const_node(new_reg);
}

void free_original_index::QueryParser::generate_query_plan(const std::string & reg_str) {
    // parse the string and pick out the literal component
    std::string curr_str(reg_str);
    query_plan_.reset(build_rooted_plan(curr_str).release());
}

void print_plan_helper(const std::string& prefix, 
                       const std::unique_ptr<QueryPlanNode> & node, 
                       bool is_left) {
    if (node) {
        std::cout << prefix;
        std::cout << (is_left ? "|--" : "L--");
        // print the value of the node
        std::cout << node->to_string() << std::endl;
        // enter the next tree level - left and right branch
        print_plan_helper(prefix + (is_left ? "|   " : "    "), node->right_, true);
        print_plan_helper(prefix + (is_left ? "|   " : "    "), node->left_, false);
    }
}

void free_original_index::QueryParser::print_plan() {
    std::cout << "#################### BEGIN PLAN #######################" << std::endl;
    print_plan_helper("", query_plan_, false);
    std::cout << "#################### END PLAN #######################" << std::endl;
}

void remove_null_node(std::unique_ptr<QueryPlanNode> & node);

void remove_null_node_and(std::unique_ptr<QueryPlanNode> & node) {
    // Assume node is not nullptr
    remove_null_node(node->left_);
    remove_null_node(node->right_);

    if (node->left_->is_null() && node->right_->is_null()) {
        // let current node be null
        node = make_const_node("");
    } else if (node->left_->is_null()) {
        node->left_.release();
        node = std::move(node->right_);
    } else if (node->right_->is_null()) {
        node->right_.release();
        node = std::move(node->left_);
    }
}

void remove_null_node_or(std::unique_ptr<QueryPlanNode> & node) {
    // Assume node is not nullptr
    remove_null_node(node->left_);
    remove_null_node(node->right_);

    if (node->left_->is_null() || node->right_->is_null()) {
        // let current node be null
        node = make_const_node("");
    } 
}

void remove_null_node(std::unique_ptr<QueryPlanNode> & node) {
    if (!node || node->is_null() ||
        node->get_type() == free_original_index::NodeType::kLiteralNode) {
        return;
    }
    if (node->get_type() == free_original_index::NodeType::kAndNode) {
        remove_null_node_and(node);
    } else if (node->get_type() == free_original_index::NodeType::kOrNode) {
        remove_null_node_or(node);
    } else {
        std::cerr << "Invalid node type my friend" << std::endl;
    }
} 

// Remove Null node according to rule in table 2
void free_original_index::QueryParser::remove_null() {
    remove_null_node(query_plan_);
}

void free_original_index::QueryParser::rewrite_node_by_index(std::unique_ptr<QueryPlanNode> & node) {
    if (!node || node->is_null()) {
        return;
    }

    if (node->get_type() == free_original_index::NodeType::kAndNode || 
        node->get_type() == free_original_index::NodeType::kOrNode) {
        rewrite_node_by_index(node->left_);
        rewrite_node_by_index(node->right_);
    } else if (node->get_type() == free_original_index::NodeType::kLiteralNode) {
        auto all_keys = k_index_.find_all_keys(node->to_string());
        if (all_keys.empty()) {
            node = make_const_node("");
        } else {
            std::unique_ptr<QueryPlanNode> k_nullptr = nullptr;
            std::unique_ptr<QueryPlanNode> & curr = k_nullptr;
            for (const auto key : all_keys) {
                make_or_assign_node(key, '&', curr);
            }
            node = std::move(curr);
        }
    } else {
        std::cerr << "Invalid node type my friend" << std::endl;
    }
} 

void free_original_index::QueryParser::rewrite_by_index() {
    if (k_index_.empty()) {
        return;
        // std::cerr << "No index provided; plan stays untouched." << std::endl;
    }
    rewrite_node_by_index(query_plan_);
    rewrote_by_index_ = true;
}

std::vector<size_t> free_original_index::QueryParser::get_index_by_node(
        std::unique_ptr<QueryPlanNode> & node) {
    std::vector<size_t> result;

    if (!node || node->is_null() ||
        node->get_type() == free_original_index::NodeType::kInvalidNode) {
        return result;
    }
    if (node->get_type() == free_original_index::NodeType::kLiteralNode) {
        return k_index_.get_line_pos_at(node->to_string());
    }
    auto left_idxs = get_index_by_node(node->left_);
    auto right_idxs = get_index_by_node(node->right_);

    if (node->get_type() == free_original_index::NodeType::kAndNode) {
        result = sorted_lists_intersection(left_idxs, right_idxs);
    } else if (node->get_type() == free_original_index::NodeType::kOrNode) {
        result = sorted_lists_union(left_idxs, right_idxs);        
    } 
    return result;
} 

bool free_original_index::QueryParser::get_index_by_plan(std::vector<size_t> & container) {
    if (!rewrote_by_index_) {
        rewrite_by_index();
    }
    if (!query_plan_ || query_plan_->is_null() ||
        query_plan_->get_type() == free_original_index::NodeType::kInvalidNode) {
        return false; // if helped_by_idx
    }
    auto result = get_index_by_node(query_plan_);
    container.swap(result);
    return true;
}

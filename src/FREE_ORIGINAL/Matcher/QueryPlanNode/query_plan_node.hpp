#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_

#include <memory>
#include <string>

#include "types.hpp"

namespace free_original_index {

class QueryPlanNode {
 public:
    QueryPlanNode() {}
    QueryPlanNode(std::unique_ptr<QueryPlanNode> l, std::unique_ptr<QueryPlanNode> r) 
        : left_(std::move(l)), right_(std::move(r)) {}
    ~QueryPlanNode() {}

    virtual const std::string & to_string() { return k_generic_node_name_; }
    virtual bool is_null() { return false; }
    virtual NodeType get_type() { return NodeType::kInvalidNode; }

    std::unique_ptr<QueryPlanNode> left_; 
    std::unique_ptr<QueryPlanNode> right_; 
 private:
    inline static const std::string k_generic_node_name_ = "";
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_
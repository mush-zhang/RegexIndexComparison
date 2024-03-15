#ifndef FREE_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_

#include <memory>
#include <string>

#include "types.hpp"

namespace free_matcher {

class QueryPlanNode {
 public:
    QueryPlanNode() {}
    QueryPlanNode(std::unique_ptr<QueryPlanNode> l, std::unique_ptr<QueryPlanNode> r) 
        : left_(std::move(l)), right_(std::move(r)) {}
    ~QueryPlanNode() {}

    virtual std::string to_string() { return ""; }
    virtual bool is_null() { return false; }
    virtual NodeType get_type() { return NodeType::kInvalid; }

    std::unique_ptr<QueryPlanNode> left_; 
    std::unique_ptr<QueryPlanNode> right_; 
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_QUERY_PLAN_NODE_HPP_
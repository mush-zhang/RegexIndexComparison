#ifndef FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

class NullNode: public QueryPlanNode {
 public:
    NullNode() {}
    ~NullNode() {}
    bool is_null() override { return true; }

    std::string to_string() override { return "NULL"; }    
    NodeType get_type() override { return NodeType::kNull; }
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
#ifndef FREE_MATCHER_QUERYPLANNODE_OR_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_OR_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

class OrNode: public QueryPlanNode {
 public:
    ~OrNode() {}
    using QueryPlanNode::QueryPlanNode;

    std::string to_string() override { return "OR"; }
    NodeType get_type() override { return NodeType::kOrNode; }

};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_OR_NODE_HPP_
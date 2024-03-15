#ifndef FREE_MATCHER_QUERYPLANNODE_AND_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_AND_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

class AndNode: public QueryPlanNode {
 public:
    ~AndNode() {}
    using QueryPlanNode::QueryPlanNode;
    std::string to_string() override { return "AND"; }
    NodeType get_type() override { return NodeType::kAndNode; }

};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_AND_NODE_HPP_
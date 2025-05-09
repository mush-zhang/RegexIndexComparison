#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_AND_NODE_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_AND_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_original_index {

class AndNode: public QueryPlanNode {
 public:
    ~AndNode() {}
    using QueryPlanNode::QueryPlanNode;
    const std::string & to_string() override { return k_and_node_name_; }
    NodeType get_type() override { return NodeType::kAndNode; }
 private:
    inline static const std::string k_and_node_name_ = "AND";
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_AND_NODE_HPP_
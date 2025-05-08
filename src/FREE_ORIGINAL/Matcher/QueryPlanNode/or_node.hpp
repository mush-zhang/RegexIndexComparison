#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_OR_NODE_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_OR_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_original_index {

class OrNode: public QueryPlanNode {
 public:
    ~OrNode() {}
    using QueryPlanNode::QueryPlanNode;

    const std::string & to_string() override { return k_or_node_name_; }
    NodeType get_type() override { return NodeType::kOrNode; }

 private:
    inline static const std::string k_or_node_name_ = "OR";
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_OR_NODE_HPP_
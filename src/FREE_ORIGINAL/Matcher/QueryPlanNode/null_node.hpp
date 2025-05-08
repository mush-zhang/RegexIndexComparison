#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_original_index {

class NullNode: public QueryPlanNode {
 public:
    NullNode() {}
    ~NullNode() {}
    bool is_null() override { return true; }

    const std::string & to_string() override { return k_null_node_name_; }    
    NodeType get_type() override { return NodeType::kNullNode; }
 private:
    inline static const std::string k_null_node_name_ = "NULL";
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
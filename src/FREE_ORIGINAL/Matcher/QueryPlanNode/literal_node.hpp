#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_original_index {

class LiteralNode: public QueryPlanNode {
 public:
    LiteralNode() {}
    ~LiteralNode() {}
    LiteralNode(const std::string & token) : literal_(token) {}
    const std::string & to_string() override { return literal_; }
    NodeType get_type() override { return NodeType::kLiteralNode; }

 private:
    std::string literal_;
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
#ifndef FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

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

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
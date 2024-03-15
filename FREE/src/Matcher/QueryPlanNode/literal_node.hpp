#ifndef FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

class LiteralNode: public QueryPlanNode {
 public:
    LiteralNode() {}
    ~LiteralNode() {}
    LiteralNode(const std::string & token) : literal_(token) {}
    std::string to_string() override { return literal_; }

 private:
    std::string literal_;
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_LITERAL_NODE_HPP_
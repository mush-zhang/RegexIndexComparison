#ifndef FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
#define FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_

#include "query_plan_node.hpp"

namespace free_matcher {

class NullNode: public QueryPlanNode {
 public:
    NullNode() {}
    ~NullNode() {}
    std::string to_string() override { return "NULL"; }    
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_NULL_NODE_HPP_
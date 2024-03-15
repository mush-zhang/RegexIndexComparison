#ifndef FREE_MATCHER_QUERY_PARSER_HPP_
#define FREE_MATCHER_QUERY_PARSER_HPP_

#include <memory>
#include "query_plan_node.hpp"

namespace free_matcher {

/**
 * @brief Parse query to plan tree; 
 *        refering to Table 1, we only need to deal with limited types of regexes
 *        For testing purpose we will 
 */
class QueryParser {
 public:
    QueryParser() {}
    ~QueryParser() {}

    void generate_query_plan(const std::string & reg_str);
    void remove_null();
    void print_plan();

 private:
    std::unique_ptr<QueryPlanNode> k_query_plan_;    
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERY_PARSER_HPP_
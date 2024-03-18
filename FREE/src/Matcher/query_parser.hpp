#ifndef FREE_MATCHER_QUERY_PARSER_HPP_
#define FREE_MATCHER_QUERY_PARSER_HPP_

#include <memory>
#include "query_plan_node.hpp"
#include "../Index/multigram_index.hpp"

namespace free {

/**
 * @brief Parse query to plan tree; 
 *        refering to Table 1, we only need to deal with limited types of regexes
 *        For testing purpose we will 
 */
class QueryParser {
 public:
    QueryParser() {}
    QueryParser(MultigramIndex* index) : k_index_(index) {}
    ~QueryParser() {}

    void generate_query_plan(const std::string & reg_str);
    void remove_null();
    void rewrite_by_index();
    std::vector<long> get_index_by_plan();
    void print_plan();

 private:
    std::unique_ptr<QueryPlanNode> query_plan_;
    MultigramIndex* k_index_ = nullptr;
    bool rewrote_by_index_ = false;
    void rewrite_node_by_index(std::unique_ptr<QueryPlanNode> & node);
    std::vector<long> get_index_by_node(std::unique_ptr<QueryPlanNode> & node);
};

} // namespace free

#endif // FREE_MATCHER_QUERY_PARSER_HPP_
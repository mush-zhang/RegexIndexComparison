#ifndef FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
#define FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_

namespace free_matcher {

enum NodeType {
    kInvalid,
    kAnd,
    kOr,
    kLiteral,
    kNull
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
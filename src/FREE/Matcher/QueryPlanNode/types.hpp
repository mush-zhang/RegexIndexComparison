#ifndef FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
#define FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_

namespace free_index {

enum NodeType {
    kInvalidNode,
    kAndNode,
    kOrNode,
    kLiteralNode,
    kNullNode
};

} // namespace free_index

#endif // FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
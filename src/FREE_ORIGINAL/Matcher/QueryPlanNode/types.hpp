#ifndef FREE_ORIGINAL_MATCHER_QUERYPLANNODE_TYPES_HPP_
#define FREE_ORIGINAL_MATCHER_QUERYPLANNODE_TYPES_HPP_

namespace free_original_index {

enum NodeType {
    kInvalidNode,
    kAndNode,
    kOrNode,
    kLiteralNode,
    kNullNode
};

} // namespace free_original_index

#endif // FREE_ORIGINAL_MATCHER_QUERYPLANNODE_TYPES_HPP_
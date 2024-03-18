#ifndef FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
#define FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_

namespace free {

enum NodeType {
    kInvalidNode,
    kAndNode,
    kOrNode,
    kLiteralNode,
    kNullNode
};

} // namespace free

#endif // FREE_MATCHER_QUERYPLANNODE_TYPES_HPP_
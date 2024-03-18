#include "multigram_index.hpp"

#ifndef FREE_INDEX_PRESUF_SHELL_HPP_
#define FREE_INDEX_PRESUF_SHELL_HPP_

namespace free {

class PresufShell: public MultigramIndex {
 public:
    PresufShell() = delete;
    PresufShell(const std::vector<std::string> &&) = delete;
    PresufShell(const std::vector<std::string> & dataset, double sel_threshold)
        : MultigramIndex(dataset, sel_threshold) {}
    ~PresufShell() {}
    void build_index(int upper_k) override;

 private:
    void compute_suffix_free_set();
};

} // namespace free

#endif // FREE_INDEX_PRESUF_SHELL_HPP_
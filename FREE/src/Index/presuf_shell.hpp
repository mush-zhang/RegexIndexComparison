#include "plain_multigram.hpp"

#ifndef FREE_INDEX_PRESUF_SHELL_HPP_
#define FREE_INDEX_PRESUF_SHELL_HPP_

namespace free_index {

class PresufShell: public PlainMultigram {
 public:

    PresufShell() = delete;
    PresufShell(const std::vector<std::string> &&) = delete;
    PresufShell(const std::vector<std::string> & dataset, double sel_threshold)
        : PlainMultigram(dataset, sel_threshold) {}
    ~PresufShell() {}
    void build_index(int upper_k) override;

 private:
    void compute_suffix_free_set();
};

} // namespace free_index

#endif // FREE_INDEX_PRESUF_SHELL_HPP_
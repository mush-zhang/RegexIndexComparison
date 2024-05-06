#ifndef FREE_INDEX_PRESUF_SHELL_HPP_
#define FREE_INDEX_PRESUF_SHELL_HPP_

#include "multigram_index.hpp"

namespace free_index {

class PresufShell: public MultigramIndex {
 public:
    PresufShell() = delete;
    PresufShell(const PresufShell &&) = delete;
    PresufShell(const std::vector<std::string> & dataset, double sel_threshold)
        : MultigramIndex(dataset, sel_threshold) {
            k_tag_ = "-presuf";
        }
    ~PresufShell() {}
    void build_index(int upper_n) override;

 protected:
    void fill_posting(int upper_n) override { MultigramIndex::fill_posting(upper_n); }

 private:
    void compute_suffix_free_set();
};

} // namespace free_index

#endif // FREE_INDEX_PRESUF_SHELL_HPP_
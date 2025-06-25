#include <iostream>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <thread>
#include <future>
#include <mutex>

using RecordId = size_t;
using PostingList = std::vector<RecordId>;
using Dataset = std::vector<std::string>;

// ------------------------------------
// Gram extension (thread local, flat map)
// ------------------------------------
void RecursiveExtend(
    const std::string& gram,
    const PostingList& rec_ids,
    const Dataset& dataset,
    size_t tau,
    size_t qmin,
    std::set<std::string>& k_index_keys_,
    std::unordered_map<std::string, PostingList>& k_index_,
    size_t maxlen = 16
) {
    if (gram.size() < qmin) return;
    if (rec_ids.size() <= tau || gram.size() >= maxlen) {
        k_index_keys_.insert(gram);
        k_index_[gram] = rec_ids;
        return;
    }
    std::unordered_map<std::string, PostingList> ext_map;
    for (RecordId rec_id : rec_ids) {
        const std::string& rec = dataset[rec_id];
        for (size_t pos = 0; pos + gram.size() < rec.size(); ++pos) {
            if (rec.substr(pos, gram.size()) == gram) {
                std::string ext_gram = gram + rec[pos + gram.size()];
                ext_map[ext_gram].push_back(rec_id);
            }
        }
    }
    for (auto& kv : ext_map) {
        std::sort(kv.second.begin(), kv.second.end());
        kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
        RecursiveExtend(kv.first, kv.second, dataset, tau, qmin, k_index_keys_, k_index_, maxlen);
    }
}

// ------------------------------------
// Thread-safe merge for index maps
// ------------------------------------
void MergeIndexMaps(
    std::set<std::string>& out_keys,
    std::unordered_map<std::string, PostingList>& out_index,
    const std::set<std::string>& in_keys,
    const std::unordered_map<std::string, PostingList>& in_index
) {
    for (const auto& gram : in_keys) {
        out_keys.insert(gram);
        auto& plist = out_index[gram];
        auto it = in_index.find(gram);
        if (it != in_index.end()) {
            plist.insert(plist.end(), it->second.begin(), it->second.end());
        }
    }
}

// ------------------------------------
// Parallel v-gram index construction
// ------------------------------------
void BuildVGramIndexParallel(
    const Dataset& dataset,
    size_t qmin,
    size_t tau,
    size_t thread_count_,
    std::set<std::string>& k_index_keys_,
    std::unordered_map<std::string, PostingList>& k_index_,
    size_t maxlen = 16
) {
    size_t N = dataset.size();
    size_t chunk = (N + thread_count_ - 1) / thread_count_;
    std::vector<std::future<void>> futures;
    std::vector<std::set<std::string>> thread_keys(thread_count_);
    std::vector<std::unordered_map<std::string, PostingList>> thread_index(thread_count_);

    for (size_t t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk, end = std::min(N, (t + 1) * chunk);
        futures.emplace_back(std::async(std::launch::async, [&, t, start, end]() {
            std::unordered_map<std::string, PostingList> initial_grams;
            for (RecordId rec_id = start; rec_id < end; ++rec_id) {
                const std::string& rec = dataset[rec_id];
                for (size_t i = 0; i + qmin <= rec.size(); ++i) {
                    std::string gram = rec.substr(i, qmin);
                    initial_grams[gram].push_back(rec_id);
                }
            }
            for (auto& kv : initial_grams) {
                std::sort(kv.second.begin(), kv.second.end());
                kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
                RecursiveExtend(kv.first, kv.second, dataset, tau, qmin, thread_keys[t], thread_index[t], maxlen);
            }
        }));
    }
    // Wait for threads
    for (auto& fut : futures) fut.get();

    // Merge all thread results (postings lists may overlap, need to deduplicate)
    for (size_t t = 0; t < thread_count_; ++t) {
        MergeIndexMaps(k_index_keys_, k_index_, thread_keys[t], thread_index[t]);
    }
    // De-duplicate all posting lists
    for (auto& kv : k_index_) {
        std::sort(kv.second.begin(), kv.second.end());
        kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
    }
}

// ------------------------------------
// Set cover: select minimal index keys to cover query literals
// ------------------------------------
std::vector<std::string> VGGraphGreedyCover(
    const std::vector<std::string>& literals,
    const std::set<std::string>& k_index_keys_,
    const std::unordered_map<std::string, PostingList>& k_index_
) {
    size_t total_chars = 0;
    std::vector<size_t> lit_offsets;
    for (const auto& lit : literals) {
        lit_offsets.push_back(total_chars);
        total_chars += lit.size();
    }
    std::unordered_map<std::string, std::vector<size_t>> gram2cover;
    for (const auto& gram : k_index_keys_) {
        std::vector<size_t> covered;
        size_t offset = 0;
        for (size_t li = 0; li < literals.size(); ++li) {
            const std::string& lit = literals[li];
            for (size_t i = 0; i + gram.size() <= lit.size(); ++i) {
                if (lit.substr(i, gram.size()) == gram) {
                    for (size_t k = 0; k < gram.size(); ++k)
                        covered.push_back(lit_offsets[li] + i + k);
                }
            }
        }
        if (!covered.empty()) gram2cover[gram] = std::move(covered);
    }
    std::vector<bool> covered(total_chars, false);
    size_t uncovered = total_chars;
    std::vector<std::string> selected;
    while (uncovered) {
        double best_score = std::numeric_limits<double>::max();
        std::string best_gram;
        for (const auto& kv : gram2cover) {
            const auto& gram = kv.first;
            size_t cost = k_index_.at(gram).size();
            size_t gain = 0;
            for (size_t pos : kv.second) if (!covered[pos]) ++gain;
            if (gain == 0) continue;
            double score = static_cast<double>(cost) / gain;
            if (score < best_score) {
                best_score = score;
                best_gram = gram;
            }
        }
        if (best_gram.empty()) break;
        for (size_t pos : gram2cover[best_gram]) {
            if (!covered[pos]) { covered[pos] = true; --uncovered; }
        }
        selected.push_back(best_gram);
    }
    return selected;
}

// ------------------------------------
// Dynamic tau: quantile of initial qmin-gram frequencies
// ------------------------------------
size_t DynamicTau(const std::unordered_map<std::string, PostingList>& grams, double quantile = 0.9) {
    std::vector<size_t> sizes;
    for (const auto& kv : grams) sizes.push_back(kv.second.size());
    std::sort(sizes.begin(), sizes.end());
    return sizes.empty() ? 10 : sizes[size_t(sizes.size() * quantile)];
}

// ------------------------------------
// Example usage with thread_count_
// ------------------------------------
int main() {
    Dataset k_dataset_ = {
        "thequickbrownfox", "jumpsoverthelazydog", "quickfoxquick", "lazydogbrownfox",
        "quickbrown", "foxjump", "thelazydog", "dogbrownquick", "quickthebrown"
    };
    std::vector<std::vector<std::string>> query_literals = {
        {"quick", "fox"},
        {"brown", "dog"},
        {"lazy", "the", "jump"},
        {"quick", "brown", "fox"}
    };
    size_t qmin = 2;
    size_t thread_count_ = std::max<size_t>(1, std::thread::hardware_concurrency());

    // For dynamic tau computation
    std::unordered_map<std::string, PostingList> gram2posting;
    for (RecordId rec_id = 0; rec_id < k_dataset_.size(); ++rec_id) {
        const std::string& rec = k_dataset_[rec_id];
        for (size_t i = 0; i + qmin <= rec.size(); ++i) {
            std::string gram = rec.substr(i, qmin);
            gram2posting[gram].push_back(rec_id);
        }
    }
    for (auto& kv : gram2posting) {
        std::sort(kv.second.begin(), kv.second.end());
        kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
    }
    size_t tau = DynamicTau(gram2posting, 0.8); // 80th percentile
    std::cout << "Chosen tau = " << tau << ", thread_count_ = " << thread_count_ << std::endl;

    std::set<std::string> k_index_keys_;
    std::unordered_map<std::string, PostingList> k_index_;

    BuildVGramIndexParallel(k_dataset_, qmin, tau, thread_count_, k_index_keys_, k_index_);

    std::cout << "Indexed grams (total " << k_index_keys_.size() << "): ";
    int n = 0; for (const auto& gram : k_index_keys_) {
        if (++n < 30) std::cout << gram << " ";
    }
    if (k_index_keys_.size() > 30) std::cout << "...";
    std::cout << "\n";

    // Query-time: set cover only on indexed grams
    for (size_t qidx = 0; qidx < query_literals.size(); ++qidx) {
        auto selected = VGGraphGreedyCover(query_literals[qidx], k_index_keys_, k_index_);
        std::cout << "Query #" << (qidx + 1) << " literals: ";
        for (const auto& l : query_literals[qidx]) std::cout << l << " ";
        std::cout << "\n  Selected grams: ";
        for (const auto& gram : selected) std::cout << gram << " ";
        std::cout << "\n";
    }
    return 0;
}

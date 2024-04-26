#include "freq_ngram.hpp"
#include "../../utils/reg_utils.hpp"
#include "../../utils/utils.hpp"

template<size_t N, size_t K> 
void rei_index::FreqNgramIndex<N,K>::select_grams(int upper_n) {
    // essentially get top k most frequent ngrams

    // count number of occurence for all ngrams
    std::unordered_map<std::array<char,N>, size_t, hash_array> ngram_counts;
    std::vector<std::array<char,N>> keys;
    
    for (const auto & reg_string : this->k_queries_) {
        auto curr_literals = extract_literals(reg_string);
        std::unordered_set<std::array<char,N>, hash_array> curr_grams;
        for (const auto & lit : curr_literals) {
            insert_unique_ngrams_to_set<N>(curr_grams, lit);
        }
        for (const auto & line_gram: curr_grams) {
            auto it = ngram_counts.find(line_gram);
            if (it != ngram_counts.end()) {
                ngram_counts[line_gram]++;
            } else {
                ngram_counts.insert({line_gram, 1});
                keys.push_back(line_gram);
            }
        }
    }

    // sort by number of occurence
    std::sort(
        keys.begin(), 
        keys.end(),
        [&ngram_counts](std::array<char,N> const& a, std::array<char,N> const& b) {
            auto lex_order = (a <=> b);
            if (lex_order == std::weak_ordering::equivalent) {
                std::cerr << "Should not happen to insert two equivalent keys" << std::endl;
                return false;
            }
            if (ngram_counts[a] > ngram_counts[b]) {
                return true;
            }
            else if (ngram_counts[a] < ngram_counts[b]) {
                return false;
            }
            // return a < b;
            return lex_order == std::weak_ordering::less;
    });

    auto threshold_count = k_threshold_ * this->k_queries_size_;
    size_t idx = 0;
    for (size_t key_idx = 0; key_idx < keys.size() && idx < K; key_idx++) {
        if (ngram_counts[keys[key_idx]] <= threshold_count) {
            this->k_index_keys_.insert({keys[key_idx], idx++});
        }
    }
}

template<size_t N, size_t K> 
void rei_index::FreqNgramIndex<N,K>::fill_posting() {
    for (const auto & line : this->k_dataset_) {
        // init a bitset of all 0's
        std::bitset<K> curr_bitarr;
        auto curr_grams = make_unique_ngrams<N>(line);
        if (curr_grams.size() > K) {
            for (const auto & [idx_gram, idx]: this->k_index_keys_) {
                if (curr_grams.find(idx_gram) != curr_grams.end()) {
                    // ngram exits, set bit at the index
                    curr_bitarr.set(idx);
                }
            }
        } else {
            for (const auto & line_gram: curr_grams) {
                auto it = this->k_index_keys_.find(line_gram);
                if (it != this->k_index_keys_.end()) {
                    // ngram exits, set bit at the index
                    curr_bitarr.set(it->second);
                }
            }
        }
        this->k_index_.push_back(curr_bitarr);
    }
}

template<size_t N, size_t K> 
void rei_index::FreqNgramIndex<N,K>::build_index(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_n);
    auto selection_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << selection_time << " s" << std::endl;
    
    std::ostringstream log;
    log << "REI-" << N << "-" << K << ","; // e.g.. REI-2-4
    log << this->thread_count_ << "," << N << "," << k_threshold_ << ",";
    log << selection_time << ",";

    start = std::chrono::high_resolution_clock::now();
    fill_posting();
    auto build_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    
    std::cout << "Index Building End in " << build_time << std::endl;

    log << build_time << "," << build_time+selection_time << ",";
    log << this->get_num_keys() << "," << this->get_bytes_used() << ",";
    this->write_to_file(log.str());
}

template class rei_index::FreqNgramIndex<2, 4>;
template class rei_index::FreqNgramIndex<2, 8>;
template class rei_index::FreqNgramIndex<2, 16>;
template class rei_index::FreqNgramIndex<2, 32>;
template class rei_index::FreqNgramIndex<2, 64>;
template class rei_index::FreqNgramIndex<2, 96>;
template class rei_index::FreqNgramIndex<2, 128>;
template class rei_index::FreqNgramIndex<2, 192>;
template class rei_index::FreqNgramIndex<2, 256>;
template class rei_index::FreqNgramIndex<2, 320>;
template class rei_index::FreqNgramIndex<2, 384>;
template class rei_index::FreqNgramIndex<2, 448>;
template class rei_index::FreqNgramIndex<2, 512>;

template class rei_index::FreqNgramIndex<3, 4>;
template class rei_index::FreqNgramIndex<3, 8>;
template class rei_index::FreqNgramIndex<3, 16>;
template class rei_index::FreqNgramIndex<3, 32>;
template class rei_index::FreqNgramIndex<3, 64>;
template class rei_index::FreqNgramIndex<3, 96>;
template class rei_index::FreqNgramIndex<3, 128>;
template class rei_index::FreqNgramIndex<3, 192>;
template class rei_index::FreqNgramIndex<3, 256>;
template class rei_index::FreqNgramIndex<3, 320>;
template class rei_index::FreqNgramIndex<3, 384>;
template class rei_index::FreqNgramIndex<3, 448>;
template class rei_index::FreqNgramIndex<3, 512>;

template class rei_index::FreqNgramIndex<4, 4>;
template class rei_index::FreqNgramIndex<4, 8>;
template class rei_index::FreqNgramIndex<4, 16>;
template class rei_index::FreqNgramIndex<4, 32>;
template class rei_index::FreqNgramIndex<4, 64>;
template class rei_index::FreqNgramIndex<4, 96>;
template class rei_index::FreqNgramIndex<4, 128>;
template class rei_index::FreqNgramIndex<4, 192>;
template class rei_index::FreqNgramIndex<4, 256>;
template class rei_index::FreqNgramIndex<4, 320>;
template class rei_index::FreqNgramIndex<4, 384>;
template class rei_index::FreqNgramIndex<4, 448>;
template class rei_index::FreqNgramIndex<4, 512>;
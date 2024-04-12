#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>

#include "../src/BEST/Index/single_threaded.hpp"
#include "../src/FAST/Index/lpms.hpp"
#include "../src/simple_query_matcher.hpp"

#include "../src/FREE/Index/multigram_index.hpp"
#include "../src/FREE/Matcher/query_matcher.hpp"

inline constexpr const char * kRegexDefault = "data/regexes_traffic.txt";
inline constexpr const char * kDataDefault = "data/US_Accidents_Dec21_updated.csv";

inline constexpr std::string_view kHeader = "regex\ttime(s)\tnum_match";

inline constexpr std::string_view kUsage = "usage:  \n\
    ./benchmark gram_selection input_regex_file input_data_file [output_file] \n\
    \t gram_selection: Name of the gram selection strategy. \n\
    \t                 Options available are 'FAST', 'BEST', 'FREE', 'REI'. \n\
    \t input_regex_file: Path to the list of regex queries. \n\
    \t                   Each line of the file is considered a regex query. \n\
    \t input_data_file:  Path to the list of data to be queried upon. \n\
    \t                   Each line of the file is considered an individual (log) line. \n\
    \t [output_file]:    Path to output file.";

char * getCmdOption(char ** begin, char ** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

template<class T>
std::pair<T, T> getStats(std::vector<T> & arr) {
    T num_reps = arr.size();
    std::sort(arr.begin(), arr.end());
    T ave= std::accumulate(arr.begin(), arr.end(), 0.0) / num_reps;
    T trimmed_ave = ave;
    if (num_reps > 3) {
        trimmed_ave = std::accumulate(arr.begin()+1, arr.end()-1, 0.0) / (num_reps-2);
    }
    return std::make_pair(ave, trimmed_ave);
}

std::vector<std::string> readTraffic() {
    std::string line;
    std::vector<std::string> lines;
    std::ifstream data_in(kDataDefault);
    if (!data_in.is_open()) {
        std::cerr << "Could not open data file '" << kDataDefault << "'" << std::endl;
        std::cerr << "Try downloading it first per instruction in ../data/dataset.txt" << std::endl;
        return lines;
    }
    size_t i = 0;
    // while (i++ < 100000 && getline(data_in, line)){
    while (getline(data_in, line)){
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::stringstream streamData(line);
        std::string s; 
        int i = 0;
        while (getline(streamData, s, ',')) {
            if (i++ == 9){
                lines.push_back(s);
                break;
            }
        }
    }
    data_in.close();
    return lines;
}

int parseArgs(int argc, char** argv, int * num_repeat, 
        std::string * input_regex_file, std::string * input_data_file) {
    // argument should have -r -d -o for optional input regex file, input data file, and output file specification

    if (argc < 2) {
        std::cerr << "Arguments missing: output file." << std::endl;
        std::cerr << kUsage << std::endl;
        return EXIT_FAILURE;
    }

    auto repeat_string = getCmdOption(argv, argv + argc, "-n");
    if (repeat_string) {
        *num_repeat = std::stoi(repeat_string);
    } else {
        *num_repeat = 10;
    }

    auto input_regex_file_raw = getCmdOption(argv, argv + argc, "-r");
    if (input_regex_file_raw) {
        *input_regex_file = input_regex_file_raw;
    } else {
        *input_regex_file = kRegexDefault;
    }
    auto input_data_file_raw = getCmdOption(argv, argv + argc, "-d");
    if (input_data_file_raw) {
        *input_data_file = input_data_file_raw;
    } else {
        *input_data_file = "";
    }

    return EXIT_SUCCESS;
}

std::vector<std::string> readDataIn(const std::string & file_type, 
                                    const std::string & infile_name) {
    std::vector<std::string> in_strings;
    std::ifstream data_in(infile_name);
    if (!data_in.is_open()) {
        std::cerr << "Could not open " << file_type << " file '" << infile_name << "'" << std::endl;
    } else {
        std::string line;
        while (getline(data_in, line)){
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            in_strings.push_back(line);
        }
        data_in.close();
    }
    return in_strings;
}

void run_end_to_end(const std::vector<std::string> & regexes, 
                    const std::vector<std::string> & lines) {
    auto pi = best_index::SingleThreadedIndex(lines, regexes, 0.1);
    pi.build_index();

    auto matcher = SimpleQueryMatcher(pi);

    matcher.match_all();
}

// void experiment(std::ofstream & r_file, const std::vector<std::string> & regexes, 
//         const std::vector<std::string> & lines, int num_repeat) {
//     for (const std::string & r : regexes) {
//         std::vector<double> elapsed_time_blare;
//         std::vector<double> elapsed_time_direct;
//         std::vector<double> elapsed_time_split;
//         std::vector<double> elapsed_time_multi;
//         std::vector<int> strategies;
//         int match_count_blare;
//         int match_count_direct;
//         int match_count_split;
//         int match_count_multi;
//         for (int i = 0; i < num_repeat; i++) {
//             auto [dtime, dnum] = DirectMatch(lines, r);
//             auto [smtime, smnum] = SplitMatch3Way(lines, r);
//             auto [btime, bnum, bstrat] = Blare(lines, r);
//             auto [multime, mulnum] = SplitMatchMultiWay(lines, r);

//             elapsed_time_blare.push_back(btime);
//             strategies.push_back(bstrat);
//             elapsed_time_direct.push_back(dtime);
//             elapsed_time_split.push_back(smtime);
//             elapsed_time_multi.push_back(multime);

//             match_count_blare = bnum;
//             match_count_direct = dnum;
//             match_count_split = smnum;
//             match_count_multi = mulnum;
//         }
        
//         auto [ave_direct, mid_ave_direct] = getStats<double>(elapsed_time_direct);
//         auto [ave_multi, mid_ave_multi] = getStats<double>(elapsed_time_multi);
//         auto [ave_split, mid_ave_split] = getStats<double>(elapsed_time_split);
//         auto [ave_blare, mid_ave_blare] = getStats<double>(elapsed_time_blare);
//         auto [ave_stratgy, mid_ave_strategy] = getStats<int>(strategies);

//         std::vector<double> mid_times(3, 0);
//         mid_times[ARM::kSplitMatch] = mid_ave_split;
//         mid_times[ARM::kMultiMatch] = mid_ave_multi;
//         mid_times[ARM::kDirectMatch] = mid_ave_direct;
//         auto true_strategy = std::distance(mid_times.begin(), std::min_element(mid_times.begin(), mid_times.end()));
//         auto strategy_diff = std::count(strategies.begin(), strategies.end(), true_strategy);

//         r_file << r << "\t";
//         r_file << true_strategy << "\t"  << ave_stratgy << "\t" << mid_ave_strategy << "\t" << strategy_diff << "\t";
//         r_file << ave_blare << "\t" << mid_ave_blare << "\t";
//         r_file << ave_multi << "\t" << mid_ave_multi << "\t";
//         r_file << ave_split << "\t" << mid_ave_split << "\t";
//         r_file << ave_direct << "\t" << mid_ave_direct << "\t";
//         r_file << match_count_blare << "\t" << match_count_multi << "\t" << match_count_split << "\t" << match_count_direct << std::endl;
//     }
// }

template std::pair<int, int> getStats(std::vector<int> & arr);
template std::pair<double, double> getStats(std::vector<double> & arr);
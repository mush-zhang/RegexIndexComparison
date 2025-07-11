#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string.h>
#include <filesystem>
#include <climits>

#include "../src/BEST/Index/parallelizable.hpp"

#include "../src/FREE/Index/multigram_index.hpp"
#include "../src/FREE/Index/presuf_shell.hpp"
#include "../src/FREE/Index/parallel_multigram_index.hpp"
#include "../src/Trigram/Index/trigram_inverted_index.hpp"
#include "../src/VGGRAPH_GREEDY/Index/vggraph_greedy_index.hpp"

#include "../src/simple_query_matcher.hpp"

#include "utils.hpp"
#include "../src/utils/reg_utils.hpp"

inline constexpr const int kNumIndexBuilding = 1;

inline constexpr const char * kTrafficRegex = "data/regexes_traffic.txt";
inline constexpr const char * kDbxRegex = "data/regexes_dbx.txt";
inline constexpr const char * kSysyRegex = "data/regexes_sysy.txt";
inline constexpr const char * kEnronRegex = "data/enron/regexes_enron_wlit.txt";
inline constexpr const char * kWebRegex = "data/webpages/regexes_webpages.txt";
inline constexpr const char * kWebRegexFree = "data/webpages/regexes_webpages_free.txt";
inline constexpr const char * kPrositeRegex = "data/protein/prosites.txt";

inline constexpr const std::string_view kSummaryHeader = 
    "name,num_threads,gram_size,selectivity,key_upper_bound,num_queries,selection_time,build_time,overall_index_time,num_keys,index_size,compile_time,match_time";

inline constexpr const std::string_view kExprHeader = "regex\ttime\tcount\tnum_after_filter";

selection_type get_method(const std::string gs) {
    if (gs == "FREE") {
        return selection_type::kFree;
    }
    if (gs == "BEST") {
        return selection_type::kBest;
    }
    if (gs == "LPMS") {
        return selection_type::kFast;
    }
    if (gs == "TRIGRAM") {
        return selection_type::kTrigram;
    }
    if (gs == "VGGRAPH") {
        return selection_type::kVGGraph;
    }
    if (gs == "NONE") {
        return selection_type::kNone;
    }
    return selection_type::kInvalid;
}

void error_print(const std::string & msg) {
    std::cerr << "Error: " << msg << std::endl;
    std::cerr << kUsage << std::endl;
}

int error_return(const std::string & msg) {
    error_print(msg);
    return EXIT_FAILURE;
}

std::string getCmdOption(char ** begin, char ** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return std::string(*itr);
    }
    return "";
}

bool cmdOptionExists(char ** begin, char ** end, const std::string & option) {
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

int parseArgs(int argc, char ** argv, 
             expr_info & expr_info, 
             free_info & free_info, best_info & best_info, 
             lpms_info & lpms_info, 
             trigram_info & trigram_info,
             vggraph_info & vggraph_info) {

    if (argc < 6) {
        return error_return("Missing required arguments.");
    }

    // check gram selection type
    expr_info.stype = get_method(argv[1]);
    if (expr_info.stype == selection_type::kInvalid) {
        return error_return("Invalid gram selection method.");
    }

    // check workload
    auto wl_string = getCmdOption(argv, argv + argc, "-w");
    if (wl_string.empty()) {
        return error_return("Missing type of workload used.");
    } else {
        expr_info.wl = std::stoi(wl_string);
        if (expr_info.wl < 0 || expr_info.wl > 6) {
            return error_return("Invalid workload type.");
        } else if (expr_info.wl == 0) {
            expr_info.reg_file = getCmdOption(argv, argv + argc, "-r");
            if (expr_info.reg_file.empty()) {
                return error_return("Missing path to regex file for customized workload.");        
            }
            expr_info.data_file = getCmdOption(argv, argv + argc, "-d");
            if (expr_info.data_file.empty()) {
                return error_return("Missing path to data file for customized workload.");        
            }
        }
    }

    expr_info.out_dir = getCmdOption(argv, argv + argc, "-o");
    if (expr_info.out_dir.empty()) {
        return error_return("Missing output file.");
    }

    auto repeat_string = getCmdOption(argv, argv + argc, "-e");
    int rep = 10;
    if (!repeat_string.empty()) {
        rep = std::stoi(repeat_string);
    }

    auto max_key_string = getCmdOption(argv, argv + argc, "-k");
    long long int max_key = LLONG_MAX;
    if (!max_key_string.empty()) {
        max_key = std::stol(max_key_string);
    }
    expr_info.test_reg_file = getCmdOption(argv, argv + argc, "--test");

    auto thread_string = getCmdOption(argv, argv + argc, "-t");
    int thread_count = 0;
    if (thread_string.empty()) {
        return error_return("Missing number of threads.");
    } else {
        thread_count = std::stoi(thread_string);
    }
    auto gram_size_string = getCmdOption(argv, argv + argc, "-n");
    int n = 0;
    if (!gram_size_string.empty()) {
        n = std::stoi(gram_size_string);
    }
    auto selec_string = getCmdOption(argv, argv + argc, "-c");
    double selec = -1;
    if (!selec_string.empty()) {
        selec = std::stod(selec_string);
    }
    switch (expr_info.stype) {
        case selection_type::kNone:
            expr_info.num_repeat = rep;
            break;
        case selection_type::kFree: {
            free_info.num_repeat = rep;
            free_info.key_upper_bound = max_key;
            free_info.num_threads = thread_count;
            if (n == 0) {
                return error_return("Missing/Invalid upper bound on n of n-gram.");
            }
            free_info.upper_n = n;
            if (selec > 0 && selec <= 1) {
                free_info.sel_threshold = selec;
            } 
            free_info.use_presuf = cmdOptionExists(argv, argv + argc, "--presuf");
            break;
        }
        case selection_type::kBest: {
            best_info.num_repeat = rep;
            best_info.key_upper_bound = max_key;
            best_info.num_threads = thread_count;
            if (selec > 0 && selec <= 1) {
                best_info.sel_threshold = selec;
            } 
            auto wl_reduce_string = getCmdOption(argv, argv + argc, "--wl_reduce");
            if (!wl_reduce_string.empty()) {
                if (wl_reduce_string.find('.') != std::string::npos) {
                    best_info.wl_reduced_frac = std::stod(wl_reduce_string);
                    if (best_info.wl_reduced_frac <= 0 || best_info.wl_reduced_frac > 1) {
                        return error_return("Invalid fraction of workload reduction.");                        
                    }
                } else {
                    best_info.wl_reduced_size = std::stoi(wl_reduce_string);
                }
            }
            auto dtype_string = getCmdOption(argv, argv + argc, "--dist");
            if (!dtype_string.empty()) {
                switch (std::stoi(dtype_string)) {
                    case 1:
                        best_info.dtype = best_index::dist_type::kMaxDevDist1;
                        break;
                    case 2:
                        break;
                    case 3:
                        best_info.dtype = best_index::dist_type::kMaxDevDist3;
                        break;
                    default:
                        return error_return("Invalid distance type.");
                }
            }
            break;
        }
        case selection_type::kFast: {
            lpms_info.num_repeat = rep;
            lpms_info.key_upper_bound = max_key;
            lpms_info.num_threads = thread_count;
            auto relax_string = getCmdOption(argv, argv + argc, "--relax");
            lpms_info.rtype_str = relax_string;
            if (relax_string.empty()) {
                return error_return("Missing type of relaxation.");
            } else if (relax_string == "DETERM") {
                lpms_info.rtype = lpms_index::relaxation_type::kDeterministic;
            } else if (relax_string == "RANDOM") {
                lpms_info.rtype = lpms_index::relaxation_type::kRandomized;
            } else {
                return error_return("Invalid relaxation type.");
            }
            break;
        }
        case selection_type::kTrigram: {
            trigram_info.num_repeat = rep;
            trigram_info.key_upper_bound = max_key;
            trigram_info.num_threads = thread_count;
            break;
        }
        case selection_type::kVGGraph: {
            vggraph_info.num_repeat = rep;
            vggraph_info.key_upper_bound = max_key;
            vggraph_info.num_threads = thread_count;
            if (n == 0) {
                return error_return("Missing/Invalid upper bound on n of n-gram.");
            }
            vggraph_info.upper_n = n;
            if (selec > 0 && selec <= 1) {
                vggraph_info.selectivity_threshold = static_cast<float>(selec);
            } 
            break;
        }
        default:
            // should not have reached here.
            return error_return("Invalid gram selection method. (Wrong place to return)");
    }

    return EXIT_SUCCESS;
}

std::vector<std::string> read_traffic(int max_lines=-1) {
    std::string line;
    std::vector<std::string> lines;
    std::string traffic_file = "data/US_Accidents_Dec21_updated.csv";

    std::ifstream data_in(traffic_file);
    if (!data_in.is_open()) {
        std::cerr << "Could not open data file '" << traffic_file << "'" << std::endl;
        return lines;
    }

    size_t i = 0;
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
        if (max_lines > 0 && lines.size() > max_lines) {
            break;
        }
    }
    data_in.close();
    return lines;
}

auto read_file_to_string(const std::string & path) -> std::string {
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path);
    stream.exceptions(std::ios_base::badbit);

    if (not stream) {
        throw std::ios_base::failure("file does not exist");
    }
    
    auto out = std::string();
    auto buf = std::string(read_size, '\0');

    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    stream.close();
    return out;
}

std::vector<std::string> read_webpages(int max_lines=-1) {
    std::string line;

    std::vector<std::string> lines;
    std::string path = "data/webpages/processed";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        std::string data_file = entry.path();
        lines.push_back(read_file_to_string(data_file));
        if (max_lines > 0 && lines.size() > max_lines) {
            return lines;
        }
    }
    return lines;
}

void read_file(const std::string & file_type, const std::string & infile_name,
               std::vector<std::string> & in_strings, int max_lines=-1) {
    std::ifstream data_in(infile_name);
    if (!data_in.is_open()) {
        std::cerr << "Could not open " << file_type << " file '";
        std::cerr << infile_name << "'" << std::endl;
    } else {
        std::string line;
        while (getline(data_in, line)){
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            in_strings.push_back(line);
            if (max_lines > 0 && in_strings.size() > max_lines) {
                break;
            }
        }
        data_in.close();
    }
}

std::vector<std::string> read_file(const std::string & file_type, 
                                   const std::string & infile_name,
                                   int max_lines=-1) {
    std::vector<std::string> in_strings;
    std::ifstream data_in(infile_name);
    if (!data_in.is_open()) {
        std::cerr << "Could not open " << file_type << " file '";
        std::cerr << infile_name << "'" << std::endl;
    } else {
        std::string line;
        while (getline(data_in, line)){
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            in_strings.push_back(line);
            if (max_lines > 0 && in_strings.size() > max_lines) {
                break;
            }
        }
        data_in.close();
    }
    return in_strings;
}

std::vector<std::string> read_directory(const std::string & file_type, 
                                        const std::string & path, int max_lines=-1) {
    std::string line;

    std::vector<std::string> lines;
    for (const auto & entry : std::filesystem::recursive_directory_iterator(path)) {
        std::string data_file = entry.path();
        read_file(file_type, data_file, lines, max_lines);
    }
    return lines;
}

std::vector<std::string> read_enron(int max_files) {
    std::vector<std::string> data_strings;
    
    try {
        for (const auto & entry : std::filesystem::recursive_directory_iterator("data/enron/maildir")) {
            // Only process regular files, skip directories
            if (!entry.is_regular_file()) {
                continue;
            }
            
            std::string file_path = entry.path();
            std::ifstream file_in(file_path);
            
            if (!file_in.is_open()) {
                std::cerr << "Could not open Enron file: " << file_path << std::endl;
                continue;
            }
            
            // Read entire file content as one string, preserving newlines
            std::string file_content;
            std::string line;
            bool first_line = true;
            
            while (std::getline(file_in, line)) {
                if (!first_line) {
                    file_content += "\n";
                }
                // Remove carriage returns but keep the content
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                file_content += line;
                first_line = false;
            }
            
            file_in.close();
            
            // Add the complete file content as one data string
            data_strings.push_back(file_content);
            
            // Check if we've reached the maximum number of files
            if (max_files > 0 && data_strings.size() >= max_files) {
                break;
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error while reading Enron directory: " << ex.what() << std::endl;
    }
    
    return data_strings;
}

int readWorkload(const expr_info & expr_info, 
                 std::vector<std::string> & regexes, 
                 std::vector<std::string> & test_regexes, 
                 std::vector<std::string> & lines,
                 int max_lines) {
    switch (expr_info.wl) {
        case 1: 
            regexes = read_file("regex", kTrafficRegex);
            lines = read_traffic(max_lines);
            break;
        case 2:
            regexes = read_file("regex", kDbxRegex);
            // lines = read_dbx(max_lines);
            lines = read_directory("data", "data/extracted", max_lines);
            break;
        case 3:
            if (expr_info.stype == selection_type::kFree) {
                regexes = read_file("regex", kWebRegexFree);
            } else {
                regexes = read_file("regex", kWebRegex);
            }
            lines = read_webpages(std::min(max_lines, 7000000));
            break;
        case 4:
            regexes = read_file("regex", kPrositeRegex);
            lines = read_directory("data", "data/protein/sequences", max_lines);
            break;
        case 5:
            regexes = read_file("regex", kSysyRegex);
            lines = read_file("data", "data/tagged_data.csv", max_lines);
            break;
        case 6:
            regexes = read_file("regex", kEnronRegex);
            lines = read_enron(max_lines);
            break;
        default:
            regexes = read_file("regex", expr_info.reg_file);
            lines = read_file("data", expr_info.data_file, max_lines);
    }

    std::cout << "read workload end." << std::endl;
    std::cout << "Number of regexes: " << regexes.size() << "."<< std::endl;
    
    if (!expr_info.test_reg_file.empty()) {
        test_regexes = read_file("regex", expr_info.test_reg_file);
        std::cout << "Number of test regexes: " << test_regexes.size() << "."<< std::endl;
    }

    std::cout << "Number of data lines: " << lines.size() << "." << std::endl;

#ifndef NDEBUG
    std::cout << "Print literals" << std::endl;

    for (const auto & reg : regexes) {
        std::cout << reg << std::endl;
        std::cout << "\t";
        auto literals = extract_literals(reg);
        for (const auto & lit : literals) {
            std::cout << "[ " << lit << " ] ";
        }
        std::cout << std::endl;
    }
#endif  

    if (regexes.empty() || lines.empty()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::ofstream open_summary(const std::filesystem::path & dir_path) {
    // open summary csv file
    std::filesystem::path out_path = dir_path / "summary.csv";
    std::ofstream outfile;
    if (!std::filesystem::exists(out_path)) {
        // write header
        outfile.open(out_path, std::ios::out);
        outfile << kSummaryHeader << std::endl;
    } else {
        outfile.open(out_path, std::ios::app);
    }
    return std::move(outfile);
}

void benchmarkFree(const std::filesystem::path dir_path, 
                   const std::vector<std::string> & regexes, 
                   const std::vector<std::string> & test_regexes, 
                   const std::vector<std::string> & lines,
                   const free_info & free_info) {
    std::ofstream outfile = open_summary(dir_path);

    std::ostringstream stats_name;
    // index building
    free_index::MultigramIndex * pi = nullptr;
    if (free_info.use_presuf) {
        stats_name << "FREE-presuf_";
        pi = new free_index::PresufShell(lines, free_info.sel_threshold, 
                                         std::max(1, free_info.num_threads));
    } else {
        stats_name << "FREE_";
        if (free_info.num_threads <= 1) {
            pi = new free_index::MultigramIndex(lines, free_info.sel_threshold);
        } else {
            pi = new free_index::ParallelMultigramIndex(lines, 
                 free_info.sel_threshold, free_info.num_threads);
        }
    }
    pi->set_key_upper_bound(free_info.key_upper_bound);
    pi->set_outfile(outfile);
    pi->build_index(free_info.upper_n);

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < free_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            pi->write_to_file(",,,,,,,,,,,");
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << free_info.num_threads << "_" << free_info.upper_n;
    stats_name << "_" << free_info.sel_threshold << "_";
    stats_name << free_info.key_upper_bound << "_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << matcher.get_num_after_filter(regex) << std::endl;
    }

    statsfile.close();
}

void benchmarkBest(const std::filesystem::path dir_path, 
                   const std::vector<std::string> & regexes, 
                   const std::vector<std::string> & test_regexes, 
                   const std::vector<std::string> & lines,
                   const best_info & best_info) {
    if (best_info.wl_reduced_size > int(regexes.size())) {
        std::cerr << best_info.wl_reduced_size << " " << regexes.size() << std::endl;
        error_print("Invalid workload reduction size larger than number of queries.");
        return;
    }
    std::ofstream outfile = open_summary(dir_path);

    bool reduce = true;
    double red_size = best_info.wl_reduced_size;
    if (red_size < 0) {
        if (best_info.wl_reduced_frac < 0) {
            reduce = false;
        } else {
            red_size = std::min(
                std::max(best_info.wl_reduced_frac * regexes.size(), 10.0), 
                static_cast<double>(regexes.size())
            );
        }
    } 

    std::ostringstream stats_name;
    // index building
    best_index::SingleThreadedIndex * pi = nullptr;
    if (best_info.num_threads > 1) {
        if (reduce) {
            pi = new best_index::ParallelizableIndex(
                    lines, regexes, best_info.sel_threshold, best_info.num_threads,
                    red_size, best_info.dtype);
        } else {
            pi = new best_index::ParallelizableIndex(
                    lines, regexes, best_info.sel_threshold, best_info.num_threads);
        }
    } else {
        if (reduce) {
            pi = new best_index::SingleThreadedIndex(
                    lines, regexes, best_info.sel_threshold,
                    red_size, best_info.dtype);
        } else {
            pi = new best_index::SingleThreadedIndex(
                    lines, regexes, best_info.sel_threshold);
        }

    }
    pi->set_key_upper_bound(best_info.key_upper_bound);
    pi->set_outfile(outfile);
    pi->build_index();

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < best_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            pi->write_to_file(",,,,,,,,,,,");
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << "BEST_" << best_info.num_threads << "_" << "-1";
    stats_name << "_" << best_info.sel_threshold << "_" << red_size;
    stats_name << "_" << best_info.key_upper_bound << "_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << matcher.get_num_after_filter(regex) << std::endl;
    }

    statsfile.close();
}

void benchmarkFast(const std::filesystem::path dir_path,
                   const std::vector<std::string> & regexes, 
                   const std::vector<std::string> & test_regexes, 
                   const std::vector<std::string> & lines,
                   const lpms_info & lpms_info) {

    std::ofstream outfile = open_summary(dir_path);

    std::ostringstream stats_name;
    // index building
    auto * pi = new lpms_index::LpmsIndex(lines, regexes, lpms_info.num_threads, lpms_info.rtype);
    pi->set_thread_count(lpms_info.num_threads);
    pi->set_key_upper_bound(lpms_info.key_upper_bound);
    pi->set_outfile(outfile);
    pi->build_index();

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < lpms_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            pi->write_to_file(",,,,,,,,,,,");
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << "LPMS-" << lpms_info.rtype_str << "_" << lpms_info.num_threads << "_" << "-1";
    stats_name << "_" << "-1" << "_" << lpms_info.key_upper_bound << "_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << matcher.get_num_after_filter(regex) << std::endl;
    }

    statsfile.close();
}


void benchmarkTrigram(const std::filesystem::path dir_path,
                   const std::vector<std::string> & regexes, 
                   const std::vector<std::string> & test_regexes, 
                   const std::vector<std::string> & lines,
                   const trigram_info & trigram_info) {

    std::ofstream outfile = open_summary(dir_path);

    std::ostringstream stats_name;
    // index building
    auto * pi = new trigram_index::TrigramInvertedIndex(lines, regexes);
    pi->set_thread_count(trigram_info.num_threads);
    pi->set_key_upper_bound(trigram_info.key_upper_bound);
    pi->set_outfile(outfile);
    pi->build_index();

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < trigram_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            pi->write_to_file(",,,,,,,,,,,");
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << "Trigram" << "_" << trigram_info.num_threads << "_" << "-1";
    stats_name << "_" << "-1" << "_" << trigram_info.key_upper_bound << "_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << matcher.get_num_after_filter(regex) << std::endl;
    }

    statsfile.close();
}

void benchmarkVGGraph(const std::filesystem::path dir_path,
                      const std::vector<std::string> & regexes, 
                      const std::vector<std::string> & test_regexes, 
                      const std::vector<std::string> & lines,
                      const vggraph_info & vggraph_info) {
    std::ofstream outfile = open_summary(dir_path);

    std::ostringstream stats_name;
    stats_name << "VGGRAPH_";
    
    // index building
    auto pi = new vggraph_greedy_index::VGGraph_Greedy(lines, regexes, 
                                                 vggraph_info.selectivity_threshold,
                                                 vggraph_info.upper_n,
                                                 vggraph_info.num_threads);
    pi->set_key_upper_bound(vggraph_info.key_upper_bound);
    pi->set_outfile(outfile);
    pi->build_index(vggraph_info.upper_n);

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < vggraph_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            pi->write_to_file(",,,,,,,,,,,");
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << vggraph_info.num_threads << "_" << vggraph_info.upper_n;
    stats_name << "_" << vggraph_info.selectivity_threshold << "_";
    stats_name << vggraph_info.key_upper_bound << "_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << matcher.get_num_after_filter(regex) << std::endl;
    }

    statsfile.close();
    delete pi;
}

void benchmarkBaseline(const std::filesystem::path dir_path,
                        const std::vector<std::string> & regexes, 
                        const std::vector<std::string> & test_regexes, 
                        const std::vector<std::string> & lines,
                        const expr_info & expr_info) {

    std::ofstream outfile = open_summary(dir_path);

    std::ostringstream stats_name;
    // dummy index
    auto * pi = new lpms_index::LpmsIndex(lines, regexes);
    pi->set_outfile(outfile);

    auto tr = regexes;
    if (!test_regexes.empty()) {
        tr = test_regexes;
    }

    for (size_t i = 0; i < expr_info.num_repeat; i++) {
        if (i >= kNumIndexBuilding) {
            // not re-running the time consuming index afterwards,
            // filling the empty slots
            outfile << ",,,,,,,,,,," << std::endl;
        } else {
            outfile << "Baseline,,,,,,,,,,," << std::endl;
        }
        // matching; add match time to the overall file
        auto matcher = SimpleQueryMatcher(*pi, tr);
        matcher.match_all();
    }

    outfile.close();

    // open stats file
    stats_name << "Baseline_stats.csv";
    std::filesystem::path stats_path = dir_path / stats_name.str();
    std::ofstream statsfile;
    statsfile.open(stats_path, std::ios::out);
    statsfile << kExprHeader << std::endl;
    pi->set_outfile(statsfile);

    auto matcher = SimpleQueryMatcher(*pi, tr, false);

    // Get individual stats
    for (const auto & regex : tr) {
        statsfile << regex << "\t";
        matcher.match_one(regex);
        statsfile << "-1" << std::endl;
    }

    statsfile.close();
}

template std::pair<int, int> getStats(std::vector<int> & arr);
template std::pair<double, double> getStats(std::vector<double> & arr);

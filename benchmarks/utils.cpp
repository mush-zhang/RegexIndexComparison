#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string.h>
#include <filesystem>

#include "../src/BEST/Index/parallelizable.hpp"
#include "../src/simple_query_matcher.hpp"

#include "../src/FREE/Index/multigram_index.hpp"
#include "../src/FREE/Index/presuf_shell.hpp"
#include "../src/FREE/Matcher/query_matcher.hpp"

#include "utils.hpp"

inline constexpr const char * kTrafficRegex = "data/regexes_traffic.txt";
inline constexpr const char * kDbxRegex = "data/regexes_dbx.txt";
inline constexpr const char * kSysyRegex = "data/regexes_sysy.txt";

inline constexpr const std::string_view kIndexHeader = "\
    name,num_threads,gram_size,selectivity,selection_time,build_time,overall_time,num_keys,index_size,compile_time,match_time";

selection_type get_method(const std::string gs) {
    if (gs == "REI") {
        return selection_type::kRei;
    }
    if (gs == "FREE") {
        return selection_type::kFree;
    }
    if (gs == "BEST") {
        return selection_type::kBest;
    }
    if (gs == "FAST") {
        return selection_type::kFast;
    }
    return selection_type::kInvalid;
}

int error_return(const std::string msg) {
    std::cerr << "Error: " << msg << std::endl;
    std::cerr << kUsage << std::endl;
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
             expr_info & expr_info, rei_info & rei_info, 
             free_info & free_info, best_info & best_info, 
             fast_info & fast_info) {

    if (argc < 8) {
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
        if (expr_info.wl < 0 || expr_info.wl > 3) {
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
        case selection_type::kRei: {
            rei_info.num_repeat = rep;
            rei_info.num_threads = thread_count;
            if (n > 1) {
                return error_return("Missing/Invalid size n of n-gram.");
            }
            rei_info.gram_size = n;
            auto gram_num_string = getCmdOption(argv, argv + argc, "-k");
            if (gram_num_string.empty()) {
                return error_return("Missing number of grams.");
            } else {
                rei_info.num_grams = std::stoi(gram_num_string);
            }
            break;
        }
        case selection_type::kFree: {
            free_info.num_repeat = rep;
            free_info.num_threads = thread_count;
            if (n == 0) {
                return error_return("Missing/Invalid upper bound on n of n-gram.");
            }
            free_info.upper_k = n;
            if (selec > 0 && selec <= 1) {
                free_info.sel_threshold = selec;
            } 
            free_info.use_presuf = cmdOptionExists(argv, argv + argc, "--presuf");
            break;
        }
        case selection_type::kBest: {
            best_info.num_repeat = rep;
            best_info.num_threads = thread_count;
            if (selec > 0 && selec <= 1) {
                best_info.sel_threshold = selec;
            } 
            auto wl_reduce_string = getCmdOption(argv, argv + argc, "--wl_reduce");
            if (!wl_reduce_string.empty()) {
                if (wl_reduce_string.find('.') != std::string::npos) {
                    best_info.wl_reduced_frac = std::stod(wl_reduce_string);
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
            fast_info.num_repeat = rep;
            fast_info.num_threads = thread_count;
            auto relax_string = getCmdOption(argv, argv + argc, "--relax");
            if (relax_string.empty()) {
                return error_return("Missing type of relaxation.");
            } else if (relax_string == "DETERM") {
                fast_info.rtype = fast_index::relaxation_type::kDeterministic;
            } else if (relax_string == "RANDOM") {
                fast_info.rtype = fast_index::relaxation_type::kRandomized;
            } else {
                return error_return("Invalid relaxation type.");
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

std::vector<std::string> read_dbx(int max_lines=-1) {
    std::string line;

    std::vector<std::string> lines;
    std::string path = "data/extracted";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        std::string data_file = entry.path();
        std::ifstream data_in(data_file);
        if (!data_in.is_open()) {
            std::cerr << "Could not open the file - '" << data_file << "'" << std::endl;
            return lines;
        }
        //  101876733 
        while (getline(data_in, line)){
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (line.size() > 2) {
                lines.push_back(line);
                if (max_lines > 0 && lines.size() > max_lines) {
                    data_in.close();
                    return lines;
                }
            }
        }
        data_in.close();
    }
    return lines;
}

std::vector<std::string> read_sysy(int max_lines=-1) {
    std::string line;
    std::vector<std::string> lines;
    std::string data_file = "data/tagged_data.csv";

    std::ifstream data_in(data_file);
    if (!data_in.is_open()) {
        std::cerr << "Could not open the file - '" << data_file << "'" << std::endl;
        return lines;
    }
    while (getline(data_in, line)){
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::stringstream streamData(line);
        std::vector<std::string> curr_line;
        std::string s; 
        while (getline(streamData, s, '\t')) {
            curr_line.push_back(s);
        }
        curr_line.resize(curr_line.size()-4);

        std::string curr;
        for (const auto &piece : curr_line) curr += piece;

        lines.push_back(curr);
        if (max_lines > 0 && lines.size() > max_lines) {
            break;
        }
    }
    data_in.close();
    return lines;
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

int readWorkload(const expr_info & expr_info, 
                 std::vector<std::string> & regexes, 
                 std::vector<std::string> & lines,
                 int max_lines) {
    switch (expr_info.wl) {
        case 1: 
            regexes = read_file("regex", kTrafficRegex);
            lines = read_traffic(max_lines);
            break;
        case 2:
            regexes = read_file("regex", kDbxRegex);
            lines = read_dbx(max_lines);
            break;
        case 3:
            regexes = read_file("regex", kSysyRegex);
            lines = read_sysy(max_lines);
            break;
        default:
            regexes = read_file("regex", expr_info.reg_file);
            lines = read_file("data", expr_info.data_file, max_lines);
    }
    if (regexes.empty() || lines.empty()) {
        return EXIT_FAILURE;
    }
    std::cout << "read workload end." << std::endl;
    std::cout << "Number of regexes: " << regexes.size() << "."<< std::endl;
    std::cout << "Number of data lines: " << lines.size() << "." << std::endl;
    return EXIT_SUCCESS;
}

void run_end_to_end(const std::vector<std::string> & regexes, 
                    const std::vector<std::string> & lines) {
    std::vector<double> threshs({0.1, 0.3, 0.6, 0.8, 1});
    for (double t : threshs) {
        std::cout << "Start with threshold = " << t << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        auto pi = best_index::ParallelizableIndex(lines, regexes, t);
        pi.build_index();
        pi.print_index(true);
        auto matcher = SimpleQueryMatcher(pi);
        matcher.match_all();
        std::cout << "--------------------------------------" << std::endl;
    }

    // std::cout << "Start with FAST" << std::endl;
    // std::cout << "--------------------------------------" << std::endl;
    // auto pi = fast_index::LpmsIndex(lines, regexes);
    // pi.build_index();
    // std::cout << "------print index------------------------------" << std::endl;
    // pi.print_index(true);
    // std::cout << "--------------------------------------" << std::endl;
    // auto matcher = SimpleQueryMatcher(pi);
    // matcher.match_all();
    // std::cout << "--------------------------------------" << std::endl;

    // double threshold = 0.3;
    // std::vector<size_t> upper_k({3, 5, 7, 10});
    // for (size_t t : upper_k) {
    //     std::cout << "Start with max_k = " << t << std::endl;
    //     std::cout << "--------------------------------------" << std::endl;
    //     auto pi = free_index::MultigramIndex(lines, threshold);
    //     pi.build_index(t);
    //     // pi.print_index(true);
    //     auto matcher = free_index::QueryMatcher(pi, regexes);
    //     matcher.match_all();
    //     std::cout << "--------------------------------------" << std::endl;
    // }


}

int benchmark(const expr_info & expr_info, 
               const rei_info & rei_info, const free_info & free_info, 
               const best_info & best_info, const fast_info & fast_info) {
    // Create overall result folder
    const std::filesystem::path dir_path = expr_info.out_dir;
    if (!std::filesystem::exists(dir_path)) {
        std::filesystem::create_directory(dir_path);
    }

    // index building file
    std::filesystem::path out_path = dir_path / "summary.csv";
    std::ofstream outfile;
    if (!std::filesystem::exists(out_path)) {
        // write header
        outfile.open(out_path, std::ios::out);
        outfile << kIndexHeader << std::endl;
    } else {
        outfile.open(out_path, std::ios::app);
    }
    

    outfile.close();
    return EXIT_SUCCESS;
}
void benchmarkRei(std::ofstream & outfile, 
                  const std::vector<std::string> regexes, 
                  const std::vector<std::string> lines,
                  const rei_info & rei_info) {
    // get index building time
}

void benchmarkFree(std::ofstream & outfile, 
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const free_info & free_info) {
    // index building
    free_index::MultigramIndex * pi = nullptr;
    if (free_info.use_presuf) {
        pi = new free_index::PresufShell(lines, free_info.sel_threshold);
    } else {
        pi = new free_index::MultigramIndex(lines, free_info.sel_threshold);
    }
    pi->set_outfile(outfile);
    pi->build_index(free_info.upper_k);

    // matching; add match time to the overall file
    auto matcher = free_index::QueryMatcher(*pi, regexes);
    matcher.match_all();


    // double threshold = 0.3;
    // std::vector<size_t> upper_k({3, 5, 7, 10});
    // for (size_t t : upper_k) {
    //     std::cout << "Start with max_k = " << t << std::endl;
    //     std::cout << "--------------------------------------" << std::endl;
    //     pi.build_index(t);
    //     // pi.print_index(true);
    //     auto matcher = free_index::QueryMatcher(pi, regexes);
    //     matcher.match_all();
    //     std::cout << "--------------------------------------" << std::endl;
    // }
}

void benchmarkBest(std::ofstream & outfile, 
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const best_info & best_info) {}

void benchmarkFast(std::ofstream & outfile, 
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const fast_info & fast_info) {}

template std::pair<int, int> getStats(std::vector<int> & arr);
template std::pair<double, double> getStats(std::vector<double> & arr);
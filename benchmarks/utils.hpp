#ifndef BENCHMARKS_UTILS
#define BENCHMARKS_UTILS

#include <vector>
#include <string>

#include "../src/BEST/Index/single_threaded.hpp"
//#include "../src/FAST/Index/lpms.hpp"

inline constexpr std::string_view kHeader = "regex\ttime(s)\tnum_match";

inline constexpr std::string_view kUsage = "usage:  \n\
    ./benchmark gram_selection -t num_thread -r input_regex_file -d input_data_file -o output_file [options] \n\
    \t gram_selection: \t Required first argument. Name of the gram selection strategy. \n\
    \t                 \t Options available are 'FAST', 'BEST', 'FREE', 'REI'. \n\
      general options:\n\
    \t -t [int], required \t Number of threads for gram selection. \n\
    \t -w [0|1|2|3], required \t Workload used. \n\
    \t                        \t 1 for US-Accident workload; \n\
    \t                        \t 2 for DB-X workload; \n\
    \t                        \t 3 for Sys-Y workload; \n\
    \t                        \t 0 for customized workload (path to regex and data files required). \n\
    \t -r [path] \t Path to the list of regex queries. \n\
    \t           \t Each line of the file is considered a regex query. \n\
    \t -d [path] \t Path to the list of data to be queried upon. \n\
    \t           \t Each line of the file is considered an individual (log) line. \n\
    \t -o [path], required \t Path to directory holding all output files.\n\
    \t -e [int] \t Number of experiment repeat runs; default to 10.\n\
    \t -c [double] \t Selectivity threshold t; prune grams whose occurance is larger than t.\n\
    \t             \t The default is 0.1 for FREE and for BEST, optional for REI, and not applicable to FAST.\n\
      REI specific options:\n\
    \t -n, required \t Length of n-grams used for indexing.\n\
    \t -k, required \t Number of n-grams used for indexing. \n\
      FREE specific options:\n\
    \t -n [int], required \t Upper bound of multi-gram size.\n\
    \t --presuf \t Use presuf shell to generate a gram set that is also suffix-free; default not used.\n\
      BEST specific options:\n\
    \t --wl_reduce [int|double] \t Reduce the workload size for gram selection in BEST.\n\
    \t                          \t The value can be a fraction of the whole dataset, or an exact number.\n\
    \t --dist [1|2|3] \t Type of distance measurement used in workload reduction and clustering. \n\
    \t                \t Default to 2. \n\
      FAST specific options:\n\
    \t --relax [DETERM|RANDOM], required \t Type of relaxation method.";
/*-------------------------------------------------------------------------------------------------------------------*/

enum selection_type { kFast, kBest, kFree, kRei, kInvalid };

struct expr_info {
    selection_type stype;
    int wl;
    std::string reg_file = "";
    std::string data_file = "";
    std::string out_dir;
};

struct rei_info {
    int num_repeat = 10;
    int num_threads;
    double sel_threshold;
    int gram_size; // n
    int num_grams; // k
};

struct free_info {
    int num_repeat = 10;
    int num_threads;
    double sel_threshold = 0.1;
    int upper_n; // k
    bool use_presuf = false;
};

struct best_info {
    int num_repeat = 10;
    int num_threads;
    double sel_threshold = 0.1;
    int wl_reduced_size = -1;
    double wl_reduced_frac = -1;
    best_index::dist_type dtype = best_index::dist_type::kMaxDevDist2;
};

struct fast_info {
    int num_repeat = 10;
    int num_threads;
//    fast_index::relaxation_type rtype;
};

std::string getCmdOption(char ** begin, char ** end, const std::string & option);

bool cmdOptionExists(char ** begin, char ** end, const std::string & option);

template<class T>
std::pair<T, T> getStats(std::vector<T> & arr);

int parseArgs(int argc, char ** argv, 
             expr_info & expr_info, rei_info & rei_info, 
             free_info & free_info, best_info & best_info, 
             fast_info & fast_info);

int readWorkload(const expr_info & expr_info, 
                 std::vector<std::string> & regexes, 
                 std::vector<std::string> & lines,
                 int max_lines=-1);

void benchmarkRei(const std::filesystem::path dir_path,
                  const std::vector<std::string> regexes, 
                  const std::vector<std::string> lines,
                  const rei_info & rei_info);

void benchmarkFree(const std::filesystem::path dir_path, 
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const free_info & free_info);

void benchmarkBest(const std::filesystem::path dir_path,
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const best_info & best_info);

void benchmarkFast(const std::filesystem::path dir_path,
                   const std::vector<std::string> regexes, 
                   const std::vector<std::string> lines,
                   const fast_info & fast_info);

#endif // BENCHMARKS_UTILS

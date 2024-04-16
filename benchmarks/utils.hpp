#ifndef BENCHMARKS_UTILS
#define BENCHMARKS_UTILS

#include <vector>
#include <string>

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

char * getCmdOption(char ** begin, char ** end, const std::string & option);

bool cmdOptionExists(char** begin, char** end, const std::string& option);

template<class T>
std::pair<T, T> getStats(std::vector<T> & arr);

std::vector<std::string> readTraffic();

int parseArgs(int argc, char** argv, int * num_repeat, 
              std::string * input_regex_file, 
              std::string * input_data_file);

std::vector<std::string> readDataIn(const std::string & file_type, 
                                    const std::string & infile_name);

void run_end_to_end(const std::vector<std::string> & regexes, 
                    const std::vector<std::string> & lines);

#endif // BENCHMARKS_UTILS
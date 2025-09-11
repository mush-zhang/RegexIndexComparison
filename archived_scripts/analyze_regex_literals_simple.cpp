#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <map>
#include <filesystem>

// Include the regex utilities directly
#include "src/utils/reg_utils.hpp"

struct LiteralStats {
    std::string regex;
    std::vector<std::string> literals;
    int total_literal_chars;
    int num_literals;
    double avg_literal_length;
};

struct WorkloadStats {
    std::string workload_name;
    std::vector<LiteralStats> regex_stats;
    double avg_literals_per_regex;
    double avg_literal_chars_per_regex;
    double avg_literal_length;
    int total_regexes;
    int total_literals;
    int total_literal_chars;
};

// Simple file reading function
std::vector<std::string> read_regex_file(const std::string& filename) {
    std::vector<std::string> regexes;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return regexes;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            regexes.push_back(line);
        }
    }
    file.close();
    return regexes;
}

void print_usage() {
    std::cout << "Usage: ./analyze_regex_literals_simple [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  -f <file>       Regex file to analyze (required)\n";
    std::cout << "  -o <file>       Output CSV file (optional)\n";
    std::cout << "  -v              Verbose output\n";
    std::cout << "  -h              Show this help\n";
}

LiteralStats analyze_regex_literals(const std::string& regex) {
    LiteralStats stats;
    stats.regex = regex;
    stats.literals = extract_literals(regex);
    stats.num_literals = stats.literals.size();
    stats.total_literal_chars = 0;
    
    for (const auto& literal : stats.literals) {
        stats.total_literal_chars += literal.length();
    }
    
    stats.avg_literal_length = stats.num_literals > 0 ? 
        static_cast<double>(stats.total_literal_chars) / stats.num_literals : 0.0;
    
    return stats;
}

WorkloadStats analyze_workload(const std::vector<std::string>& regexes, const std::string& workload_name) {
    WorkloadStats workload_stats;
    workload_stats.workload_name = workload_name;
    workload_stats.total_regexes = regexes.size();
    workload_stats.total_literals = 0;
    workload_stats.total_literal_chars = 0;
    
    std::cout << "Analyzing " << regexes.size() << " regexes..." << std::endl;
    
    for (size_t i = 0; i < regexes.size(); i++) {
        if (i % 100 == 0) {
            std::cout << "Progress: " << i << "/" << regexes.size() << " (" 
                      << std::fixed << std::setprecision(1) 
                      << (100.0 * i / regexes.size()) << "%)" << std::endl;
        }
        
        LiteralStats stats = analyze_regex_literals(regexes[i]);
        workload_stats.regex_stats.push_back(stats);
        workload_stats.total_literals += stats.num_literals;
        workload_stats.total_literal_chars += stats.total_literal_chars;
    }
    
    // Calculate averages
    workload_stats.avg_literals_per_regex = workload_stats.total_regexes > 0 ?
        static_cast<double>(workload_stats.total_literals) / workload_stats.total_regexes : 0.0;
    
    workload_stats.avg_literal_chars_per_regex = workload_stats.total_regexes > 0 ?
        static_cast<double>(workload_stats.total_literal_chars) / workload_stats.total_regexes : 0.0;
    
    workload_stats.avg_literal_length = workload_stats.total_literals > 0 ?
        static_cast<double>(workload_stats.total_literal_chars) / workload_stats.total_literals : 0.0;
    
    return workload_stats;
}

void print_stats(const WorkloadStats& stats, bool verbose) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "REGEX LITERAL ANALYSIS RESULTS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "File: " << stats.workload_name << std::endl;
    std::cout << "Total Regexes: " << stats.total_regexes << std::endl;
    std::cout << "Total Literals: " << stats.total_literals << std::endl;
    std::cout << "Total Literal Characters: " << stats.total_literal_chars << std::endl;
    std::cout << std::endl;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average literals per regex: " << stats.avg_literals_per_regex << std::endl;
    std::cout << "Average literal characters per regex: " << stats.avg_literal_chars_per_regex << std::endl;
    std::cout << "Average literal length: " << stats.avg_literal_length << std::endl;
    std::cout << std::endl;
    
    // Distribution analysis
    std::map<int, int> literal_count_dist;
    std::map<int, int> literal_chars_dist;
    
    for (const auto& regex_stat : stats.regex_stats) {
        literal_count_dist[regex_stat.num_literals]++;
        // Group literal chars into bins of 10
        int chars_bin = (regex_stat.total_literal_chars / 10) * 10;
        literal_chars_dist[chars_bin]++;
    }
    
    std::cout << "Distribution of literals per regex:" << std::endl;
    for (const auto& [count, freq] : literal_count_dist) {
        std::cout << "  " << count << " literals: " << freq << " regexes ("
                  << std::fixed << std::setprecision(1)
                  << (100.0 * freq / stats.total_regexes) << "%)" << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "Distribution of literal characters per regex (binned by 10s):" << std::endl;
    for (const auto& [chars, freq] : literal_chars_dist) {
        std::cout << "  " << chars << "-" << (chars + 9) << " chars: " << freq << " regexes ("
                  << std::fixed << std::setprecision(1)
                  << (100.0 * freq / stats.total_regexes) << "%)" << std::endl;
    }
    
    if (verbose) {
        std::cout << "\n" << std::string(60, '-') << std::endl;
        std::cout << "DETAILED REGEX ANALYSIS" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        std::cout << std::left << std::setw(5) << "ID" 
                  << std::setw(8) << "Literals" 
                  << std::setw(8) << "Chars"
                  << std::setw(8) << "AvgLen"
                  << "Regex" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        for (size_t i = 0; i < std::min(size_t(50), stats.regex_stats.size()); i++) {
            const auto& rs = stats.regex_stats[i];
            std::cout << std::left << std::setw(5) << i + 1
                      << std::setw(8) << rs.num_literals
                      << std::setw(8) << rs.total_literal_chars
                      << std::setw(8) << std::fixed << std::setprecision(1) << rs.avg_literal_length
                      << rs.regex.substr(0, 40);
            if (rs.regex.length() > 40) std::cout << "...";
            std::cout << std::endl;
            
            if (verbose && !rs.literals.empty()) {
                std::cout << "    Literals: ";
                for (size_t j = 0; j < rs.literals.size(); j++) {
                    if (j > 0) std::cout << ", ";
                    std::cout << "\"" << rs.literals[j] << "\"";
                }
                std::cout << std::endl;
            }
        }
        
        if (stats.regex_stats.size() > 50) {
            std::cout << "... (" << (stats.regex_stats.size() - 50) << " more regexes)" << std::endl;
        }
    }
}

void save_csv(const WorkloadStats& stats, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file " << filename << std::endl;
        return;
    }
    
    // Write header
    file << "regex_id,regex,num_literals,total_literal_chars,avg_literal_length,literals\n";
    
    // Write data
    for (size_t i = 0; i < stats.regex_stats.size(); i++) {
        const auto& rs = stats.regex_stats[i];
        file << (i + 1) << ",\"" << rs.regex << "\"," 
             << rs.num_literals << "," << rs.total_literal_chars << ","
             << std::fixed << std::setprecision(3) << rs.avg_literal_length << ",\"";
        
        for (size_t j = 0; j < rs.literals.size(); j++) {
            if (j > 0) file << ";";
            file << rs.literals[j];
        }
        file << "\"\n";
    }
    
    file.close();
    std::cout << "Results saved to: " << filename << std::endl;
}

int main(int argc, char* argv[]) {
    std::string regex_file;
    std::string output_file;
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage();
            return 0;
        } else if (arg == "-f" && i + 1 < argc) {
            regex_file = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "-v") {
            verbose = true;
        }
    }
    
    if (regex_file.empty()) {
        std::cerr << "Error: Regex file (-f) is required" << std::endl;
        print_usage();
        return 1;
    }
    
    // Read regexes
    std::vector<std::string> regexes = read_regex_file(regex_file);
    if (regexes.empty()) {
        std::cerr << "Error: Could not read regexes from " << regex_file << std::endl;
        return 1;
    }
    
    std::string workload_name = std::filesystem::path(regex_file).stem().string();
    
    // Analyze literals
    WorkloadStats stats = analyze_workload(regexes, workload_name);
    
    // Print results
    print_stats(stats, verbose);
    
    // Save CSV if requested
    if (!output_file.empty()) {
        save_csv(stats, output_file);
    }
    
    return 0;
}

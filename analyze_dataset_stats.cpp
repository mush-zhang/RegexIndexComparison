#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <set>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <map>

struct DatasetStats {
    std::string dataset_name;
    size_t total_lines;
    size_t total_characters;
    size_t alphabet_size;
    double avg_line_length;
    size_t min_line_length;
    size_t max_line_length;
    std::set<char> alphabet;
    std::map<char, size_t> char_frequency;
    std::vector<size_t> line_lengths;
};

// Function to read all lines from a single file
std::vector<std::string> read_file_lines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return lines; // Return empty vector if file can't be opened
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    
    file.close();
    return lines;
}

// Function to read a single file as one string (like read_enron method)
std::string read_file_as_string(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return ""; // Return empty string if file can't be opened
    }
    
    // Read entire file content as one string, preserving newlines
    std::string file_content;
    std::string line;
    bool first_line = true;
    
    while (std::getline(file, line)) {
        if (!first_line) {
            file_content += "\n";
        }
        // Remove carriage returns but keep the content
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        file_content += line;
        first_line = false;
    }
    
    file.close();
    return file_content;
}

// Function to read all lines from a directory recursively (like read_directory in utils.cpp)
std::vector<std::string> read_directory_lines(const std::string& path, int max_lines = -1) {
    std::vector<std::string> all_lines;
    
    if (!std::filesystem::exists(path)) {
        std::cerr << "Error: Directory " << path << " does not exist" << std::endl;
        return all_lines;
    }
    
    std::cout << "Reading files from directory: " << path << std::endl;
    
    size_t file_count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            file_count++;
            if (file_count % 100 == 0) {
                std::cout << "Processed " << file_count << " files, " 
                          << all_lines.size() << " lines so far..." << std::endl;
            }
            
            std::vector<std::string> file_lines = read_file_lines(entry.path());
            all_lines.insert(all_lines.end(), file_lines.begin(), file_lines.end());
            
            if (max_lines > 0 && static_cast<int>(all_lines.size()) >= max_lines) {
                all_lines.resize(max_lines);
                break;
            }
        }
    }
    
    std::cout << "Finished reading. Total files: " << file_count 
              << ", Total lines: " << all_lines.size() << std::endl;
    
    return all_lines;
}

// Function to read all files from a directory as individual strings (like read_enron method)
std::vector<std::string> read_directory_as_strings(const std::string& path, int max_files = -1) {
    std::vector<std::string> all_strings;
    
    if (!std::filesystem::exists(path)) {
        std::cerr << "Error: Directory " << path << " does not exist" << std::endl;
        return all_strings;
    }
    
    std::cout << "Reading files from directory as individual strings: " << path << std::endl;
    
    size_t file_count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            file_count++;
            if (file_count % 100 == 0) {
                std::cout << "Processed " << file_count << " files, " 
                          << all_strings.size() << " documents so far..." << std::endl;
            }
            
            std::string file_content = read_file_as_string(entry.path());
            if (!file_content.empty()) {
                all_strings.push_back(file_content);
            }
            
            if (max_files > 0 && static_cast<int>(all_strings.size()) >= max_files) {
                break;
            }
        }
    }
    
    std::cout << "Finished reading. Total files: " << file_count 
              << ", Total documents: " << all_strings.size() << std::endl;
    
    return all_strings;
}

// Function to analyze dataset statistics
DatasetStats analyze_dataset(const std::vector<std::string>& lines, const std::string& dataset_name) {
    DatasetStats stats;
    stats.dataset_name = dataset_name;
    stats.total_lines = lines.size();
    stats.total_characters = 0;
    stats.min_line_length = SIZE_MAX;
    stats.max_line_length = 0;
    
    std::cout << "Analyzing " << lines.size() << " lines..." << std::endl;
    
    for (size_t i = 0; i < lines.size(); i++) {
        if (i % 10000 == 0) {
            std::cout << "Progress: " << i << "/" << lines.size() << " (" 
                      << std::fixed << std::setprecision(1) 
                      << (100.0 * i / lines.size()) << "%)" << std::endl;
        }
        
        const std::string& line = lines[i];
        size_t line_length = line.length();
        
        stats.total_characters += line_length;
        stats.line_lengths.push_back(line_length);
        
        if (line_length < stats.min_line_length) {
            stats.min_line_length = line_length;
        }
        if (line_length > stats.max_line_length) {
            stats.max_line_length = line_length;
        }
        
        // Count characters and build alphabet
        for (char c : line) {
            stats.alphabet.insert(c);
            stats.char_frequency[c]++;
        }
    }
    
    if (stats.min_line_length == SIZE_MAX) {
        stats.min_line_length = 0;
    }
    
    stats.alphabet_size = stats.alphabet.size();
    stats.avg_line_length = stats.total_lines > 0 ? 
        static_cast<double>(stats.total_characters) / stats.total_lines : 0.0;
    
    return stats;
}

// Function to print detailed statistics
void print_detailed_stats(const DatasetStats& stats) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "DATASET STATISTICS: " << stats.dataset_name << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    std::cout << "Basic Statistics:" << std::endl;
    std::cout << "  Total lines: " << stats.total_lines << std::endl;
    std::cout << "  Total characters: " << stats.total_characters << std::endl;
    std::cout << "  Alphabet size: " << stats.alphabet_size << " distinct characters" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Average line length: " << stats.avg_line_length << " characters" << std::endl;
    std::cout << "  Minimum line length: " << stats.min_line_length << " characters" << std::endl;
    std::cout << "  Maximum line length: " << stats.max_line_length << " characters" << std::endl;
    std::cout << std::endl;
    
    // Length distribution
    if (!stats.line_lengths.empty()) {
        std::vector<size_t> lengths = stats.line_lengths;
        std::sort(lengths.begin(), lengths.end());
        
        std::cout << "Line Length Distribution:" << std::endl;
        std::cout << "  Median: " << lengths[lengths.size() / 2] << std::endl;
        std::cout << "  25th percentile: " << lengths[lengths.size() / 4] << std::endl;
        std::cout << "  75th percentile: " << lengths[3 * lengths.size() / 4] << std::endl;
        std::cout << "  90th percentile: " << lengths[9 * lengths.size() / 10] << std::endl;
        std::cout << "  95th percentile: " << lengths[95 * lengths.size() / 100] << std::endl;
        std::cout << std::endl;
    }
    
    // Character frequency analysis
    std::cout << "Character Alphabet Analysis:" << std::endl;
    std::cout << "  Printable ASCII characters: ";
    size_t printable_count = 0;
    for (char c : stats.alphabet) {
        if (c >= 32 && c <= 126) { // Printable ASCII range
            printable_count++;
        }
    }
    std::cout << printable_count << std::endl;
    
    std::cout << "  Control characters: " << (stats.alphabet_size - printable_count) << std::endl;
    
    // Show alphabet characters (first 50 printable ones)
    std::cout << "  Sample alphabet characters: ";
    size_t shown = 0;
    for (char c : stats.alphabet) {
        if (c >= 32 && c <= 126 && shown < 50) {
            std::cout << "'" << c << "' ";
            shown++;
        }
    }
    if (stats.alphabet.size() > 50) {
        std::cout << "... (" << (stats.alphabet.size() - 50) << " more)";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    
    // Most frequent characters
    std::vector<std::pair<size_t, char>> char_freq_vec;
    for (const auto& [c, freq] : stats.char_frequency) {
        char_freq_vec.emplace_back(freq, c);
    }
    std::sort(char_freq_vec.rbegin(), char_freq_vec.rend());
    
    std::cout << "Most Frequent Characters:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(10), char_freq_vec.size()); i++) {
        char c = char_freq_vec[i].second;
        size_t freq = char_freq_vec[i].first;
        double pct = (100.0 * freq) / stats.total_characters;
        
        std::cout << "  ";
        if (c == ' ') {
            std::cout << "SPACE";
        } else if (c == '\t') {
            std::cout << "TAB";
        } else if (c == '\n') {
            std::cout << "NEWLINE";
        } else if (c >= 32 && c <= 126) {
            std::cout << "'" << c << "'";
        } else {
            std::cout << "CTRL-" << static_cast<int>(c);
        }
        std::cout << ": " << freq << " occurrences (" 
                  << std::fixed << std::setprecision(2) << pct << "%)" << std::endl;
    }
}

// Function to save statistics to CSV
void save_stats_csv(const DatasetStats& stats, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file " << filename << std::endl;
        return;
    }
    
    file << "dataset_name,total_lines,total_characters,alphabet_size,avg_line_length,";
    file << "min_line_length,max_line_length,median_line_length\n";
    
    // Calculate median
    std::vector<size_t> lengths = stats.line_lengths;
    std::sort(lengths.begin(), lengths.end());
    size_t median = lengths.empty() ? 0 : lengths[lengths.size() / 2];
    
    file << stats.dataset_name << "," << stats.total_lines << "," << stats.total_characters << ",";
    file << stats.alphabet_size << "," << std::fixed << std::setprecision(3) << stats.avg_line_length << ",";
    file << stats.min_line_length << "," << stats.max_line_length << "," << median << "\n";
    
    file.close();
    std::cout << "Statistics saved to: " << filename << std::endl;
}

void print_usage() {
    std::cout << "Usage: ./analyze_dataset_stats [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  -d <dir>        Directory to analyze (default: data/enron/maildir)\n";
    std::cout << "  -f <file>       Single file to analyze\n";
    std::cout << "  -n <name>       Dataset name (default: from directory name)\n";
    std::cout << "  -o <file>       Output CSV file (optional)\n";
    std::cout << "  -m <max_lines>  Maximum lines to process (default: no limit)\n";
    std::cout << "  --enron         Use Enron-style reading (each file as one document)\n";
    std::cout << "  --sysy          Analyze Sysy dataset (reads data/tagged_data.csv)\n";
    std::cout << "  -h              Show this help\n";
}

int main(int argc, char* argv[]) {
    std::string data_path = "data/enron/maildir";
    std::string single_file;
    std::string dataset_name;
    std::string output_file;
    int max_lines = -1;
    bool use_enron_reading = false;
    bool is_sysy = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: ./analyze_dataset_stats [OPTIONS]\n";
            std::cout << "Options:\n";
            std::cout << "  -d <dir>        Directory to analyze (default: data/enron/maildir)\n";
            std::cout << "  -f <file>       Single file to analyze\n";
            std::cout << "  -n <name>       Dataset name (default: from directory name)\n";
            std::cout << "  -o <file>       Output CSV file (optional)\n";
            std::cout << "  -m <max_lines>  Maximum lines to process (default: no limit)\n";
            std::cout << "  --enron         Use Enron-style reading (each file as one document)\n";
            std::cout << "  --sysy          Analyze Sysy dataset (line-by-line reading)\n";
            std::cout << "  -h              Show this help\n";
            return 0;
        } else if (arg == "-d" && i + 1 < argc) {
            data_path = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            single_file = argv[++i];
        } else if (arg == "-n" && i + 1 < argc) {
            dataset_name = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "-m" && i + 1 < argc) {
            max_lines = std::stoi(argv[++i]);
        } else if (arg == "--enron") {
            use_enron_reading = true;
        } else if (arg == "--sysy") {
            use_enron_reading = false;  // Sysy uses line-by-line reading
            is_sysy = true;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    // Determine dataset name if not provided
    if (dataset_name.empty()) {
        if (!single_file.empty()) {
            dataset_name = std::filesystem::path(single_file).stem().string();
        } else {
            dataset_name = std::filesystem::path(data_path).filename().string();
        }
    }
    
    // Auto-detect Enron reading if analyzing enron directory
    if (!use_enron_reading && data_path.find("enron") != std::string::npos) {
        use_enron_reading = true;
        std::cout << "Auto-detected Enron dataset, using file-per-document reading mode." << std::endl;
    }
    
    // Override for Sysy dataset - use specific file
    if (is_sysy) {
        data_path = "data/tagged_data.csv";
        std::cout << "Sysy dataset detected, using single file: " << data_path << std::endl;
    }
    
    // Read data
    std::vector<std::string> data_to_analyze;
    
    if (!single_file.empty()) {
        std::cout << "Analyzing single file: " << single_file << std::endl;
        if (use_enron_reading) {
            std::string content = read_file_as_string(single_file);
            if (!content.empty()) {
                data_to_analyze.push_back(content);
            }
        } else {
            data_to_analyze = read_file_lines(single_file);
        }
    } else if (is_sysy || data_path.find(".csv") != std::string::npos) {
        // Handle single file (Sysy or CSV files)
        std::cout << "Analyzing single file: " << data_path << std::endl;
        if (use_enron_reading) {
            std::string content = read_file_as_string(data_path);
            if (!content.empty()) {
                data_to_analyze.push_back(content);
            }
        } else {
            data_to_analyze = read_file_lines(data_path);
        }
    } else {
        std::cout << "Analyzing directory: " << data_path << std::endl;
        if (use_enron_reading) {
            std::cout << "Using Enron-style reading (each file as one document)..." << std::endl;
            data_to_analyze = read_directory_as_strings(data_path, max_lines);
        } else {
            std::cout << "Using line-by-line reading..." << std::endl;
            data_to_analyze = read_directory_lines(data_path, max_lines);
        }
    }
    
    if (data_to_analyze.empty()) {
        std::cerr << "Error: No data found to analyze" << std::endl;
        return 1;
    }
    
    // Analyze statistics
    DatasetStats stats = analyze_dataset(data_to_analyze, dataset_name);
    
    // Print results
    print_detailed_stats(stats);
    
    // Save CSV if requested
    if (!output_file.empty()) {
        save_stats_csv(stats, output_file);
    }
    
    return 0;
}

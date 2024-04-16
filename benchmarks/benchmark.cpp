#include <iostream>
#include <fstream>

#include "utils.hpp"

int main(int argc, char** argv) {
    // argument -h for help
    if(cmdOptionExists(argv, argv+argc, "-h")) {
        std::cout << kUsage << std::endl;
        return EXIT_SUCCESS;
    }
    int num_repeat;
    std::string input_data_file, input_regex_file;

    int status = parseArgs(argc, argv, & num_repeat, & input_regex_file, & input_data_file);
    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    auto regexes = readDataIn("regex", input_regex_file);
    if (regexes.empty()) {
        return EXIT_FAILURE;
    }
    std::cout << "read regexes end" << std::endl;

    std::vector<std::string> lines;
    if (input_data_file.empty()) {
        lines = readTraffic();
        std::cout << "read traffic begin" << std::endl;
    } else {
        lines = readDataIn("data", input_data_file);
    }
    if (lines.empty()) {
        return EXIT_FAILURE;
    }
    std::cout << "read lines end" << std::endl;

    // std::ofstream r_file(argv[1], std::ofstream::out);
    // if (!r_file.is_open()) {
    //     std::cerr << "Could not open output file '" << argv[1] << "'" << std::endl;
    //     return EXIT_FAILURE;
    // }
    std::cout << "start best end-to-end" << std::endl;
    run_end_to_end(regexes, lines);

    // r_file.close(); 
}
#include <iostream>
#include <fstream>

#include "utils.hpp"

int main(int argc, char** argv) {
    // argument -h for help
    if(cmdOptionExists(argv, argv+argc, "-h")) {
        std::cout << kUsage << std::endl;
        return EXIT_SUCCESS;
    }
    
    expr_info expr_info;
    rei_info rei_info;
    free_info free_info;
    best_info best_info;
    fast_info fast_info;

    int status = parseArgs(argc, argv, expr_info, rei_info, free_info, best_info, fast_info);
    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    std::vector<std::string> regexes;
    std::vector<std::string> lines;
    status = readWorkload(expr_info, regexes, lines, 100000);
    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // std::ofstream r_file(expr_info.out_file, std::ofstream::out);
    // if (!r_file.is_open()) {
    //     std::cerr << "Could not open output file '" << expr_info.out_file << "'" << std::endl;
    //     return EXIT_FAILURE;
    // }

    std::cout << "start best end-to-end" << std::endl;
    run_end_to_end(regexes, lines);

    // r_file.close(); 
}
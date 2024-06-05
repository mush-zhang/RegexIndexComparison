#include <iostream>
#include <fstream>
#include <filesystem>

#include "utils.hpp"

int main(int argc, char** argv) {
    // argument -h for help
    if(cmdOptionExists(argv, argv+argc, "-h")) {
        std::cout << kUsage << std::endl;
        return EXIT_SUCCESS;
    }

    expr_info expr_info;
    free_info free_info;
    best_info best_info;
    fast_info fast_info;

    int status = parseArgs(argc, argv, expr_info, 
                           free_info, best_info, fast_info);
    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Create overall result folder
    const std::filesystem::path dir_path = expr_info.out_dir;
    if (!std::filesystem::exists(dir_path)) {
        std::filesystem::create_directory(dir_path);
    }

    std::vector<std::string> regexes;
    std::vector<std::string> lines;
#ifdef NDEBUG
    status = readWorkload(expr_info, regexes, lines, 100000);
#else
    status = readWorkload(expr_info, regexes, lines);
#endif

    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    std::cout << "start end-to-end" << std::endl;

    switch (expr_info.stype) {
        case selection_type::kFree: 
            benchmarkFree(dir_path, regexes, lines, free_info);
            break;
        case selection_type::kBest:
            benchmarkBest(dir_path, regexes, lines, best_info);
            break;
        case selection_type::kFast:
            benchmarkFast(dir_path, regexes, lines, fast_info);
            break;
        default:
            // should not have reached here.
            return EXIT_FAILURE;
    } 

    // start running! 

    // run_end_to_end(regexes, lines);
}
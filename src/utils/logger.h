class Logger {
    public:
        Logger() {};
        Logger(std::string index_file, std::string match_file) : on_(true) {};
    private:
        bool on_ = false;
        std::ofstream idx_outfile_;
        std::ofstream match_outfile_;
}
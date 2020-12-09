#ifndef QUERY_HPP
#define QUERY_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <tuple>
#include <set>

using bin_opt = std::tuple<std::string, char, std::string>;

class Query {
public:
    explicit Query(const std::string& raw_query);
    std::string get_command(void) const;
    std::string get_sort_by(void) const;
    std::string get_file_name(void) const;
    bool is_correct(void) const;
    bool have_unary_option(const std::string& option) const;
    std::vector<bin_opt> get_binary_options(void) const;
    std::string generate_request(const std::pair<time_t, time_t>& time, const std::string& id) const;
private:
    bool correct;
    std::string command;
    std::string sort_by;
    std::string filename;
    std::set<std::string> unary_option;
    std::vector<bin_opt> binary_option;

    std::string to_lower(const std::string& str) const;
    std::string erase_excess_ws(const std::string& str) const;
    std::vector<std::string> split_to_words(const std::string& str,
                                            const char splitter = ' ') const;
    bin_opt split_by_sign(const std::string& str) const;

    void set_command(const std::string& str);
    void set_sort_by(const std::string& str);
    void set_filaname(const std::string& str);
    void add_unary_option(const std::string& str);
    void add_binary_option(const bin_opt& option);
};

#endif //QUERY_HPP

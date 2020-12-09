#include "Query.hpp"

Query::Query(const std::string& raw_query) : correct {true} {
    std::string prepared_query = erase_excess_ws(to_lower(raw_query));
    bool infile = true;
    bool sortby = true;
    for (const auto& word : split_to_words(prepared_query)) {
        if (command.empty()) {
            set_command(word);
        } else if (have_unary_option("file") && infile) {
            set_filaname(word);
            infile = false;
        } else if (have_unary_option("sort-by") && sortby) {
            set_sort_by(word);
            sortby = false;
        } else if (word.at(0) == '-') {
            add_unary_option(word.substr(1));
        } else {
            add_binary_option(split_by_sign(word));
        }
    }
}

std::string Query::get_command() const {
    return command;
}

std::string Query::get_sort_by() const {
    return sort_by;
}

std::string Query::get_file_name() const {
    return filename;
}

bool Query::is_correct() const {
    return correct;
}

bool Query::have_unary_option(const std::string &option) const {
    return unary_option.count(option) != 0;
}

std::vector<bin_opt> Query::get_binary_options() const {
    return binary_option;
}

std::string Query::generate_request(const std::pair<time_t, time_t>& time, const std::string& id) const {
    std::string ret = id + "|list";
    ret.append(" start=" + std::to_string(time.first));
    ret.append(" end=" + std::to_string(time.second));
    ret.append(" \x1f");
    return ret;
}

//private section

std::string Query::to_lower(const std::string &str) const {
    std::string ret;
    std::transform(str.begin(), str.end(), std::back_inserter(ret),
                   [](const char c) {return std::tolower(c); });
    return ret;
}

std::string Query::erase_excess_ws(const std::string &str) const {
    std::string ret;
    bool ignore_next_ws = true;
    for (const auto& c : str) {
        if (c == ' ' && ignore_next_ws) {
            continue;
        } else if (c == ' ') {
            ret.push_back(c);
            ignore_next_ws = true;
        } else if (c == '=' || c == '<' || c == '>') {
            if (ret.back() == ' ') ret.pop_back();
            ret.push_back(c);
            ignore_next_ws = true;
        } else {
            ret.push_back(c);
            ignore_next_ws = false;
        }
    }
    if (ret.back() == ' ') ret.pop_back();
    return ret;
}

std::vector<std::string> Query::split_to_words(const std::string &str,
                                               const char splitter) const {
    std::vector<std::string> ret;
    std::string word;
    for (const auto& c : str) {
        if (c == splitter) {
            ret.push_back(word);
            word.clear();
        } else {
            word.push_back(c);
        }
    }
    ret.push_back(word);
    return ret;
}

bin_opt Query::split_by_sign(const std::string &str) const {
    char splitter;
    if      (str.find('=') != std::string::npos) splitter = '=';
    else if (str.find('<') != std::string::npos) splitter = '<';
    else if (str.find('>') != std::string::npos) splitter = '>';
    else    return {"", ' ', ""};
    std::vector<std::string> key_val = split_to_words(str, splitter);
    if (key_val.size() != 2) return {"", ' ', ""};
    return {key_val[0], splitter, key_val[1]};
}

void Query::set_command(const std::string &str) {
    if (str == "find" || str == "get") {
        command = str;
    } else {
        correct = false;
    }
}

void Query::set_sort_by(const std::string &str) {
    if (str == "date" || str == "type" || str == "subj" || str == "host" ||
            str == "result" || str == "id" || str == "grade") {
        sort_by = str;
    } else {
        correct = false;
    }
}

void Query::set_filaname(const std::string &str) {
    if(!filename.empty()) {
        filename = str;
    } else {
        correct = false;
    }
}

void Query::add_unary_option(const std::string &str) {
    if (str == "file" || str == "sort-by" || str == "full") {
        unary_option.insert(str);
    } else {
        correct = false;
    }
}

void Query::add_binary_option(const bin_opt &option) {
    auto [key, sign, val] = option;
    if (key.empty() || val.empty() || sign == ' ') {
        correct = false;
        return;
    }
    if (((key == "id" || key == "date" || key == "grade") &&
        (sign == '=' || sign == '<' || sign == '>'))
        ||
        ((key == "subj" || key == "host" || key == "result" || key == "type") &&
        (sign == '='))){
            binary_option.push_back(option);
    } else {
        correct = false;
    }
}


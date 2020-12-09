#include "Filter.hpp"

Filter::Filter(const Query& query) : correct{true}{
    reset();
    for (auto& [key, sign, val] : query.get_binary_options()) {
        if (key == "id") {
            if (!set_id(val, sign)) correct = false;
        } else if (key == "date") {
            if (!set_date(val, sign)) correct = false;
        } else if (key == "grade") {
            if (!set_grade(val, sign)) correct = false;
        } else if (key == "type") {
            set_type(val);
        } else if (key == "subj") {
            set_subj(val);
        } else if (key == "host") {
            set_host(val);
        } else if (key == "result") {
            set_result(val);
        } else {
            std::cout << "Unknown key\n";
        }
    }
    if (max_time == 0) time(&max_time);
}

bool Filter::check_filters(const LogLine& line) const {
    bool ret = true;
    bool ret_id, ret_date, ret_grade, ret_type, ret_subj, ret_host, ret_result;
    if (!line.is_correct() || !this->correct) return false;
    ret = ret && check_id(convert_to_int(line.get_field(LogLine::TYPE::ID)));
    //ret = ret && check_date(convert_to_time(line.get_field(LogLine::TYPE::TIME)));
    //ret = ret && check_grade(convert_to_grade(line.get_field(LogLine::TYPE::GRADE)));
    //if (!type.empty()) ret = ret && check_type(line.get_field(LogLine::TYPE::TYPE));
    //if (!subj.empty()) ret = ret && check_subj(line.get_field(LogLine::TYPE::SUBJ));
    //if (!host.empty()) ret = ret && check_host(line.get_field(LogLine::TYPE::HOST));
    //if (!result.empty()) ret = ret && check_result(line.get_field(LogLine::TYPE::RESULT));
    return ret;
}

void Filter::reset(void) {
    min_id = 0;
    max_id = 0;
    min_time = 0;
    time(&max_time);
    min_grade = GRADE::DEFAULT;
    max_grade = GRADE::RED;
    type.clear();
    subj.clear();
    host.clear();
    result.clear();
    correct = true;
}

bool Filter::is_correct() const {
    return correct;
}

std::pair<time_t, time_t> Filter::get_time() const {
    return {min_time, max_time};
}
//private section

bool Filter::set_id(const std::string& val, const char sign) {
    int value = convert_to_int(val);
    if (value == -1) return false;
    switch (sign) {
    case '=': min_id = max_id = value; break;
    case '>': min_id = value; break;
    case '<': max_id = value; break;
    default: return false;
    }
    return true;
}

bool Filter::set_date(const std::string& val, const char sign) {
    time_t value = convert_to_time(val);
    if (value == -1) {
        std::cerr << val << " is not valid date\n";
        return false;
    }
    switch (sign) {
    case '=': min_time = max_time = value; break;
    case '>': min_time = value; break;
    case '<': max_time = value; break;
    }
    return true;
}

bool Filter::set_grade(const std::string& val, const char sign) {
    GRADE value = convert_to_grade(val);
    if (value == GRADE::MAX_GRADES) return false;
    switch (sign) {
    case '=': min_grade = max_grade = value; break;
    case '>': min_grade = value;
    case '<': max_grade = value;
    }
    return true;
}

void Filter::set_type(const std::string& val) { type = val; }

void Filter::set_subj(const std::string& val) { subj = val; }

void Filter::set_host(const std::string& val) { host = val; }

void Filter::set_result(const std::string& val) { result = val; }

bool Filter::check_id(const int val) const {
    bool ret = true;
    if (max_id) ret = ret && val <= max_id;
    if (min_id) ret = ret && val >= min_id;
    return ret;
}

bool Filter::check_date(const time_t val) const {
    return (val <= max_time && val >= min_time);
}

bool Filter::check_grade(const GRADE val) const {
    return (val <= max_grade && val >= min_grade);
}

bool Filter::check_type(const std::string& val) const {
    return type.empty() ? true : type == val;
}

bool Filter::check_subj(const std::string& val) const {
    return subj.empty() ? true : subj == val;
}

bool Filter::check_host(const std::string& val) const {
    return host.empty() ? true : host == val;
}

bool Filter::check_result(const std::string& val) const {
    return result.empty() ? true : result == val;
}

time_t Filter::convert_to_time(const std::string& val) const {
    struct tm time_tm;
    memset(&time_tm, 0, sizeof(struct tm));
    strftime(const_cast<char*>(val.c_str()), val.size(), "%d/%m/%y-%R", &time_tm);
    return mktime(&time_tm);
}

Filter::GRADE Filter::convert_to_grade(const std::string &val) const {
    if      (val == "default") return GRADE::DEFAULT;
    else if (val == "green")   return GRADE::GREEN;
    else if (val == "yellow")  return GRADE::YELLOW;
    else if (val == "red")     return GRADE::RED;
    else                       return GRADE::MAX_GRADES;
}

int Filter::convert_to_int(const std::string &val) const {
    int value;
    try {
        value = stoi(val);
    }  catch (...) {
        return -1;
    }
    return value;
}

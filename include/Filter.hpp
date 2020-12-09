#ifndef FILTER_HPP
#define FILTER_HPP

#include <cstring>
#include <ctime>
#include <limits> //for MAX_INT
#include <utility>
#include "Query.hpp"
#include "LogLine.hpp"

class Filter {
public:
    enum class GRADE {
        DEFAULT,
        GREEN,
        YELLOW,
        RED,
        MAX_GRADES,
    };
    explicit Filter(const Query& query);
    bool check_filters(const LogLine& ll) const;
    void reset(void);
    bool is_correct(void) const;
    std::pair<time_t, time_t> get_time() const;
private:
    bool correct;
    int min_id, max_id;
    time_t min_time, max_time;
    GRADE min_grade, max_grade;
    std::string type, subj, host, result;

    bool set_id(const std::string& val, const char sign);
    bool set_date(const std::string& val, const char sign);
    bool set_grade(const std::string& val, const char sign);
    void set_type(const std::string& val);
    void set_subj(const std::string& val);
    void set_host(const std::string& val);
    void set_result(const std::string& val);

    bool check_id(const int val) const;
    bool check_date(const time_t val) const;
    bool check_grade(const GRADE val) const;
    bool check_type(const std::string& val) const;
    bool check_subj(const std::string& val) const;
    bool check_host(const std::string& val) const;
    bool check_result(const std::string& val) const;

    time_t convert_to_time(const std::string& val) const;
    GRADE convert_to_grade(const std::string& val) const;
    int convert_to_int(const std::string& val) const;
};



#endif //FILTER_HPP

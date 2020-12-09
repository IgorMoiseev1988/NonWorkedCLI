#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include "LogLine.hpp"
#include "Filter.hpp"
#include <mutex>
#include <algorithm>

class DataStorage
{
public:
    enum class caseSensetive {
        YES,
        NO
    };
    DataStorage();
    void add_line(const LogLine& line);
    void clear(void);
    void sort_by(LogLine::TYPE field);
    std::vector<LogLine> get_all(void);
    std::vector<LogLine> get_filtred(const Filter& filter);
    std::vector<LogLine> find(const std::string& saerch, caseSensetive cs);

private:
    LogLine::TYPE sort_field;
    std::mutex data_storage_mtx;
    std::vector<LogLine> storage;
    std::string to_lower(const std::string &str) const;
};

#endif // DATASTORAGE_HPP

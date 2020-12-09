#include "DataStorage.hpp"
/*
class DataStorage
{
public:
    enum class caseSensetive {
        YES,
        NO
    };
    std::vector<LogLine> find(const std::string& saerch, caseSensetive cs) const;
private:
};
*/
DataStorage::DataStorage() { }

void DataStorage::add_line(const LogLine &line) {
    if (!line.is_correct()) return;
    std::lock_guard<std::mutex> lock (data_storage_mtx);
    storage.push_back(line);
}

void DataStorage::clear() {
    std::lock_guard<std::mutex> lock(data_storage_mtx);
    storage.clear();
}

void DataStorage::sort_by(LogLine::TYPE field) {
    //TODO sort function
}

std::vector<LogLine> DataStorage::get_all() {
    std::lock_guard<std::mutex> lock(data_storage_mtx);
    return storage;
}

std::vector<LogLine> DataStorage::get_filtred(const Filter &filter) {
    std::vector<LogLine> filtred;
    for(const LogLine& line : get_all()) {
        if (filter.check_filters(line)) filtred.push_back(line);
    }
    return filtred;
}

std::vector<LogLine> DataStorage::find(const std::string &search, caseSensetive cs) {
    std::string find_text = (cs == caseSensetive::NO ? to_lower(search) : search);
    std::vector<LogLine> ret;
    std::lock_guard<std::mutex> lock(data_storage_mtx);
    for (const auto& line : storage) {
        for (LogLine::TYPE type = LogLine::TYPE::ID; type < LogLine::TYPE::TYPES_COUNT; LogLine::next_type(type)) {
            std::string cell = line.get_field(type);
            if (cell.find(find_text) != std::string::npos) {
                ret.push_back(line);
                break;
            }
        }
    }
    return ret;
}

//privat section

std::string DataStorage::to_lower(const std::string &str) const {
    std::string ret;
    std::transform(str.begin(), str.end(), std::back_inserter(ret),
                   [](const char c) {return std::tolower(c); });
    return ret;
}


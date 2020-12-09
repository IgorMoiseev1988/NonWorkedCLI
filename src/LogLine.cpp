#include "LogLine.hpp"

LogLine::LogLine(const std::string& raw) : correct {true} {
    line = make_data(raw);
}

std::string LogLine::get_field(TYPE type) const {
    return correct ? line.at(static_cast<size_t>(type)) : "";
}

bool LogLine::is_correct() const {
    return correct;
}

void LogLine::next_type(TYPE &type) {
    if (type == LogLine::TYPE::TYPES_COUNT) return;
    type = static_cast<TYPE>(static_cast<int>(type) + 1);
}

//private section

std::vector<std::string> LogLine::make_data(const std::string& str) {
    std::vector<std::string> ret = split(str);
    if (ret.size() != static_cast<size_t>(TYPE::TYPES_COUNT)) {
        ret.clear();
        correct = false;
    }
    return ret;
}

std::vector<std::string> LogLine::split(const std::string &str, const char splitter) const {
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
/*
std::string LogLine::at(int i) {
    switch(i) {
    case 0:  return std::to_string(ID);          break; //ID from db
    case 1:  return "";                          break; //number msg
    case 2:  return "-";                         break; //_my_id
    case 3:  return fields[type_field::GRADE];   break; //grade
    case 4:  return fields[type_field::HOST];    break; //host
    case 5:  return fields[type_field::TYPE];    break; //type
    case 6:  return fields[type_field::TIMESTR]; break; //time
    case 7:  return fields[type_field::PID];     break; //pid
    case 8:  return fields[type_field::UID];     break; //uid
    case 9:  return fields[type_field::OP];      break; //op
    case 10: return fields[type_field::SUBJ];    break; //subj
    case 11: return fields[type_field::OBJ];     break; //obj
    case 12: return fields[type_field::RESULT];  break; //res
    case 13: return fields[type_field::RAW];     break; //raw
    default: return "";                          break; //out-of-range
    }
}

*/

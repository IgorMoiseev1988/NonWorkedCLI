#ifndef __LOGLINE_HPP__
#define __LOGLINE_HPP__

#include <vector>
#include <string>
#include <ctime>
#include <iostream>

class LogLine {
public:
    enum class TYPE {
        ID,
        MSGNUM,
        MY_ID,
        GRADE,
        HOST,
        TYPE,
        TIME,
        PID,
        UID,
        OP,
        SUBJ,
        OBJ,
        RESULT,
        RAW,
        TYPES_COUNT
    };
    explicit LogLine(const std::string& raw);
    std::string get_field(TYPE) const;
    bool is_correct(void) const;
    static void next_type(TYPE& type);
private:
    bool correct;
    std::vector<std::string> line; // use vector, map izlishne bolshoj
    std::vector<std::string> make_data(const std::string& str);
    std::vector<std::string> split(const std::string& str, const char splitter = '|') const;
};
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
#endif

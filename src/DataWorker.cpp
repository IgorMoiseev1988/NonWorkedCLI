#include "DataWorker.hpp"

DataWorker::DataWorker(in_addr_t ip, int port, LogMaker& logger) {
    _run = true;
    memset(&buff, 0, BUFF_LEN);
    buff_ptr = &buff[0];
    _serv_addr = ip;
    _serv_port = port;
    _logger = &logger;
    _sock_with_ssl = nullptr;
    _ctx = _init_ctx();
    _my_id = "GUI" + std::string(getMachineName()) + ":" + 
                     std::string(std::to_string(getCpuHash())) + 
                     std::string(std::to_string(getVolumeHash()));
}

void DataWorker::start_connect() {
    bool state = true;
    struct sockaddr_in addr;
    _serv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serv_socket < 0) {
        _logger->put_log(LOG_ERROR, "socket() not gave listener");
        perror("asdf:");
        assert(_serv_socket >= 0 && "socket() fail.");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_serv_port);
    addr.sin_addr.s_addr = _serv_addr;
    int res = -1;
    
    while(res < 0) {
        res = connect(_serv_socket, (struct sockaddr *)&addr, sizeof(addr));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (res < 0) {
            if (state) {
                _logger->put_log(LOG_WARNING, "connect() to server faild");
                _logger->put_log(LOG_WARNING, "Wait coonnection...");
                state = false;
            }
        } else {
            _logger->put_log(LOG_INFO, "connect() to server success");
            state = true;
            std::string login_data = _my_id;

            _logger->put_log(LOG_TRACE, "Start SSL connect");
            _sock_with_ssl = SSL_new(_ctx);
            SSL_set_fd(_sock_with_ssl, _serv_socket);
            SSL_connect(_sock_with_ssl);
            _logger->put_log(LOG_TRACE, "SSL_connect() DONE");

            int rc = SSL_write(_sock_with_ssl, _my_id.c_str(), _my_id.size());
            if (rc < 1) {
                _logger->put_log(LOG_WARNING, "send() to server login data failed");
                res = -1;
                continue;
            } else {
                _logger->put_log(LOG_TRACE, "send() to server login data success");
            }
            char buff[64] = {0};
            rc = SSL_read(_sock_with_ssl, buff, 64);
            if (strcmp(buff, "ok") != 0) {
                _logger->put_log(LOG_WARNING, "login operation failed");
            } else {
                _logger->put_log(LOG_TRACE, "login operation success");
                break;
            }
        }
    }
    memset(buff, 0, BUFF_LEN);
    return;
}

void DataWorker::_poll_worker() {
    struct pollfd fds;
    fds.fd = _serv_socket;
    fds.events = POLLIN;
    bool zero = true;
    bool nozero = true;
    int zero_count = 0;
    int nozero_count = 0;
    while (_run) {
        Flags.reset(GETDATA);
        int ret = poll(&fds, 1, 1000);
        Flags.set(GETDATA);
        if (ret == -1) {
            _logger->put_log(LOG_ERROR, "poll crash");
        } else if (ret == 0) {
            continue;
        } else {
            _logger->put_log(LOG_TRACE, "incoming msg");
            if (fds.revents & POLLIN) {
                fds.revents = 0;
                int rc = SSL_read(_sock_with_ssl, buff_ptr, BUFF_LEN - (buff_ptr-buff));
                if (rc < 0) {
                    //server die
                    _logger->put_log(LOG_INFO, "Server die");
                    close(_serv_socket);
                    SSL_free(_sock_with_ssl);
                    _logger->put_log(LOG_INFO, "Reconnecting...");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    start_connect();
                    continue;
                } else {
                    if (rc == 0) {
                        if (zero) {
                            std::cout << "rc=" << rc << "\n";
                            zero = false;
                            nozero = true;
                        }
                        ++zero_count;
                    } else {
                        if (nozero) {
                            std::cout << "rc=" << rc << "\n";
                            zero = true;
                            nozero = false;
                        }
                        ++nozero_count;
                    }
                    parse_incoming_data(rc);

                }
            } else {
                _logger->put_log(LOG_WARNING, "have data from unknown socket");
            }
        }
    }
    std::cout << "Zero count = " << zero_count << "\nNozero_count = " << nozero_count << "\n";
}

void DataWorker::main_cycle()  {
    start_connect();
    _t_poll = std::thread(&DataWorker::_poll_worker, this);
    _t_tui = std::thread(&DataWorker::_tui_worker, this);
    _t_tui.join();
    _t_poll.join();
}
/*
void DataWorker::_tui_worker() {
    _logger->put_log(LOG_TRACE, "i'm in new thread");
    std::string str;

    while (str.find("exit") != 0) {
        std::string::size_type tmp_pos = str.find("-file");
        _FLAG.FULLVIEW = str.find("-full") != std::string::npos;

        if (tmp_pos != std::string::npos && (str.size() - tmp_pos) > 6) {
            std::string::size_type s = tmp_pos + 6;
            std::string::size_type e = str.find(" ", s);
            std::string file_name = str.substr(s, e -s);

            _file.open(file_name, std::ios::app);
            if (_file.is_open()) {
                _FLAG.INFILE = true;
                _logger->put_log(LOG_INFO, "File opened");
            } else {
                _logger->printf_log(LOG_WARNING, "Can't open file %s", file_name.c_str());
            }
        } else {
            _file.close();
            _FLAG.INFILE = false;
        }

        if (str.find("last") == 0) {
            std::string comm = _my_id + "|last \x1f";
            int rc = SSL_write(_sock_with_ssl, comm.c_str(), comm.size());
            if (rc < 0) {
                _logger->printf_log(LOG_WARNING, "Send %s to server failed!", comm.c_str());
            }
        } else if (str.find("list") == 0) {
            std::string comm = _my_id + "|list \x1f";
            int rc = SSL_write(_sock_with_ssl, comm.c_str(), comm.size());
            if (rc < 0) {
                _logger->printf_log(LOG_WARNING, "Send %s to server failed!", comm.c_str());
            }
        } else if (str.find("filter") == 0) {
            _FLAG.FILTRED = true;
            std::string comm = _my_id + "|list"; //need to use query with filter
            if (_parse_filter(str)) {
                //add filter to query
                if (!_filter.min_time.empty()) {
                    comm.append(" start=" + _filter.min_time);
                }
                if (!_filter.max_time.empty()) {
                    comm.append(" end=" + _filter.max_time);
                }
                comm.append(" \x1f");

                int rc = SSL_write(_sock_with_ssl, comm.c_str(), comm.size());
                if (rc < 0) {
                    _logger->printf_log(LOG_WARNING, "Send %s to server failed!", comm.c_str());
                }
            }
        } else if(str.find("help") == 0) {
            std::cout   << "ROSA-LOG-VIEWER-CONSOLE                                                        \n"
                        << "Используется для росмотра удаленного хранилища логов.                          \n"
                        << "Во время работы программы отображает новые сообщения (их id равен 0)           \n"
                        << "Список команд:                                                                 \n"
                        << "exit -   выход из программы                                                    \n"
                        << "last -   показать последние 10 сообщений                                       \n"
                        << "list -   показать все сообщения (рекомендуется использовать команду filter)    \n"
                        << "filter - запрашивает все сообщения с сервера и фильтрует их в соответсвии с    \n"
                        << "         заданными параметрами. Можно использовать любое количество параметров \n"
                        << "         Если один и тот же параметр встречается более одного раза, будет      \n"
                        << "         применен последний. Одинаковыми параметры будут считаться, если       \n"
                        << "         название параметра и его знак равны у обоих параметров.               \n" 
                        << "         В настоящее время поддерживаются следующие виды параметров:           \n"
                        << "id[знак]=    фильрует по id сообщения, присвоенного вовремя записи в базу      \n"
                        << "             данных(сообщения полученный в режиме раельного времени имеют id=0)\n"
                        << "             Знак может принимать любое из трех значений '>' '<' '='           \n"
                        << "             Знак больше и меньше будут комбинироваться, знак = отменяет       \n"
                        << "             действие знаков неравенства. Пример: filter id>5 id<7 - выведет   \n"
                        << "             сообщения c id в диапозоне от 10 до 15 включительно               \n"
                        << "time[знак]=  фильтрует времени сообщения (Внимание: временная зона UTC)        \n"
                        << "             Знак может принимать любое из трех значений '>' '<' '='           \n"
                        << "             Знак больше и меньше будут комбинироваться, знак = отменяет       \n"
                        << "             действие знаков неравенства. Время необходимо указывать в формате \n"
                        << "             дд/мм/гг-ЧЧ/ММ/СС Пример: filter time>04/08/20-15:00:00 - выведет \n"
                        << "             сообщения от 04 августа 2020г 15:00:00 (UTC) и новее              \n"
                        << "grade[знак]= фильтрует сообщения по категории. Знак может принимать любое из   \n"
                        << "             трех значений: '>' '<' '=' Знаки больше и меньше можно комбини-   \n"
                        << "             ровать. Знак = отменяет действие знаков неравенства. Всего есть   \n"
                        << "             четыре категори (по возрастанию значимости): 'default', 'green',  \n" 
                        << "             'yellow', 'red' Пример: filter grade>yellow - выведет сообщеня c  \n"
                        << "             категорией от yellow до категори red.                             \n"
                        << "host=        фильтрует сообщения по имени хоста.                               \n"
                        << "res=         фильтрует сообщения по значению результата                        \n"
                        << "                                                                               \n"
                        << "Так же поддерживается несколько флагов. Флаги должны идти только после команд  \n"
                        << "last, list или filter                                                          \n"
                        << "-file        перенаправляет вывод в файл, имя которого указано после флага.    \n"
                        << "             игнорирует флаг -full                                             \n"
                        << "-full        выводит сообщения в развернутом виде, без обрезания строк, по     \n"
                        << "             одному значению на каждой строке.                                 \n"
                        << "Например, для вывода в файл logs сообщений с датой позже 04/08/2020 12:00:00   \n"
                        << "с категорией red от хоста c именем nick34058 введите команду:                  \n"
                        << "filter time>04/08/20-12:00:00 grade=red host=nick34058 -file logs              \n";
        } else {
            std::cout << "Введите help для получения справки.\n";
        }
        str = "";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        while(_FLAG.GETDATA) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Wait data flag\n";
        }
        std::cout << ">";
        _FLAG.REQ = false;
        _FLAG.FILTRED = false;
        _filter.clear();
        std::getline(std::cin, str);
        _FLAG.REQ = true;
    }
    _file.close();
    _run = false;
}
*/



void DataWorker::_tui_worker() {
    std::string raw_query;
    while(raw_query != "exit") {
        std::cout << "Welcome to central-panel-CLI!>";
        std::getline(std::cin, raw_query);
        if (raw_query == "exit") break;
        if (raw_query == "help") {
            show_help();
            continue;
        }
        Query query(raw_query);
        if(!query.is_correct()) {
            std::cout << "Incorrect query. Use \"help\" commnad\n";
            continue;
        }
        Filter filter(query);
        if(!filter.is_correct()) {
            std::cout << "Filter bad. Use \"help\" command\n";
            continue;
        }

        if (query.get_command() == "get") {
            std::string request = query.generate_request(filter.get_time(), _my_id); //
            int rc = SSL_write(_sock_with_ssl, request.c_str(), request.size());
            if (rc < 0) {
                _logger->printf_log(LOG_WARNING, "Send %s to server failed!", request.c_str());
            }
            std::cout << "Wait data...\n";
            std::this_thread::sleep_for(std::chrono::seconds(4));
            out_data(storage.get_filtred(filter), query);
            continue;
        }
        if (query.get_command() == "find") {
            //find in data
            continue;
        }
        /*
        if (query.have_unary_option("file")) {
            _file.open(query.get_file_name(), std::ios::app);
        }
        if (_file.is_open()) {
            Flags.set(INFILE);
        } else {
            std::cerr << "File " << query.get_file_name() << "can't open!";
            Flags.reset(INFILE);
            continue;
        }
        */
        //do_query(query);
        while(Flags.test(GETDATA)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Wait data flag\n";
        }
        //show_data();default
        data.clear();
        std::cout << ">";

    }
    _run = false;
}

void DataWorker::out_data(const std::vector<LogLine> &data, const Query &query) const {
    bool colored = false;
    std::map<std::string, std::string> colors;
    std::FILE* out;
    if (!query.have_unary_option("file") || query.get_file_name().empty()) {
        out = stdout;
        colored = true;

    } else {
        out = fopen(query.get_file_name().c_str(), "a");
        if (out == nullptr) {
            std::cout << "Can't open file " << query.get_file_name() << "\n";
            return;
        }
    }
    for (const auto& line : data) {
        std::string id = prepare_id(line.get_field(LogLine::TYPE::ID));
        std::string color = (colored ? get_console_color(line.get_field(LogLine::TYPE::GRADE)) : "");
        std::string reset_color = (colored ? "\033[37m" : "");
        if (!query.have_unary_option("full")) { //in file or full view
            fputs(color.c_str(), out);
            fputs(id.c_str(), out);
            fputs("\nTIME=", out);
            fputs(line.get_field(LogLine::TYPE::TIME).c_str(), out);
            fputs("\nHOST=", out);
            fputs(line.get_field(LogLine::TYPE::HOST).c_str(), out);
            fputs("\nGRADE=", out);
            fputs(line.get_field(LogLine::TYPE::GRADE).c_str(), out);
            fputs("\nOPERATION=", out);
            fputs(line.get_field(LogLine::TYPE::OP).c_str(), out);
            fputs("\nSUBJ=", out);
            fputs(line.get_field(LogLine::TYPE::SUBJ).c_str(), out);
            fputs("\nOBJ=", out);
            fputs(line.get_field(LogLine::TYPE::OBJ).c_str(), out);
            fputs("\nTYPE=", out);
            fputs(line.get_field(LogLine::TYPE::TYPE).c_str(), out);
            fputs("\nRESULT=", out);
            fputs(line.get_field(LogLine::TYPE::RESULT).c_str(), out);
            fputs("\nPID=", out);
            fputs(line.get_field(LogLine::TYPE::RESULT).c_str(), out);
            fputs("\nUID=", out);
            fputs(line.get_field(LogLine::TYPE::UID).c_str(), out);
            fputs("\n", out);
            fputs(line.get_field(LogLine::TYPE::RAW).c_str(), out);
            fputs("\n", out);
            fputs(reset_color.c_str(), out);
            fputs("\n", out);
        }
    }
}

std::string DataWorker::prepare_id(const std::string &id) const {
    std::string ret;
    int len_id = id.size();
    size_t left_len, right_len;
    right_len = (80 - len_id) / 2;
    left_len = 80 - right_len - len_id;
    ret.append(left_len, '=');
    ret.append(id);
    ret.append(right_len, '=');
    return ret;
}

std::string DataWorker::get_console_color(const std::string& grade) const {
    if      (grade == "DEFAULT") return "\033[37m"; //white
    else if (grade == "GREEN")   return "\033[32m"; //green
    else if (grade == "YELLOW")  return "\033[33m"; //yellow
    else if (grade == "RED")     return "\033[31m"; //red
    else                         return "\033[37m"; //white
}




//void DataWorker::_write_line(LogLine* line) {
//    if (Flags.test(FILTRED)) {
//        if (!_check_filter(line)) return;
//    }
//    if (_file.is_open()) {
//        std::string _id = line->at(0);
//        int _len_id = _id.size();
//        _id.insert(_id.begin(), (80 - _len_id) / 2, '-');
//        _id.append(((80 - _len_id) % 2) == 1 ? ((80 - _len_id) / 2 + 1) : ((80 - _len_id) / 2), '-');
//        _file   << _id << "\n"
//                << "TIME=" << line->at(6) << "\n"
//                << "HOST=" << line->at(4) << "\n"
//                << "GRADE=" << line->at(3) << "\n"
//                << "OPERATION=" << line->at(9) << "\n"
//                << "SUBJ=" << line->at(10) << "\n"
//                << "OBJ=" << line->at(11) << "\n"
//                << "TYPE=" << line->at(5) << "\n"
//                << "RESULT=" << line->at(12) << "\n"
//                << "PID=" << line->at(7) << "\n"
//                << "UID=" << line->at(8) << "\n"
//                << "RAW STRING=" << line->at(13) << "\n"
//                << "\n\n";
//    } else {
//        _logger->put_log(LOG_WARNING, "File output error");
//    }


//}

//time_t DataWorker::convert_to_time_t(const std::string& val) {
//    struct tm time_tm;
//    memset(&time_tm, 0, sizeof(struct tm));
//    strptime(val.c_str(), "%d/%m/%y-%H:%M:%S", &time_tm);
//    return mktime(&time_tm);
//}



void DataWorker::show_help() {
    std::cout   << "ROSA-LOG-VIEWER-CONSOLE                                                        \n"
                << "Используется для росмотра удаленного хранилища логов.                          \n"
                << "Во время работы программы отображает новые сообщения (их id равен 0)           \n"
                << "Список команд:                                                                 \n"
                << "exit -   выход из программы                                                    \n"
                << "help -   показать последние справку                                            \n"
                << "Запрос сообщений выполняться отправкой фильтров. Можно использовать любое      \n"
                << "количество параметров.                                                         \n"
                << "Если один и тот же параметр встречается более одного раза, будет применен      \n"
                << "последний. Одинаковыми параметры будут считаться, если название параметра и его\n"
                << "знак равны у обоих параметров.                                                 \n"
                << "В настоящее время поддерживаются следующие виды параметров:                    \n"
                << "id[> | < | =] фильрует по id сообщения, присвоенного вовремя записи в базу     \n"
                << "             данных(сообщения полученный в режиме раельного времени имеют id=0)\n"
                << "             Знак больше и меньше будут комбинироваться, знак = отменяет       \n"
                << "             действие знаков неравенства. Пример: filter id>5 id<7 - выведет   \n"
                << "             сообщения c id в диапозоне от 10 до 15 включительно               \n"
                << "time[> | < | =]  фильтрует времени сообщения (Внимание: временная зона UTC)    \n"
                << "             Знак больше и меньше будут комбинироваться, знак = отменяет       \n"
                << "             действие знаков неравенства. Время необходимо указывать в формате \n"
                << "             дд/мм/гг-ЧЧ/ММ/СС Пример: filter time>04/08/20-15:00:00 - выведет \n"
                << "             сообщения от 04 августа 2020г 15:00:00 (UTC) и новее              \n"
                << "grade[знак]= фильтрует сообщения по категории. Знаки больше и меньше можно     \n"
                << "             комбинировать. Знак = отменяет действие знаков неравенства        \n"
                << "             Всего есть четыре категори (по возрастанию значимости):           \n"
                << "             'default', 'green', 'yellow', 'red'                               \n"
                << "             Пример: filter grade>yellow - выведет сообщеня c категорией от    \n"
                << "             yellow до категори red.                                           \n"
                << "host=        фильтрует сообщения по имени хоста. Знаки > и < не предусмотрены  \n"
                << "res=         фильтрует сообщения по значению результата                        \n"
                << "                                                                               \n"
                << "Так же поддерживается несколько флагов.                                        \n"
                << "-file        перенаправляет вывод в файл, имя которого указано после флага.    \n"
                << "             игнорирует флаг -full                                             \n"
                << "-full        выводит сообщения в развернутом виде, без обрезания строк, по     \n"
                << "             одному значению на каждой строке.                                 \n"
                << "Например, для вывода в файл logs сообщений с датой позже 04/08/2020 12:00:00   \n"
                << "с категорией red от хоста c именем nick34058 введите команду:                  \n"
                << "time>04/08/20-12:00:00 grade=red host=nick34058 -file logs                     \n";
}

void DataWorker::parse_incoming_data(int data_size) {
    std::string tmp;
    for (int i = 0; i < data_size + (buff_ptr-buff); i++) {
        if (buff[i] == '\x1f') {
            LogLine logline(tmp);
            if (logline.is_correct()) {
                storage.add_line(logline);
            } else {/*log line bad */
                _logger->put_log(LOG_WARNING, "logline bad");
                _logger->put_log(LOG_WARNING, tmp.c_str());
            }
            tmp.clear();
        } else {/* buff[i] != \x1f */
            tmp += buff[i];
        }
    }
    if (tmp.size() > 0) {
        _logger->put_log(LOG_TRACE, std::string("tmp have " + tmp).c_str());
    }
    memset(&buff, 0, BUFF_LEN);
    strcpy(buff, tmp.c_str());
    buff_ptr = buff + tmp.size();
    tmp.clear();
}

//std::string DataWorker::fix_string(const std::string& str, int len) const{
//    if (str.size() == static_cast<size_t>(len)) return str;//

//    std::string _s                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           = str;
//    int i = len - str.size();

//    if (i > 0) {/* str.size() < len */
//        for (; i >0; i--) {
//            _s.append(" ");
//        }
//    } else {       /* str.size() > len */
//        _s = _s.substr(0, len - 3);
//        _s.append("...");
//    }
//    return _s;
//}

//void DataWorker::_show_line(LogLine* line) {
//    if (Flags.test(FILTRED)) {
//        if (!_check_filter(line)) return;
//    }
//    //\033[32m - green
//    //\033[31m - red
//    //\033[33m - yellow
//    //\033[37m - white
//    std::string color;
//    std::string white = "\033[37m";
       
//    switch (line->get_grade_lvl()) {
//    case 3: color = "\033[31m"; break;
//    case 2: color = "\033[33m"; break;
//    case 1: color = "\033[32m"; break;
//    case 0: color = "\033[37m"; break;
//    }

//    std::string _id = line->at(0);
//    int _len_id = _id.size();
//    _id.insert(_id.begin(), (80 - _len_id) / 2, '-');
//    _id.append(((80 - _len_id) % 2) == 1 ? ((80 - _len_id) / 2 + 1) : ((80 - _len_id) / 2), '-');

//    if (Flags.test(FULLVIEW)) {
//        std::cout   << _id << "\n"
//                    << color
//                    << _fix_string("TIME=" + line->at(6), 40)
//                    << _fix_string("HOST=" + line->at(4), 20)
//                    << _fix_string("GRADE=" + line->at(3), 20)
//                    << "\n"
//                    << _fix_string("OPERATION=" + line->at(9), 40)
//                    << _fix_string("SUBJ=" + line->at(10), 20)
//                    << _fix_string("OBJ=" + line->at(11), 20)
//                    << "\n"
//                    << _fix_string("TYPE=" + line->at(5), 20)
//                    << _fix_string("RESULT=" + line->at(12), 20)
//                    << _fix_string("PID=" + line->at(7), 20)
//                    << _fix_string("UID=" + line->at(8), 20)
//                    << "\n"
//                    << line->at(13)
//                    << white
//                    << "\n";
//    } else {
//        std::cout   << _id << "\n"
//                    << color
//                    << "TIME=" << line->at(6) << '\n'
//                    << "HOST=" << line->at(4) << '\n'
//                    << "GRADE=" << line->at(3) << '\n'
//                    << "OPERATION=" << line->at(9) << '\n'
//                    << "SUBJ=" << line->at(10) << '\n'
//                    << "OBJ=" << line->at(11) << '\n'
//                    << "TYPE=" << line->at(5) << '\n'
//                    << "RESULT=" << line->at(12) << '\n'
//                    << "PID=" << line->at(7) << '\n'
//                    << "UID=" << line->at(8) << '\n'
//                    << line->at(13)
//                    << white << '\n';

//    }
//}

//void DataWorker::_write_line(LogLine* line) {
//    if (Flags.test(FILTRED)) {
//        if (!_check_filter(line)) return;
//    }
//    if (_file.is_open()) {
//        std::string _id = line->at(0);
//        int _len_id = _id.size();
//        _id.insert(_id.begin(), (80 - _len_id) / 2, '-');
//        _id.append(((80 - _len_id) % 2) == 1 ? ((80 - _len_id) / 2 + 1) : ((80 - _len_id) / 2), '-');
//        _file   << _id << "\n"
//                << "TIME=" << line->at(6) << "\n"
//                << "HOST=" << line->at(4) << "\n"
//                << "GRADE=" << line->at(3) << "\n"
//                << "OPERATION=" << line->at(9) << "\n"
//                << "SUBJ=" << line->at(10) << "\n"
//                << "OBJ=" << line->at(11) << "\n"
//                << "TYPE=" << line->at(5) << "\n"
//                << "RESULT=" << line->at(12) << "\n"
//                << "PID=" << line->at(7) << "\n"
//                << "UID=" << line->at(8) << "\n"
//                << "RAW STRING=" << line->at(13) << "\n"
//                << "\n\n";
//    } else {
//        _logger->put_log(LOG_WARNING, "File output error");
//    }
//}

/*
bool DataWorker::_parse_filter(const std::string& str) {
    std::string err_str = "Error query. Use \"help\" for more information\n";
    std::string::size_type s = str.find(" ");
    std::string::size_type e = str.find(" ", s + 1);
    if (s == std::string::npos) {
        std::cout << err_str;
        return false;
    }
    
    //split by whitespace
    std::list<std::string> list_tmp;
    while (s != std::string::npos) {
        list_tmp.push_back(str.substr(s+1, e - s-1));
        s = e;
        e = str.find(" ", s + 1);
    }

    while (!list_tmp.empty()) {
        std::string filter_name, sign, value, tmp;
        tmp = list_tmp.front();
        list_tmp.pop_front();
        if (tmp.find("-") == 0) break;
        if (tmp.find("=") != std::string::npos) {
            filter_name = tmp.substr(0, tmp.find("="));
            sign = "=";
            value = tmp.substr(tmp.find("=") + 1);
        } else if (tmp.find(">") != std::string::npos) {
            filter_name = tmp.substr(0, tmp.find(">"));
            sign = ">";
            value = tmp.substr(tmp.find(">") + 1);
        } else if (tmp.find("<") != std::string::npos) {
            filter_name = tmp.substr(0, tmp.find("<"));
            sign = "<";
            value = tmp.substr(tmp.find("<") + 1);
        } else {
            std::cout << err_str;
            return false;
        }

        if (filter_name == "id") {
            if (sign == "=") {
                try {
                    _filter.minid = _filter.maxid = stoi(value);
                } catch (...) {
                    std::cout << err_str;
                    return false;
                }
            } else if (sign == "<") {
                try {
                    _filter.maxid = stoi(value);
                } catch (...) {
                    std::cout << err_str;
                    return false;
                }
            } else if (sign == ">") {
                try {
                    _filter.minid = stoi(value);
                } catch (...) {
                    std::cout << err_str;
                    return false;
                }
            }
        } else if (filter_name == "grade") {
            if (sign == "=") {
                _filter.mingrade = _filter.maxgrade = value;
            } else if (sign == "<") {
                _filter.maxgrade = value;
            } else if (sign == ">") {
                _filter.mingrade = value;
            }
        } else if (filter_name == "host") {
            if (sign == "=") {
                _filter.host = value;
            } else {
                std::cout << err_str;
                return false; 
            }
        } else if (filter_name == "res") {
            if (sign == "=") {
                _filter.res = value;
            } else {
                std::cout << err_str;
                return false; 
            }
        } else if (filter_name == "time") {
            if(sign == "=") {
                _filter.max_time = value;
                _filter.min_time = value;
            } else if (sign == "<") {
                _filter.max_time = value;
            } else if (sign == ">") {
                _filter.min_time = value;
            }
        } else {
            std::cout << err_str;
            return false;
        }
    }
    return true;
} */

//bool DataWorker::_check_filter(LogLine* line) {
//    //check ID
//    if (_filter.minid != 0 && _filter.maxid != 0) {
//        if(!(line->get_ID() >= _filter.minid && line->get_ID() <= _filter.maxid)) return false;
//    } else if (_filter.minid != 0) {
//        if(!(line->get_ID() >= _filter.minid)) return false;
//    } else if (_filter.maxid != 0) {
//        if (!(line->get_ID() <= _filter.maxid)) return false;
//    }
    
//    //check time;  parse string and compare
//    time_t max;
//    time_t min;
//    time_t cur;
//    struct tm _tm;
//    memset(&_tm, 0, sizeof(struct tm));

//    strptime(line->get_time().c_str(), "%d/%m/%y-%H:%M:%S", &_tm);
//    cur = mktime(&_tm);
//    if (cur == -1) {
//        _logger->printf_log(LOG_WARNING, "mktime() fail, string time: %d", line->get_time().c_str());
//        return false;
//    }
//    if (!_filter.min_time.empty()) {
//        memset(&_tm, 0, sizeof(struct tm));
//        strptime(_filter.min_time.c_str(), "%d/%m/%y-%H:%M:%S", &_tm);
//        min = mktime(&_tm);
//        if (min == -1) {
//            _logger->printf_log(LOG_WARNING, "mktime() fail, string time: %d", _filter.min_time.c_str());
//            return false;
//        }
//    }
//    if (!_filter.max_time.empty()) {
//        memset(&_tm, 0 , sizeof(struct tm));
//        strptime(_filter.max_time.c_str(), "%d/%m/%y-%H:%M:%S", &_tm);
//        max = mktime(&_tm);
//        if (max == -1) {
//             _logger->printf_log(LOG_WARNING, "mktime() fail, string time: %d", _filter.max_time.c_str());
//            return false;
//        }
//    }
//    if (!_filter.max_time.empty() && !_filter.min_time.empty()) {
//        if (!(cur >= min && cur <= max)) return false;
//    } else if (!_filter.min_time.empty()) {
//        if (!(cur >= min)) return false;
//    } else if (!_filter.max_time.empty()) {
//        if (!(cur <= max)) return false;
//    }
    
//    //check grade;
//    char _mig, _mag;
    
//    if (_filter.mingrade == "DEFAULT" || _filter.mingrade == "default") _mig = 0;
//    else if (_filter.mingrade == "GREEN" || _filter.mingrade == "green") _mig = 1;
//    else if (_filter.mingrade == "YELLOW" || _filter.mingrade == "yellow") _mig = 2;
//    else if (_filter.mingrade == "RED" || _filter.mingrade == "red")_mig = 3;
//    if (_filter.maxgrade == "DEFAULT" || _filter.maxgrade == "default") _mag = 0;
//    else if (_filter.maxgrade == "GREEN" || _filter.maxgrade == "green") _mag = 1;
//    else if (_filter.maxgrade == "YELLOW" || _filter.maxgrade == "yellow") _mag = 2;
//    else if (_filter.maxgrade == "RED" || _filter.maxgrade == "red") _mag = 3;
//    if (!_filter.maxgrade.empty() && !_filter.mingrade.empty()) {
//        if (!(line->get_grade_lvl() >= _mig && line->get_grade_lvl() <= _mag)) return false;
//    } else if (!_filter.mingrade.empty()) {
//        if (!(line->get_grade_lvl() >= _mig)) return false;
//    } else if (!_filter.maxgrade.empty()) {
//        if (!(line->get_grade_lvl() <= _mag)) return false;
//    }

    //check host
    //if (!_filter.host.empty())
    //{
     //   if (!(line->compare_hostname(_filter.host)))
    //    {
    //        return false;
    //    }
    //}

    //check res
    //if (!_filter.res.empty())
    //{
    //    if (!(line->compare_result(_filter.res)))
    //    {
    //        return false;
    //    }
    //}
//    return true;
//}

SSL_CTX *DataWorker::_init_ctx(void) {
    // @todo: logging and error handler
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLSv1_2_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        assert(0 && "ssl_new error");
    }
    return ctx;
}

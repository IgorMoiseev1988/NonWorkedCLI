#ifndef __LISTNER_HPP__
#define __LISTNER_HPP__

#include <string>
#include <mutex>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>  
#include <thread>
#include <chrono>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <bitset>
#include <map>
#include <set>
#include <queue>
#include <cstdio>
#include "LogMaker.hpp"
#include "Utils.hpp"
#include "poll.h"
#include "LogLine.hpp"
#include "Query.hpp"
#include "Filter.hpp"
#include "DataStorage.hpp"


#define BUFF_LEN  4096

enum flags {
    INFILE,
    FULLVIEW,
    GETDATA,
    REQ,
    FILTRED,
};

class DataWorker {
public:
    explicit DataWorker(in_addr_t ip, int port, LogMaker& logger);
    void main_cycle();

private:
    std::bitset<8> Flags;
    std::vector<LogLine> data;
	std::mutex _data_mtx;
	in_addr_t _serv_addr;
	int _serv_port;
	int _serv_socket;
	LogMaker* _logger;
	std::string _my_id;
	std::thread _t_poll;
	std::thread _t_tui;
    DataStorage storage;

    char buff[BUFF_LEN];
    char* buff_ptr;

	SSL_CTX *_ctx;
	SSL *_sock_with_ssl;



    void _poll_worker();
	void _tui_worker();

    void out_data(const std::vector<LogLine>& data, const Query& query) const;
    std::string prepare_id(const std::string& id) const;
    std::string get_console_color(const std::string& grade) const;

    std::string fix_string(const std::string& str, int len) const;
	bool _run;
	void _show_line(LogLine* line);
	void _write_line(LogLine* line);

    std::string time_query(void);
    void show_help(void);
    void parse_incoming_data(int data_size);
    void show_data();
	SSL_CTX *_init_ctx(void);
    void start_connect();

};

#endif

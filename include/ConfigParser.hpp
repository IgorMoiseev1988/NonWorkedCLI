
#ifndef __CONFIGPARSER_HPP__
#define __CONFIGPARSER_HPP__

#include <string>
#include <fstream>
#include <exception>
#include <cstring>
#include <ctime>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>



class ConfigParser
{

private:
	
	std::string _conf_path;
	int _serv_port;
	in_addr_t _serv_addr;
	std::string _serv_addr_str;
	std::string _my_id;


public:
	
	ConfigParser(void) = delete;
	ConfigParser(const char *path);
	~ConfigParser(void);
	
	void set_new_path(const char *path);
	int parse(void);
	int get_serv_port(void);
	in_addr_t get_serv_addr(void);
	std::string get_serv_addr_str(void);

};

#endif /* __CONFIGPARSER_HPP__ */
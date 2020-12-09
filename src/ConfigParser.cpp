
#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const char *path) {
	_conf_path = path;
}

ConfigParser::~ConfigParser(void) { }

void ConfigParser::set_new_path(const char *path) {
	_conf_path = path;
}

int ConfigParser::parse(void) {
    std::ifstream conf;
    std::string str;
    conf.open(_conf_path);
    if (!conf.good()) {
        throw std::string("bad path or perm");
    }
    while (!conf.eof()) {
        std::getline(conf, str);
        if (str.find("ip") != std::string::npos) {
            str.erase(0, 4);
            _serv_addr_str = str;
            _serv_addr = inet_addr(str.c_str());
            if (_serv_addr == (unsigned int) - 1) throw std::string("bad server ip");
        }
        if (str.find("port") != std::string::npos) {
            str.erase(0, 6);
            _serv_port = atoi(str.data());
        }
    }
    conf.close();
    return (0);
}

int ConfigParser::get_serv_port(void) {
	return (_serv_port);
}

in_addr_t ConfigParser::get_serv_addr(void) {
	return (_serv_addr);
}

std::string ConfigParser::get_serv_addr_str(void) {
    return (_serv_addr_str);
}

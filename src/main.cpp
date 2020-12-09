#include "main.hpp"
#include "DataWorker.hpp"
#include "ConfigParser.hpp"

#define CONF_PATH "/etc/rosa-central-panel-ui.conf"
#define LOG_PATH "/var/log/rosa-central-panel-ui.log"

int main()
{
    ConfigParser cp(CONF_PATH);
    LogMaker* _logger = new LogMaker(LOG_PATH);
    _logger->set_log_level(LOG_INFO);

    try {
        cp.parse();
    } catch(const std::string str) {
        _logger->put_log(LOG_ERROR, str.c_str());
        exit(1);
    }

    _logger->put_log(LOG_INFO, std::string("Server address - " + cp.get_serv_addr_str() + ":" + std::to_string(cp.get_serv_port())).c_str());

    DataWorker dw(cp.get_serv_addr(), cp.get_serv_port(), *_logger);
    dw.main_cycle();
    
    return (0);
}

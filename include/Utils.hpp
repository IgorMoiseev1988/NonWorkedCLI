#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>

#include <stdio.h>
#include <string.h>
#include <unistd.h>          
#include <errno.h>           
#include <sys/types.h>       
#include <sys/socket.h>      
#include <sys/ioctl.h>  
#include <sys/resource.h>    
#include <sys/utsname.h>       
#include <netdb.h>           
#include <netinet/in.h>      
#include <netinet/in_systm.h>                 
#include <netinet/ip.h>      
#include <netinet/ip_icmp.h> 
#include <assert.h>

#include <ifaddrs.h>         
#include <linux/if.h>        
#include <linux/sockios.h>   

const char* getMachineName();
unsigned short getVolumeHash();
unsigned short getCpuHash();

namespace rosautils
{
    void insertStrWithSeparator(const std::string& insert, const char *sep, std::string &str);
}

#endif /* _UTILS_HPP_ */

#include "Utils.hpp"

const char* getMachineName() { 
   static struct utsname u;  
   if (uname(&u) < 0) {
      assert(0);             
      return "unknown";      
   }       
   return u.nodename;        
}   

//---------------------------------get MAC addresses ------------------------------------unsigned short-unsigned short----------        
// we just need this for purposes of unique machine id. So any one or two mac's is fine.            
unsigned short hashMacAddress(unsigned char* mac) {
   unsigned short hash = 0;             
   for (unsigned int i = 0; i < 6; i++) {
      hash += (mac[i] << ((i & 1) * 8));           
   }       
   return hash;              
} 

void getMacHash(unsigned short& mac1, unsigned short& mac2) {
   mac1 = 0;                 
   mac2 = 0;                        
   int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);                  
   if (sock < 0) return;   
   // enumerate all IP addresses of the system         
   struct ifconf conf;       
   char ifconfbuf[ 128 * sizeof(struct ifreq)  ];      
   memset( ifconfbuf, 0, sizeof( ifconfbuf ));         
   conf.ifc_buf = ifconfbuf; 
   conf.ifc_len = sizeof( ifconfbuf );        
   if (ioctl(sock, SIOCGIFCONF, &conf)) {
      assert(0);             
      return;                
   }       
   // get MAC address        
   bool foundMac1 = false;   
   struct ifreq* ifr;        
   for (ifr = conf.ifc_req; (char*)ifr < (char*)conf.ifc_req + conf.ifc_len; ifr++) {
      if (ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data)  continue;  // duplicate, skip it
      if (ioctl(sock, SIOCGIFFLAGS, ifr))                      continue;  // failed to get flags, skip it
      if (ioctl(sock, SIOCGIFHWADDR, ifr) == 0) {
         if (!foundMac1) {
            foundMac1 = true;                 
            mac1 = hashMacAddress((unsigned char*)&(ifr->ifr_addr.sa_data));       
         } else {
            mac2 = hashMacAddress((unsigned char*)&(ifr->ifr_addr.sa_data));       
            break;           
         } 
      }    
   }       
   close(sock);                     
   // sort the mac addresses. We don't want to invalidate                
   // both macs if they just change order.    
   if (mac1 > mac2) {
      unsigned short tmp = mac2;        
      mac2 = mac1;           
      mac1 = tmp;            
   }       
} 

unsigned short getVolumeHash() {
   // we don't have a 'volume serial number' like on windows. Lets hash the system name instead.    
   unsigned char* sysname = (unsigned char*)getMachineName();       
   unsigned short hash = 0;             
   for (unsigned int i = 0; sysname[i]; i++) {
      hash += (sysname[i] << (( i & 1 ) * 8));
   }
   return hash;              
}     

static void getCpuid(unsigned int* p, unsigned int ax) {
   __asm __volatile         
   (   "movl %%ebx, %%esi\n\t"               
      "cpuid\n\t"          
      "xchgl %%ebx, %%esi" 
      : "=a" (p[0]), "=S" (p[1]),           
         "=c" (p[2]), "=d" (p[3])            
      : "0" (ax)           
   );     
}         

unsigned short getCpuHash() {
   unsigned int cpuinfo[4] = { 0, 0, 0, 0 };          
   getCpuid(cpuinfo, 0);  
   unsigned short hash = 0;            
   unsigned int* ptr = (&cpuinfo[0]);                 
   for (unsigned int i = 0; i < 4; i++) {
      hash += (ptr[i] & 0xFFFF) + (ptr[i] >> 16);   
   }
   return hash;             
}         

namespace rosautils {
   void insertStrWithSeparator(const std::string& insert, const char *sep, std::string &str) {
      str.insert(0, sep);
      str.insert(0, insert);
   }
}

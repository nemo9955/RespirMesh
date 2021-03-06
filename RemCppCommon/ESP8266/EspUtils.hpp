#ifndef ESPUTILS_HPP_
#define ESPUTILS_HPP_


#include <IPAddress.h>

typedef struct _RemClientInfo
{
    bool connected_to_root;
    int16_t port;
    IPAddress rem_server_ip;


} RemClientInfo;

// String macToStr(const unsigned char *mac)
// {
//     char buf[20];
//     snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
//              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//     return String(buf);
// }

#endif /* !ESPUTILS_HPP_ */

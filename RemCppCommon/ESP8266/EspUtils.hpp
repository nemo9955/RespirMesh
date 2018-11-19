#ifndef ESPUTILS_HPP_
#define ESPUTILS_HPP_

typedef struct _RemClientInfo
{
    bool connected_to_root;
    int16_t port;
    uint8_t ip0;
    uint8_t ip1;
    uint8_t ip2;
    uint8_t ip3;

} RemClientInfo;

// String macToStr(const unsigned char *mac)
// {
//     char buf[20];
//     snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
//              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//     return String(buf);
// }

#endif /* !ESPUTILS_HPP_ */

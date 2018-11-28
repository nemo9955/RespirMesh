#ifndef X86LINUXHARDWARE_HPP_
#define X86LINUXHARDWARE_HPP_

#include <chrono>
#include <unistd.h>

class X86LinuxHardware : public RemHardware
{
  public:
    X86LinuxHardware(){};
    ~X86LinuxHardware(){};

    uint32_t chip_id;

    uint32_t device_id()
    {
        return chip_id;
    };

    void sleep_milis(uint32_t duration)
    {
        usleep(duration);
    };

    uint32_t time_milis()
    {
        uint32_t milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
        return milliseconds_since_epoch;
    };
};

#endif /* !X86LINUXHARDWARE_HPP_ */

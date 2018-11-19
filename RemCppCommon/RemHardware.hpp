#ifndef REMHARDWARE_HPP_
#define REMHARDWARE_HPP_

#include "RemHeaderTypes.h"

class RemHardware
{
  public:
    // RemHardware();
    // ~RemHardware();
    virtual uint32_t device_id() = 0;
    virtual uint32_t time_milis() = 0;
    virtual uint32_t sleep_milis(uint32_t) = 0;
};

#endif /* !REMHARDWARE_HPP_ */

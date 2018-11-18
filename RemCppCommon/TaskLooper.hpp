#ifndef TASKLOOPER_HPP_
#define TASKLOOPER_HPP_

#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.h"

class RemOrchestrator;

class TaskLooper
{
  public:
    TaskLooper();
    ~TaskLooper();

    void begin(Hardware *remHw_);
    void set(uint16_t milis);
    void set_slow();
    void set_norm();
    void set_fast();
    bool check();
    void start();
    void stop();

  protected:
    Hardware *remHw;

  private:
    uint32_t next_ts;
    uint16_t tl_interval;
};

#endif /* !TASKLOOPER_HPP_ */

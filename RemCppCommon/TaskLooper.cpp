#include "TaskLooper.hpp"

#define tli_slow (40 * 1000)
#define tli_norm (20 * 1000)
#define tli_fast (10 * 1000)

TaskLooper::TaskLooper()
{
    stop();
};

void TaskLooper::begin(Hardware *remHw_)
{
    remHw = remHw_;
    set_norm();
};

TaskLooper::~TaskLooper(){

};

bool TaskLooper::check()
{
    // logf("TaskLooper::check %u %u \n", remHw->time_milis(), next_ts);
    if (next_ts > 0 && remHw && remHw->time_milis() > next_ts)
    {
        next_ts = remHw->time_milis() + tl_interval;
        return true;
    }
    return false;
};

void TaskLooper::start()
{
    if (remHw)
    {
        next_ts = remHw->time_milis() + tl_interval;
    }
    else
    {
        stop();
    }
};

void TaskLooper::stop()
{
    next_ts = -1;
};

void TaskLooper::set(uint16_t milis)
{
    tl_interval = milis;
    start();
};

void TaskLooper::set_slow()
{
    tl_interval = tli_slow;
    start();
};

void TaskLooper::set_norm()
{

    tl_interval = tli_norm;
    start();
};

void TaskLooper::set_fast()
{
    tl_interval = tli_fast;
    start();
};

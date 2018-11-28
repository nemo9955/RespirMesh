#ifndef CONNECTIONLISTENER_HPP_
#define CONNECTIONLISTENER_HPP_

class ConnectionListener
{
  public:
    // ConnectionListener();
    // ~ConnectionListener();

    virtual void on_conn_lost(void *_chan) = 0;
    virtual void on_conn_start(void *_chan) = 0;

  protected:
  private:
};

#endif /* !CONNECTIONLISTENER_HPP_ */

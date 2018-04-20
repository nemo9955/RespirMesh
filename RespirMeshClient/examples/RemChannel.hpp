#ifndef REMCHANNEL_HPP_
#define REMCHANNEL_HPP_

typedef void (*ReceiveDataCallback)(uint8_t *data, uint16_t size);

template <class Hardware,
          uint16_t MAX_SUBSCRIBERS = 25,
          uint16_t MAX_PUBLISHERS = 25,
          uint16_t USER_IN_BUFF_SIZE = 100,
          uint16_t USER_OUT_BUFF_SIZE = 100,
          uint16_t INTERNAL_IN_BUFF_SIZE = 50,
          uint16_t INTERNAL_OUT_BUFF_SIZE = 50,
class RemChannel
{
  public:
    RemChannel();
    ~RemChannel();

    void init();
    void update();
    // void begin();
    // void pause();
    // void resume();
    // void stop();

    void send(uint8_t *data, uint16_t size);
    void onReceiveAll(ReceiveDataCallback cb, void *arg = 0); //on successful connect

  private:
    ReceiveDataCallback allDataCb;
    void *allDataCb_arg;

  protected:
};

#endif /* !REMCHANNEL_HPP_ */

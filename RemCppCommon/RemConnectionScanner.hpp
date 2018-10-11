#ifndef REMCONNECTIONSCANNER_HPP_
#define REMCONNECTIONSCANNER_HPP_

class RemConnectionScanner
{
  public:
    virtual void add_parent(char *_host, char *_port) = 0;

  protected:
  private:
};

#endif /* !REMCONNECTIONSCANNER_HPP_ */

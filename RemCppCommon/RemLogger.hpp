#ifndef REMLOGGER_HPP_
#define REMLOGGER_HPP_

#define REM_LOG_STRING_BUFFER_SIZE 128

#include <stdint.h>

#include "mesh-packet.pb.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "RemOrchestrator.hpp"

class RemOrchestrator;

class RemLogger
{
  public:
    RemLogger();
    ~RemLogger();

    int trace(const char *aFormat, ...);
    int debug(const char *aFormat, ...);
    int info(const char *aFormat, ...);
    int warn(const char *aFormat, ...);
    int error(const char *aFormat, ...);
    int fatal(const char *aFormat, ...);

    int logger(LoggingLevel log_level, const char *wFormat, const char *aFormat, ...);
    int send_log_packet(LoggingLevel log_level, const char *wFormat, const char *log_message, int log_size);

    void set_orchestrator(RemOrchestrator *remOrch_);

  protected:
    char log_buffer[REM_LOG_STRING_BUFFER_SIZE];

    uint8_t pb_buffer[REM_LOG_STRING_BUFFER_SIZE * 2];

    RemOrchestrator *remOrch;

  private:
};

#endif /* !REMLOGGER_HPP_ */

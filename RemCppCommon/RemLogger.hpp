#ifndef REMLOGGER_HPP_
#define REMLOGGER_HPP_

#define REM_LOG_STRING_BUFFER_SIZE 300


#include "mesh-packet.pb.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.hpp"

#include <cstdarg>
// #include <stdint.h>
// #ifdef ESP8266
// #else
// #endif

class RemOrchestrator;

class RemLogger
{
  public:
    RemLogger();
    ~RemLogger();

    // 0       Emergency           emerg
    // 1       Alert               alert
    // 2       Critical            crit
    // 3       Error               err
    // 4       Warning             warning
    // 5       Notice              notice
    // 6       Informational       info
    // 7       Debug               debug

    int emerg(const char *aFormat, ...);
    int alert(const char *aFormat, ...);
    int crit(const char *aFormat, ...);
    int error(const char *aFormat, ...);
    int warn(const char *aFormat, ...);
    int notice(const char *aFormat, ...);
    int info(const char *aFormat, ...);
    int debug(const char *aFormat, ...);

    int logger(Severity severity_level, const char *lTags, const char *aFormat, va_list args  );
    int send_log_packet(Severity severity_level, const char *lTags, const char *log_message, int log_size);

    void set_orchestrator(RemOrchestrator *remOrch_);

  protected:
    char log_buffer[REM_LOG_STRING_BUFFER_SIZE];

    uint8_t pb_buffer[REM_LOG_STRING_BUFFER_SIZE * 2];

    RemOrchestrator *remOrch;

  private:
};

#endif /* !REMLOGGER_HPP_ */

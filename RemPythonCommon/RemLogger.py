#!/usr/bin/env python3


import os


Severity_emerg   = 0
Severity_alert   = 1
Severity_crit    = 2
Severity_err     = 3
Severity_warning = 4
Severity_notice  = 5
Severity_info    = 6
Severity_debug   = 7
Severity_trace   = 8


RemOrchestrator = None

def set_orchestrator(remOrchestrator_):
    RemOrchestrator = remOrchestrator_

def emerg(message_):
    logger(Severity_emerg, "[EMERG]", message_);

def alert(message_):
    logger(Severity_alert, "[ALERT]", message_);

def crit(message_):
    logger(Severity_crit, "[CRIT]", message_);

def error(message_):
    logger(Severity_err, "[ERR]", message_);

def warn(message_):
    logger(Severity_warning, "[WARN]", message_);

def notice(message_):
    logger(Severity_notice, "[NOTICE]", message_);

def info(message_):
    logger(Severity_info, "[INFO]", message_);

def debug(message_):
    logger(Severity_debug, "[DEBUG]", message_ + f" \t{os.getpid()=}");

def trace(message_):
    logger(Severity_trace, "[TRACE]", message_);

# int logger(Severity severity_level, const char *lTags, const char *aFormat, va_list args  );
def logger(severity_, tags_, message_):
    print(f"{tags_} {message_}")

# int send_log_packet(Severity severity_level, const char *lTags, const char *log_message, int log_size);
def send_log_packet(severity_, tags_, message_):
    pass

def set_orchestrator():
    pass

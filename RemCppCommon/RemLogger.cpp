
#include "RemHeaderTypes.h"
#include <RemLogger.hpp>

#include <stdio.h>
#include <stdarg.h>

RemLogger::RemLogger()
{
}

RemLogger::~RemLogger()
{
}

int RemLogger::trace(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_trace, "[TRA]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::debug(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_debug, "[DBG]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::info(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_info, "[INF]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::warn(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_warn, "[WRN]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::error(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_error, "[ERR]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::fatal(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(LoggingLevel_fatal, "[FAT]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::logger(LoggingLevel log_level, const char *wFormat, const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);

    int asize, psize;
    asize = vsnprintf(log_buffer, REM_LOG_STRING_BUFFER_SIZE, aFormat, argptr);
    // wsize = snprintf(log_buffer, REM_LOG_STRING_BUFFER_SIZE, wFormat, asize, log_buffer_bk);

    psize = send_log_packet(log_level, wFormat, log_buffer, asize);
    printf("%s %d %d\t%s\n", wFormat,asize,psize, aFormat);

    va_end(argptr);
    return psize;
}

int RemLogger::send_log_packet(LoggingLevel log_level, const char *wFormat, const char *log_message, int log_size)
{
    // printf(" TIMEEEE %u \n", remOrch->basicHardware->time_milis());
    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    header->ForwardingType = ForwardingType_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;
    header->ProtobufType = ProtobufType_LOG;

    RespirMeshLog mesh_log_data = RespirMeshLog_init_default;
    mesh_log_data.source_time = remOrch->basicHardware->time_milis();
    mesh_log_data.source_id = remOrch->basicHardware->device_id();
    // mesh_log_data.type = ProtobufType_LOG;
    mesh_log_data.log_level = log_level;
    strcpy( mesh_log_data.tags , wFormat);
    strcpy( mesh_log_data.message , log_message);


    // logf(" *******  %d  %d  %d  \n", sizeof(*header), sizeof(RemDataHeaderByte), sizeof(RemBasicHeader));
    uint16_t offsetHeader = sizeof(*header);
    pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshLog_fields, &mesh_log_data);

    if (!pb_status)
    {
        errorf("Encoding Mesh Topology failed: %s\n", PB_GET_ERROR(&ostream));
        return -1;
    }
    uint16_t packet_size = ostream.bytes_written + offsetHeader;

    // funcf("Send LOG                     \t");
    // for (uint8_t i = 0; i < packet_size; i++)
    //     funcf("%d ", pb_buffer[i]);
    // funcf("\n");

    remOrch->remRouter->send_packet(pb_buffer, packet_size);
    // // handle_mesh_topo(pb_buffer, packet_size);
    return packet_size ;
}


void RemLogger::set_orchestrator(RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
};

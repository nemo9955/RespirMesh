
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

int RemLogger::emerg(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_emerg, "[EMERG]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::alert(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_alert, "[ALERT]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::crit(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_crit, "[CRIT]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::error(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_err, "[ERR]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::warn(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_warning, "[WARN]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::notice(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_notice, "[NOTICE]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::info(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_info, "[INFO]", aFormat, argptr);
    va_end(argptr);
    return rv;
}

int RemLogger::debug(const char *aFormat, ...)
{
    va_list argptr;
    va_start(argptr, aFormat);
    int rv = logger(Severity_debug, "[DEBUG]", aFormat, argptr);
    va_end(argptr);
    return rv;
}


int RemLogger::logger(Severity severity_level, const char *lTags, const char *aFormat, va_list args )
{
    va_list args1;
    // va_list args2;

    va_copy(args1, args);
    // va_copy(args2, args);

    // int asize = 1+std::vsnprintf(nullptr, 0, fmt, args1) ;
    int asize, psize;
    // weird segmentaion fault on vsnprintf if called before connections to clients is estalished
    // vprintf(aFormat, args2);

    asize = vsnprintf(log_buffer, sizeof(log_buffer), aFormat, args1);

    // va_end(args2);
    va_end(args1);
    psize = send_log_packet(severity_level, lTags, log_buffer, asize);
    printf("%s %d %d\t%s\n", lTags, asize, psize, log_buffer);



    return psize;
}

int RemLogger::send_log_packet(Severity severity_level, const char *lTags, const char *log_message, int log_size)
{
    // printf(" TIMEEEE %u \n", remOrch->basicHardware->time_milis());
    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    header->ForwardingType = ForwardingType_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;
    header->ProtobufType = ProtobufType_LOG;

    RespirMeshLog mesh_log_data = RespirMeshLog_init_default;
    mesh_log_data.source_timestamp = remOrch->basicHardware->time_milis();
    mesh_log_data.source_id = remOrch->basicHardware->device_id();
    // mesh_log_data.type = ProtobufType_LOG;
    mesh_log_data.severity = severity_level;
    // strcpy(mesh_log_data.tags, lTags);
    strcpy(mesh_log_data.message, log_message);

    // logf(" *******  %d  %d  %d  \n", sizeof(*header), sizeof(RemDataHeaderByte), sizeof(RemBasicHeader));
    uint16_t offsetHeader = sizeof(*header);
    pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshLog_fields, &mesh_log_data);

    if (!pb_status)
    {
        remOrch->log->error("Encoding Mesh Topology failed: %s\n", PB_GET_ERROR(&ostream));
        return -1;
    }
    uint16_t packet_size = ostream.bytes_written + offsetHeader;

    // funcf("Send LOG                     \t");
    // for (uint8_t i = 0; i < packet_size; i++)
    //     funcf("%d ", pb_buffer[i]);
    // funcf("\n");

    remOrch->remRouter->send_packet(pb_buffer, packet_size);
    // // handle_mesh_topo(pb_buffer, packet_size);
    return packet_size;
}

void RemLogger::set_orchestrator(RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
};

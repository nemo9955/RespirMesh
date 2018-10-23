/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9.1 at Tue Oct 23 21:52:25 2018. */

#include "mesh-packet.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t RespirMeshInfo_fields[4] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, RespirMeshInfo, target_id, target_id, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshInfo, source_id, target_id, 0),
    PB_FIELD(  3, UENUM   , SINGULAR, STATIC  , OTHER, RespirMeshInfo, type, source_id, 0),
    PB_LAST_FIELD
};

const pb_field_t RespirMeshLog_fields[6] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, RespirMeshLog, source_time, source_time, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshLog, source_id, source_time, 0),
    PB_FIELD(  4, UENUM   , SINGULAR, STATIC  , OTHER, RespirMeshLog, log_level, source_id, 0),
    PB_FIELD(  5, STRING  , SINGULAR, STATIC  , OTHER, RespirMeshLog, tags, log_level, 0),
    PB_FIELD(  6, STRING  , SINGULAR, STATIC  , OTHER, RespirMeshLog, message, tags, 0),
    PB_LAST_FIELD
};

const pb_field_t RespirMeshTimeSync_fields[7] = {
    PB_FIELD(  1, MESSAGE , SINGULAR, STATIC  , FIRST, RespirMeshTimeSync, info, info, &RespirMeshInfo_fields),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshTimeSync, request_sent_time, info, 0),
    PB_FIELD(  3, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshTimeSync, response_arrive_time, request_sent_time, 0),
    PB_FIELD(  4, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshTimeSync, response_sent_time, response_arrive_time, 0),
    PB_FIELD(  5, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshTimeSync, request_arrive_time, response_sent_time, 0),
    PB_FIELD(  6, UINT32  , SINGULAR, STATIC  , OTHER, RespirMeshTimeSync, server_timestamp_unix, request_arrive_time, 0),
    PB_LAST_FIELD
};






/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(RespirMeshTimeSync, info) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_RespirMeshInfo_RespirMeshLog_RespirMeshTimeSync)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(RespirMeshTimeSync, info) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_RespirMeshInfo_RespirMeshLog_RespirMeshTimeSync)
#endif


/* @@protoc_insertion_point(eof) */

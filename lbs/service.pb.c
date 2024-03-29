/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9.3 at Mon Jul 29 23:37:52 2019. */

#include "service.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t request_header_fields[2] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, request_header, request_id, request_id, 0),
    PB_LAST_FIELD
};

const pb_field_t response_header_fields[3] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, response_header, request_id, request_id, 0),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, response_header, error, request_id, 0),
    PB_LAST_FIELD
};

const pb_field_t echo_request_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, echo_request, header, header, &request_header_fields),
    PB_FIELD(  2, BYTES   , REQUIRED, STATIC  , OTHER, echo_request, data, header, 0),
    PB_LAST_FIELD
};

const pb_field_t echo_response_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, echo_response, header, header, &response_header_fields),
    PB_FIELD(  2, BYTES   , REQUIRED, STATIC  , OTHER, echo_response, data, header, 0),
    PB_LAST_FIELD
};

const pb_field_t add_disk_request_fields[5] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, add_disk_request, header, header, &request_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, add_disk_request, name, header, 0),
    PB_FIELD(  3, INT64   , REQUIRED, STATIC  , OTHER, add_disk_request, size, name, 0),
    PB_FIELD(  4, INT64   , REQUIRED, STATIC  , OTHER, add_disk_request, block_size, size, 0),
    PB_LAST_FIELD
};

const pb_field_t add_disk_response_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, add_disk_response, header, header, &response_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, add_disk_response, disk_id, header, 0),
    PB_LAST_FIELD
};

const pb_field_t stat_disk_request_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, stat_disk_request, header, header, &request_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, stat_disk_request, disk_id, header, 0),
    PB_LAST_FIELD
};

const pb_field_t stat_disk_response_fields[5] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, stat_disk_response, header, header, &response_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, stat_disk_response, name, header, 0),
    PB_FIELD(  3, INT64   , REQUIRED, STATIC  , OTHER, stat_disk_response, size, name, 0),
    PB_FIELD(  4, INT64   , REQUIRED, STATIC  , OTHER, stat_disk_response, block_size, size, 0),
    PB_LAST_FIELD
};

const pb_field_t write_disk_request_fields[5] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, write_disk_request, header, header, &request_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, write_disk_request, disk_id, header, 0),
    PB_FIELD(  3, INT64   , REQUIRED, STATIC  , OTHER, write_disk_request, offset, disk_id, 0),
    PB_FIELD(  4, BYTES   , REQUIRED, STATIC  , OTHER, write_disk_request, data, offset, 0),
    PB_LAST_FIELD
};

const pb_field_t write_disk_response_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, write_disk_response, header, header, &response_header_fields),
    PB_FIELD(  2, INT64   , REQUIRED, STATIC  , OTHER, write_disk_response, bytes_written, header, 0),
    PB_LAST_FIELD
};

const pb_field_t read_disk_request_fields[5] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, read_disk_request, header, header, &request_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, read_disk_request, disk_id, header, 0),
    PB_FIELD(  3, INT64   , REQUIRED, STATIC  , OTHER, read_disk_request, offset, disk_id, 0),
    PB_FIELD(  4, INT64   , REQUIRED, STATIC  , OTHER, read_disk_request, size, offset, 0),
    PB_LAST_FIELD
};

const pb_field_t read_disk_response_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, read_disk_response, header, header, &response_header_fields),
    PB_FIELD(  3, BYTES   , REQUIRED, STATIC  , OTHER, read_disk_response, data, header, 0),
    PB_LAST_FIELD
};

const pb_field_t sync_disk_request_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, sync_disk_request, header, header, &request_header_fields),
    PB_FIELD(  2, STRING  , REQUIRED, STATIC  , OTHER, sync_disk_request, disk_id, header, 0),
    PB_LAST_FIELD
};

const pb_field_t sync_disk_response_fields[2] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, sync_disk_response, header, header, &response_header_fields),
    PB_LAST_FIELD
};



/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
#error Field descriptor for read_disk_response.data is too large. Define PB_FIELD_32BIT to fix this.
#endif


/* @@protoc_insertion_point(eof) */

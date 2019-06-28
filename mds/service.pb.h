/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.9.3 at Fri Jun 28 13:58:58 2019. */

#ifndef PB_SERVICE_PB_H_INCLUDED
#define PB_SERVICE_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _request_type {
    request_type_echo = 1
} request_type;
#define _request_type_MIN request_type_echo
#define _request_type_MAX request_type_echo
#define _request_type_ARRAYSIZE ((request_type)(request_type_echo+1))

/* Struct definitions */
typedef PB_BYTES_ARRAY_T(65536) echo_request_data_t;
typedef struct _echo_request {
    echo_request_data_t data;
/* @@protoc_insertion_point(struct:echo_request) */
} echo_request;

typedef PB_BYTES_ARRAY_T(65536) echo_response_data_t;
typedef struct _echo_response {
    echo_response_data_t data;
/* @@protoc_insertion_point(struct:echo_response) */
} echo_response;

/* Default values for struct fields */

/* Initializer values for message structs */
#define echo_request_init_default                {{0, {0}}}
#define echo_response_init_default               {{0, {0}}}
#define echo_request_init_zero                   {{0, {0}}}
#define echo_response_init_zero                  {{0, {0}}}

/* Field tags (for use in manual encoding/decoding) */
#define echo_request_data_tag                    1
#define echo_response_data_tag                   1

/* Struct field encoding specification for nanopb */
extern const pb_field_t echo_request_fields[2];
extern const pb_field_t echo_response_fields[2];

/* Maximum encoded size of messages (where known) */
#define echo_request_size                        65540
#define echo_response_size                       65540

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define SERVICE_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif

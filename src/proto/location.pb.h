/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.3-dev */

#ifndef PB_LOCATION_V1_LOCATION_V1_LOCATION_PB_H_INCLUDED
#define PB_LOCATION_V1_LOCATION_V1_LOCATION_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _location_v1_GetLocationRequest {
    pb_callback_t payload;
} location_v1_GetLocationRequest;

typedef struct _location_v1_GetLocationResponse {
    float latitude;
    float longitude;
    float altitude;
} location_v1_GetLocationResponse;


/* Initializer values for message structs */
#define location_v1_GetLocationRequest_init_default {{{NULL}, NULL}}
#define location_v1_GetLocationResponse_init_default {0, 0, 0}
#define location_v1_GetLocationRequest_init_zero {{{NULL}, NULL}}
#define location_v1_GetLocationResponse_init_zero {0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define location_v1_GetLocationRequest_payload_tag 1
#define location_v1_GetLocationResponse_latitude_tag 1
#define location_v1_GetLocationResponse_longitude_tag 2
#define location_v1_GetLocationResponse_altitude_tag 3

/* Struct field encoding specification for nanopb */
#define location_v1_GetLocationRequest_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   payload,           1)
#define location_v1_GetLocationRequest_CALLBACK pb_default_field_callback
#define location_v1_GetLocationRequest_DEFAULT NULL

#define location_v1_GetLocationResponse_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    latitude,          1) \
X(a, STATIC,   SINGULAR, FLOAT,    longitude,         2) \
X(a, STATIC,   SINGULAR, FLOAT,    altitude,          3)
#define location_v1_GetLocationResponse_CALLBACK NULL
#define location_v1_GetLocationResponse_DEFAULT NULL

extern const pb_msgdesc_t location_v1_GetLocationRequest_msg;
extern const pb_msgdesc_t location_v1_GetLocationResponse_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define location_v1_GetLocationRequest_fields &location_v1_GetLocationRequest_msg
#define location_v1_GetLocationResponse_fields &location_v1_GetLocationResponse_msg

/* Maximum encoded size of messages (where known) */
/* location_v1_GetLocationRequest_size depends on runtime parameters */
#define location_v1_GetLocationRequest_size -1
#define location_v1_GetLocationResponse_size     15

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

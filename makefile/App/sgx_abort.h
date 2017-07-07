#pragma once

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

extern sgx_errlist_t sgx_errlist[];
extern void sgx_abort(sgx_status_t ret);

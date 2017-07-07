#include <cstdio>
#include "sgx_error.h"
#include "sgx_eid.h"
#include "sgx_urts.h"
#include "Enclave_u.h"
#include "sgx_abort.h"
#define TOKEN_FILENAME   "enclave.token"
#define ENCLAVE_FILENAME "enclave.signed.so"

sgx_enclave_id_t global_eid;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: ./app <name>\n");
        exit(1);
    }
    sgx_status_t ret;
    sgx_launch_token_t token = {0};
    int updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    const int len = 128;
    char buf[len];
    ret = get_greeting_string(global_eid, argv[1], buf, len);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    puts(buf);
}

#include <cmath>
#include <cstdio>
#include <chrono>
#include "sgx_error.h"
#include "sgx_eid.h"
#include "sgx_urts.h"
#include "Enclave_u.h"
#include "sgx_abort.h"
#define TOKEN_FILENAME   "enclave.token"
#define ENCLAVE_FILENAME "enclave.signed.so"

sgx_enclave_id_t global_eid;
uint8_t *g_untrusted_memory;

void untrusted_malloc(size_t len) {
    if (g_untrusted_memory != nullptr)
        puts("untrusted_malloc: g_untrusted_memory is not nullptr before malloc");
    g_untrusted_memory = new uint8_t[len];
    if (g_untrusted_memory == nullptr)
        puts("untrusted_malloc: g_untrusted_memory is still nullptr after malloc");
}

void untrusted_free() {
    delete [] g_untrusted_memory;
    g_untrusted_memory = 0;
}

void untrusted_read(uint32_t offset, uint8_t *out, size_t len) {
    memcpy(out, g_untrusted_memory + offset, len);
}

void untrusted_write(uint32_t offset, uint8_t *in, size_t len) {
    memcpy(g_untrusted_memory + offset, in, len);
}

double trusted_random_read_test(const size_t mem_size, const size_t access_range, const size_t op_size, const size_t chunk_size) {
    auto start = std::chrono::high_resolution_clock::now();
    sgx_status_t ret;

    ret = trusted_malloc(global_eid, mem_size);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    uint8_t buf[chunk_size];
    for (size_t n = 0; n < op_size; n += chunk_size) {
        uint32_t offset = rand() % (access_range - chunk_size);
        ret = trusted_read(global_eid, offset, buf, chunk_size);
        if (ret != SGX_SUCCESS) sgx_abort(ret);
    }

    ret = trusted_free(global_eid);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

double untrusted_random_read_test(const size_t mem_size, const size_t access_range, const size_t op_size, const size_t chunk_size) {
    auto start = std::chrono::high_resolution_clock::now();
    sgx_status_t ret;

    untrusted_malloc(mem_size);

    uint8_t buf[chunk_size];
    for (size_t n = 0; n < op_size; n += chunk_size) {
        uint32_t offset = rand() % (access_range - chunk_size);
        untrusted_read(offset, buf, chunk_size);
    }

    untrusted_free();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

int main() {
    sgx_status_t ret;
    sgx_launch_token_t token = {0};
    int updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    const size_t op_size = 1ULL<<31;
    const size_t chunk_size = 4<<10;
    for (double expon = 20; expon <= 28; expon += 0.2) {
        const size_t mem_size = static_cast<int>(std::pow(2, expon));
        double trusted = trusted_random_read_test(mem_size, mem_size, op_size, chunk_size);
        double untrusted = untrusted_random_read_test(mem_size, mem_size, op_size, chunk_size);
        printf("2^%.1f = %.2fMB: %fs/%fs = %.1fx slower\n", expon, mem_size / 1024.0 / 1024.0, trusted, untrusted, trusted / untrusted);
    }
}

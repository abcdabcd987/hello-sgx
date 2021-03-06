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
    sgx_status_t ret;

    ret = trusted_malloc(global_eid, mem_size);
    if (ret != SGX_SUCCESS)
        sgx_abort(ret);

    uint8_t buf[chunk_size];
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t n = 0; n < op_size; n += chunk_size)
    {
        uint32_t offset = rand() % (access_range - chunk_size);
        ret = trusted_read(global_eid, offset, buf, chunk_size);
        if (ret != SGX_SUCCESS) sgx_abort(ret);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    ret = trusted_free(global_eid);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

double untrusted_random_read_test(const size_t mem_size, const size_t access_range, const size_t op_size, const size_t chunk_size)
{
    untrusted_malloc(mem_size);

    uint8_t buf[chunk_size];
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t n = 0; n < op_size; n += chunk_size)
    {
        uint32_t offset = rand() % (access_range - chunk_size);
        untrusted_read(offset, buf, chunk_size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    untrusted_free();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

double trusted_random_write_test(const size_t mem_size, const size_t access_range, const size_t op_size, const size_t chunk_size) {
    sgx_status_t ret;

    ret = trusted_malloc(global_eid, mem_size);
    if (ret != SGX_SUCCESS)
        sgx_abort(ret);

    uint8_t buf[chunk_size];
    for (size_t i = 0; i < chunk_size; ++i)
        buf[i] = static_cast<uint8_t>(i % 255);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t n = 0; n < op_size; n += chunk_size)
    {
        uint32_t offset = rand() % (access_range - chunk_size);
        ret = trusted_write(global_eid, offset, buf, chunk_size);
        if (ret != SGX_SUCCESS) sgx_abort(ret);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    ret = trusted_free(global_eid);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

double untrusted_random_write_test(const size_t mem_size, const size_t access_range, const size_t op_size, const size_t chunk_size)
{
    untrusted_malloc(mem_size);

    uint8_t buf[chunk_size];
    for (size_t i = 0; i < chunk_size; ++i)
        buf[i] = static_cast<uint8_t>(i % 255);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t n = 0; n < op_size; n += chunk_size)
    {
        uint32_t offset = rand() % (access_range - chunk_size);
        untrusted_write(offset, buf, chunk_size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    untrusted_free();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

int main() {
    sgx_status_t ret;
    sgx_launch_token_t token = {0};
    int updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) sgx_abort(ret);

    const size_t op_size = 2ULL<<30;
    const size_t chunk_size = 4<<10;
    
    printf("read test, chunk size: %lu, total read per test: %lu\n", chunk_size, op_size);
    for (double expon = 20; expon <= 30; expon += 0.2)
    {
        const size_t mem_size = static_cast<size_t>(std::pow(2, expon));
        double trusted = trusted_random_read_test(mem_size, mem_size, op_size, chunk_size);
        double untrusted = untrusted_random_read_test(mem_size, mem_size, op_size, chunk_size);
        printf("2^%.1f = %.2fMB: %fs/%fs = %.1fx slower\n", expon, mem_size / 1024.0 / 1024.0, trusted, untrusted, trusted / untrusted);
    }

    printf("write test, chunk size: %lu, total write per test: %lu\n", chunk_size, op_size);
    for (double expon = 20; expon <= 30; expon += 0.2)
    {
        const size_t mem_size = static_cast<size_t>(std::pow(2, expon));
        double trusted = trusted_random_write_test(mem_size, mem_size, op_size, chunk_size);
        double untrusted = untrusted_random_write_test(mem_size, mem_size, op_size, chunk_size);
        printf("2^%.1f = %.2fMB: %fs/%fs = %.1fx slower\n", expon, mem_size / 1024.0 / 1024.0, trusted, untrusted, trusted / untrusted);
    }
}

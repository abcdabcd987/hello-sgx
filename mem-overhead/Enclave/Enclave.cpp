#include <cstdio>
#include <cstring>
#include <chrono>
#include "sgx_trts.h"
#include "Enclave_t.h"

uint8_t *g_enclave_memory;

void trusted_malloc(size_t len) {
    if (g_enclave_memory != nullptr)
        ocall_puts("trusted_malloc: g_enclave_memory is not nullptr before malloc");
    g_enclave_memory = new uint8_t[len];
    if (g_enclave_memory == nullptr)
        ocall_puts("trusted_malloc: g_enclave_memory is still nullptr after malloc");
}

void trusted_free() {
    delete [] g_enclave_memory;
    g_enclave_memory = 0;
}

void trusted_read(uint32_t offset, uint8_t *out, size_t len) {
    memcpy(out, g_enclave_memory + offset, len);
}

void trusted_write(uint32_t offset, uint8_t *in, size_t len) {
    memcpy(g_enclave_memory + offset, in, len);
}

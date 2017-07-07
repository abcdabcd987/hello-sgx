#include <cstdio>
#include "Enclave_t.h"

void get_greeting_string(const char *name, char *greeting, size_t len) {
    uint64_t magic;
    if (get_magic_number(&magic, name) != SGX_SUCCESS) abort();
    snprintf(greeting, len, "Hello %s! The magic number is 0x%016lx.", name, magic);
}

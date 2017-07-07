#include "Enclave_u.h"

uint64_t get_magic_number(const char *name) {
    uint64_t h = 0;
    for (const char *pc = name; *pc; ++pc)
        h = h * 31 + *pc;
    return h;
}

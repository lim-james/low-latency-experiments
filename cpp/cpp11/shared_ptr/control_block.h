#ifndef CONTROL_BLOCK
#define CONTROL_BLOCK

#include <stddef.h>
#include <atomic>

struct control_block {
    std::atomic<size_t> strong_count;
};

#endif

#include <string.h>

#include "cn-cbor/cn-cbor.h"
#include "memarray.h"

#define REGISTRY_CBOR_BLOCKS (64)

static cn_cbor _block_storage[REGISTRY_CBOR_BLOCKS];
static memarray_t _storage;

static void *_cbor_calloc(size_t count, size_t size, void *memblock);
static void _cbor_free(void *ptr, void *memblock);

cn_cbor_context ctx = {
    .calloc_func = _cbor_calloc,
    .free_func = _cbor_free,
    .context = &_storage
};

static void *_cbor_calloc(size_t count, size_t size, void *memblock)
{
    (void)count;
    void *block = memarray_alloc(memblock);
    if (block) {
        memset(block, 0, size);
    }
    return block;
}

static void _cbor_free(void *ptr, void *memblock)
{
    memarray_free(memblock, ptr);
}

cn_cbor_context *cbor_memarray_init(void)
{
    memarray_init(&_storage, _block_storage, sizeof(cn_cbor), REGISTRY_CBOR_BLOCKS);
    return &ctx;
}

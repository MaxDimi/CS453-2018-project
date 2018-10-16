/**
 * @file   tm.c
 * @author [...]
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section DESCRIPTION
 *
 * Implementation of your own transaction manager.
 * You can completely rewrite this file (and create more files) as you wish.
 * Only the interface (i.e. exported symbols and semantic) must be preserved.
**/

// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE   200809L
#ifdef __STDC_NO_ATOMICS__
    #error Current C11 compiler does not support atomic operations
#endif

// External headers
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// Internal headers
#include <tm.h>
#include "lock_t.h"

// extern const int EINVAL;
// extern const int ENOMEM;

// -------------------------------------------------------------------------- //

/** Define a proposition as likely true.
 * @param prop Proposition
**/
#undef likely
#ifdef __GNUC__
    #define likely(prop) \
        __builtin_expect((prop) ? 1 : 0, 1)
#else
    #define likely(prop) \
        (prop)
#endif

/** Define a proposition as likely false.
 * @param prop Proposition
**/
#undef unlikely
#ifdef __GNUC__
    #define unlikely(prop) \
        __builtin_expect((prop) ? 1 : 0, 0)
#else
    #define unlikely(prop) \
        (prop)
#endif

/** Define one or several attributes.
 * @param type... Attribute names
**/
#undef as
#ifdef __GNUC__
    #define as(type...) \
        __attribute__((type))
#else
    #define as(type...)
    #warning This compiler has no support for GCC attributes
#endif

// -------------------------------------------------------------------------- //

typedef struct region{
    void* start;
    size_t size;
    size_t align;
    lock_t lock;
} region;

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/
shared_t tm_create(size_t size as(unused), size_t align as(unused)) {
    printf("TM CREATE\n");
    region* reg = (region*) malloc(sizeof(region));
    if(reg==NULL){
        printf("[tm_create] Malloc failed\n");
        return invalid_shared;
    }

    if(! lock_init(&(reg->lock))) {
        printf("[tm_create] Lock failed\n");
        return invalid_shared;
    }

    if(align<sizeof(void *))
        align=sizeof(void*);

    int err = posix_memalign(&(reg->start), align, size);
    if(err != 0) {
    printf("[tm_create] Memalign failed\n");
        if(err==EINVAL){
            printf("The alignment argument was not a power of two, or was not a multiple of sizeof(void *).\n");
            printf("size: %d, align: %d, sizeof(void*): %d", size, align, sizeof(void *));
        }
        else if(err==ENOMEM)
            printf("There was insufficient memory to fulfill the allocation request.\n");
        free(reg);
        return invalid_shared;
    }

    reg->size = size;
    reg->align = align;

    return reg;
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
**/
void tm_destroy(shared_t shared as(unused)) {
    printf("TM DESTROY\n");
    lock_acquire(&(((region*) shared)->lock));
    region* reg = (region*) shared;
    lock_cleanup(&(reg->lock));
    free(reg->start);
    free(reg);
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
**/
void* tm_start(shared_t shared as(unused)) {
    return ((region*) shared)->start;
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
**/
size_t tm_size(shared_t shared as(unused)) {
    return ((region*) shared)->size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
**/
size_t tm_align(shared_t shared as(unused)) {
    return ((region*) shared)->align;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @return Opaque transaction ID, 'invalid_tx' on failure
**/
tx_t tm_begin(shared_t shared as(unused)) {
    printf("Begin acquire\n");
    if(shared==NULL || &(((region*) shared)->lock)==NULL)
        return invalid_tx;
    lock_acquire(&(((region*) shared)->lock));
    printf("Acquired\n");
    return ~(invalid_tx);
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
**/
bool tm_end(shared_t shared as(unused), tx_t tx as(unused)) {
    printf("Begin release\n");
    if(shared==NULL || tx==invalid_tx)
        return false;
    lock_release(&(((region*) shared)->lock));
    printf("Released\n");
    tx = invalid_tx;
    return true;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
**/
bool tm_read(shared_t shared as(unused), tx_t tx as(unused), void const* source as(unused), size_t size as(unused), void* target as(unused)) {

    printf("Begin read\n");
    region* reg = (region*) shared;
    // if((reg==NULL) || (source==NULL) || (target==NULL) || (tx==invalid_tx) ||
    //    (size<0) || (size % reg->align!=0) || (source+size>reg->start+reg->size) ||
    //    (source < reg->start))
    //     return false;

    memcpy(target, source, size);
    printf("End read\n");

    return true;
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
**/
bool tm_write(shared_t shared as(unused), tx_t tx as(unused), void const* source as(unused), size_t size as(unused), void* target as(unused)) {

    printf("Begin write\n");
    region* reg = (region*) shared;

    // if((reg==NULL) || (source==NULL) || (target==NULL) || (tx==invalid_tx) ||
    //    (size<0) || (size % reg->align!=0) || (target+size>reg->start+reg->size) ||
    //    (target < reg->start)){
    //     printf("oops\n");
    //     return false;
    //    }


    printf("memcpy write\n");
    memcpy(target, source, size);

    printf("End write\n");

    return true;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
**/
alloc_t tm_alloc(shared_t shared as(unused), tx_t tx as(unused), size_t size as(unused), void** target as(unused)) {
    // TODO: tm_alloc(shared_t, tx_t, size_t, void**)
    printf("!!! tm_alloc !!!\n");
    return abort_alloc;
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
**/
bool tm_free(shared_t shared as(unused), tx_t tx as(unused), void* target as(unused)) {
    // TODO: tm_free(shared_t, tx_t, void*)
    printf("!!! tm_free !!!\n");
    return false;
}

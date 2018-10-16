// External headers
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

// Internal headers
#include "lock_t.h"
// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――


/** Lock initialization.
 * @param lock Uninitialized lock structure
 * @return Whether initialization is a success
**/
bool lock_init(lock_t* lock __attribute__((unused))) {
    // Code here
    lock->locked = false;
    return true;
}

/** Lock clean up.
 * @param lock Initialized lock structure
**/
void lock_cleanup(lock_t* lock __attribute__((unused))) {
    // Code here
}

/** [thread-safe] Acquire the given lock, wait if it has already been acquired.
 * @param lock Initialized lock structure
**/
void lock_acquire(lock_t* lock __attribute__((unused))) {
    printf("Lock balise 1\n");
    bool b = false;
    int count = 0;
    //bool* b = (bool*) malloc(sizeof(bool));
    while (! atomic_compare_exchange_strong(&(lock->locked), &b, true)){
    //while (! atomic_compare_exchange_strong(&(lock->locked), b, true)){
        count++;
        printf("[%d] %d - Lock balise 2\n", lock, count);
        if(lock==NULL)
            printf("WTF\n");
        printf("[%d] %d - Lock balise 2 %d\n", lock, count, b);
        b = false;
        printf("[%d] %d - Lock balise 2.5 %d\n", lock, count, b);
        // usleep(100);
    }
    printf("[%d] %d - Lock balise 3\n", lock, count);
    //free(b);
}

/** [thread-safe] Release the given lock.
 * @param lock Initialized lock structure
**/
void lock_release(lock_t* lock __attribute__((unused))) {
    bool b = true;
    atomic_compare_exchange_strong(&(lock->locked), &b, false);
}

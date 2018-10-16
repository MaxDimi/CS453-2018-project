/**
 * @file   entrypoint.h
 * @author Sébastien Rouault <sebastien.rouault@epfl.ch>
 *
 * @section LICENSE
 *
 * Copyright © 2018 Sébastien Rouault.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version. Please see https://gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * @section DESCRIPTION
 *
 * Interface for the entry point.
**/

#pragma once

// External headers
#include <stdbool.h>
#include <stddef.h> // true = 1, false = 0
#include <stdlib.h>

// typedef int bool;
// #define true 1
// #define false 0
// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

/** Your lock type.
**/
typedef struct lock_t {
    _Atomic bool locked;
} lock_t;

bool lock_init(lock_t*);
void lock_cleanup(lock_t*);
void lock_acquire(lock_t*);
void lock_release(lock_t*);

// ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――

void entry_point(size_t, size_t, lock_t*);

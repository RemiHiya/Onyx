//
// Created by remsc on 19/06/2025.
//

#ifndef POOL_H
#define POOL_H

#include <stdlib.h>

#define alloc(size) ({ \
    void* __ptr = malloc(size); \
    poolAdd(currentPool, __ptr); \
    __ptr; \
})

typedef struct {
    void** pool;
    size_t size;
} pool;

typedef struct {
    pool** stack;
    size_t size;
} poolStack;

poolStack* poolStackInit();
void poolStackFree(poolStack* stack);

void poolStackNew(poolStack* stack);
void poolStackPop(poolStack* stack);

pool* poolInit();
void poolAdd(pool* pool, void* ptr);
void poolRemove(pool* pool, void* ptr);
void poolFree(pool* pool);


#endif //POOL_H

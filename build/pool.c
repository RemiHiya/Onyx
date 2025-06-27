//
// Created by remsc on 19/06/2025.
//

#include <stdlib.h>
#include "pool.h"

poolStack* poolStackInit() {
    poolStack* stack = malloc(sizeof(poolStack));
    stack->size = 0;
    stack->stack = malloc(0);
    return stack;
}

void poolStackFree(poolStack* stack) {
    for (int i=0; i<stack->size; ++i) {
        poolFree(stack->stack[i]);
    }
    free(stack->stack);
    free(stack);
}

void poolStackNew(poolStack* stack) {
    stack->stack = realloc(stack->stack, (stack->size + 1) * sizeof(pool*));
    stack->stack[stack->size] = poolInit();
    stack->size++;
}

// Should free all memory in the current scope
void poolStackPop(poolStack* stack) {
    if (stack->size == 0) return; // Sécurité
    poolFree(stack->stack[stack->size - 1]);
    stack->size--;
    stack->stack = realloc(stack->stack, stack->size * sizeof(pool*));
}

pool* poolInit() {
    pool* pool = malloc(sizeof(pool));
    pool->size = 0;
    pool->pool = malloc(0);
    return pool;
}

void poolAdd(pool* pool, void* ptr) {
    pool->size++;
    pool->pool = realloc(pool->pool, pool->size * sizeof(void*));
    pool->pool[pool->size - 1] = ptr;
}

void poolFree(pool* pool) {
    for (int i=0; i<pool->size; ++i) {
        free(pool->pool[i]);
    }
    free(pool->pool);
    free(pool);
}

void poolRemove(pool* pool, void* ptr) {
    size_t index = (size_t)-1;

    for (size_t i = 0; i < pool->size; ++i) {
        if (pool->pool[i] == ptr) {
            index = i;
            break;
        }
    }
    if (index == (size_t)-1) return;

    for (size_t i = index; i < pool->size - 1; ++i) {
        pool->pool[i] = pool->pool[i + 1];
    }

    pool->size--;
    pool->pool = realloc(pool->pool, pool->size * sizeof(void*));
}


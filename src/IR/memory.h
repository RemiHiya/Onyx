//
// Created by remsc on 08/07/2025.
//

#ifndef MEMORY_H
#define MEMORY_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Node for linked list of pointeurs
typedef struct VoidPtrNode {
    void* ptr;
    struct VoidPtrNode* next;
} VoidPtrNode;

// Hash table node
typedef struct HashTableEntry {
    void* key_ptr;
    int value_int;
    struct HashTableEntry* next;
} HashTableEntry;

typedef struct PtrIntList {
    VoidPtrNode** int_to_ptr_lists; // Array of linked lists
    int max_int_value;              // Max int level

    HashTableEntry** ptr_to_int_table; // Hash table, void* -> int
    int hash_table_capacity;
    int hash_table_size;            // Table size

    int current_scope;
} PtrIntList;


void enter_scope_impl(PtrIntList* list);
bool register_ptr_impl(PtrIntList* list, void* ptr);
bool exit_scope_impl(PtrIntList* list);
bool move_ptr_impl(PtrIntList* list, void* ptr, int new_scope_level);

// API :
extern PtrIntList* global_pool;
#define enterScope() enter_scope_impl(global_pool)
#define register_ptr(ptr) register_ptr_impl(global_pool, ptr)
#define exitScope() exit_scope_impl(global_pool)
#define move_ptr(ptr, new_scope_level) move_ptr_impl(global_pool, ptr, new_scope_level)

#define initGlobalPool(initial_max_int_value, hash_table_capacity) \
(global_pool = create_ptr_int_list_impl(initial_max_int_value, hash_table_capacity))

#define destroyGlobalPool() \
    do { \
        destroy_ptr_int_list_impl(global_pool); \
        global_pool = NULL; \
    } while(0)

#define alloc(size) \
    ({ \
    void* allocated_ptr = malloc(size); \
    if (allocated_ptr) { \
        if (!register_ptr(allocated_ptr)) { \
            fprintf(stderr, "Error: Failed to register allocated pointer %p.\n", allocated_ptr); \
            free(allocated_ptr); \
            allocated_ptr = NULL; \
        } \
    } else { \
        fprintf(stderr, "Error: Malloc failed for size %zu.\n", (size_t)size); \
    } \
    allocated_ptr; \
    })


#endif //MEMORY_H

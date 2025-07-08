//
// Created by remsc on 08/07/2025.
//

#include "memory.h"

// Hash function for void*
unsigned int hash_void_ptr(void* ptr, const int capacity) {
    return ((unsigned long)ptr >> 3) % capacity;
}

PtrIntList* create_ptr_int_list(const int initial_max_int_value, const int hash_table_capacity) {
    PtrIntList* list = malloc(sizeof(PtrIntList));
    if (!list) return NULL;

    list->max_int_value = initial_max_int_value > 0 ? initial_max_int_value : 10; // Ensure at least 10
    list->hash_table_capacity = hash_table_capacity > 0 ? hash_table_capacity : 100; // Ensure at least 100
    list->hash_table_size = 0;
    list->current_scope = 0;

    // Allocation for linked list
    list->int_to_ptr_lists = (VoidPtrNode**)calloc(list->max_int_value + 1, sizeof(VoidPtrNode*));
    if (!list->int_to_ptr_lists) {
        free(list);
        return NULL;
    }

    // Allocation for hash table
    list->ptr_to_int_table = (HashTableEntry**)calloc(list->hash_table_capacity, sizeof(HashTableEntry*));
    if (!list->ptr_to_int_table) {
        free(list->int_to_ptr_lists);
        free(list);
        return NULL;
    }
    return list;
}

// Return true if the resize succeeded
static bool resize_int_list_array(PtrIntList* list, const int new_max_int_value) {
    if (new_max_int_value <= list->max_int_value) return true;

    VoidPtrNode** new_array = realloc(list->int_to_ptr_lists, (new_max_int_value + 1) * sizeof(VoidPtrNode*));
    if (!new_array) return false;

    // Initialize new memory to NULL
    for (int i = list->max_int_value + 1; i <= new_max_int_value; ++i) {
        new_array[i] = NULL;
    }

    list->int_to_ptr_lists = new_array;
    list->max_int_value = new_max_int_value;
    return true;
}

// Resize the hash table
static bool resize_hash_table(PtrIntList* list) {
    const int old_capacity = list->hash_table_capacity;
    const int new_capacity = old_capacity * 2; // Double the capacity

    HashTableEntry** old_table = list->ptr_to_int_table;
    HashTableEntry** new_table = calloc(new_capacity, sizeof(HashTableEntry*));
    if (!new_table) return false;

    list->ptr_to_int_table = new_table;
    list->hash_table_capacity = new_capacity;
    list->hash_table_size = 0; // Reset size

    // Rehash all entries from the old table into the new table
    for (int i = 0; i < old_capacity; ++i) {
        HashTableEntry* entry = old_table[i];
        while (entry != NULL) {
            // Re-insert into the new hash table
            const unsigned int index = hash_void_ptr(entry->key_ptr, list->hash_table_capacity);
            HashTableEntry* new_entry = malloc(sizeof(HashTableEntry));
            if (!new_entry) {
                fprintf(stderr, "Error: Failed to allocate memory during hash table resize.\n");
                return false;
            }
            new_entry->key_ptr = entry->key_ptr;
            new_entry->value_int = entry->value_int;
            new_entry->next = list->ptr_to_int_table[index];
            list->ptr_to_int_table[index] = new_entry;
            list->hash_table_size++;

            HashTableEntry* temp = entry;
            entry = entry->next;
            free(temp); // Free the old entry
        }
    }
    free(old_table); // Free the old table
    return true;
}

// Adds or updates an association (void* -> int)
// Returns true on success, false on failure (e.g., memory allocation error)
bool add_or_update_association(PtrIntList* list, void* ptr, int value) {
    if (!list || value < 0) return false;

    // 1. Update/Add to the hash table
    const unsigned int hash_index = hash_void_ptr(ptr, list->hash_table_capacity);
    HashTableEntry* current_entry = list->ptr_to_int_table[hash_index];
    HashTableEntry* existing_entry = NULL;

    // Check if ptr already exists in the hash table
    while (current_entry != NULL) {
        if (current_entry->key_ptr == ptr) {
            existing_entry = current_entry;
            break;
        }
        current_entry = current_entry->next;
    }

    int old_value = -1; // To store the old value if updated

    if (existing_entry != NULL) {
        // Update existing entry
        old_value = existing_entry->value_int;
        existing_entry->value_int = value;
    } else {
        // Add new entry
        HashTableEntry* new_hash_entry = malloc(sizeof(HashTableEntry));
        if (!new_hash_entry) return false;
        new_hash_entry->key_ptr = ptr;
        new_hash_entry->value_int = value;
        new_hash_entry->next = list->ptr_to_int_table[hash_index];
        list->ptr_to_int_table[hash_index] = new_hash_entry;
        list->hash_table_size++;

        // Resize hash table if load factor is too high
        if ((double)list->hash_table_size / list->hash_table_capacity > 0.75) {
            if (!resize_hash_table(list)) {
                fprintf(stderr, "Warning: Failed to resize hash table.\n");
            }
        }
    }

    // 2. Update the list of ptr
    // Remove ptr from its old value's list if it was updated
    if (existing_entry != NULL && old_value != value) {
        if (old_value >= 0 && old_value <= list->max_int_value) {
            VoidPtrNode* current = list->int_to_ptr_lists[old_value];
            VoidPtrNode* prev = NULL;
            while (current != NULL && current->ptr != ptr) {
                prev = current;
                current = current->next;
            }
            if (current != NULL) { // Found the ptr in the old list
                if (prev == NULL) { // It was the head
                    list->int_to_ptr_lists[old_value] = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current); // Free the old node from the list
            }
        }
    }

    // Resize the int_to_ptr_lists array if 'value' is too large
    if (value > list->max_int_value) {
        if (!resize_int_list_array(list, value)) { // Adjust the capacity to "value"
            fprintf(stderr, "Warning: Failed to resize integer list array.\n");
            // Still proceed with adding to hash table if successful, but list might be truncated
        }
    }

    // Add ptr to the new value's list (only if it's a new association or value changed)
    if (existing_entry == NULL || old_value != value) {
        VoidPtrNode* new_list_node = malloc(sizeof(VoidPtrNode));
        if (!new_list_node) {
            fprintf(stderr, "Error: Failed to allocate memory for list node. Data structure inconsistent.\n");
            return false;
        }
        new_list_node->ptr = ptr;
        new_list_node->next = list->int_to_ptr_lists[value];
        list->int_to_ptr_lists[value] = new_list_node;
    }
    return true;
}

// Lookup for the associated int for a ptr
int lookup_int_by_ptr(PtrIntList* list, void* ptr) {
    if (!list || !ptr) return -1;

    const unsigned int hash_index = hash_void_ptr(ptr, list->hash_table_capacity);
    const HashTableEntry* current = list->ptr_to_int_table[hash_index];

    while (current != NULL) {
        if (current->key_ptr == ptr) {
            return current->value_int;
        }
        current = current->next;
    }
    return -1; // Not found
}

// Get a pointer to the head of the requested list
VoidPtrNode* get_ptrs_by_int(PtrIntList* list, const int value) {
    if (!list || value < 0 || value > list->max_int_value) {
        return NULL;
    }
    return list->int_to_ptr_lists[value];
}

// Removes an association (void* -> int)
bool remove_association(PtrIntList* list, void* ptr) {
    if (!list || !ptr) return false;

    // 1. Remove from Hash Table
    const unsigned int hash_index = hash_void_ptr(ptr, list->hash_table_capacity);
    HashTableEntry* current_hash_entry = list->ptr_to_int_table[hash_index];
    HashTableEntry* prev_hash_entry = NULL;
    int value_to_remove = -1;

    while (current_hash_entry != NULL && current_hash_entry->key_ptr != ptr) {
        prev_hash_entry = current_hash_entry;
        current_hash_entry = current_hash_entry->next;
    }
    if (current_hash_entry == NULL) {
        return false; // ptr not found in hash table
    }

    value_to_remove = current_hash_entry->value_int;

    if (prev_hash_entry == NULL) { // Head of the list
        list->ptr_to_int_table[hash_index] = current_hash_entry->next;
    } else {
        prev_hash_entry->next = current_hash_entry->next;
    }
    free(current_hash_entry);
    list->hash_table_size--;

    // 2. Remove from int_to_ptr_lists
    if (value_to_remove != -1 && value_to_remove <= list->max_int_value) {
        VoidPtrNode* current_list_node = list->int_to_ptr_lists[value_to_remove];
        VoidPtrNode* prev_list_node = NULL;

        while (current_list_node != NULL && current_list_node->ptr != ptr) {
            prev_list_node = current_list_node;
            current_list_node = current_list_node->next;
        }

        if (current_list_node != NULL) { // Found in the list
            if (prev_list_node == NULL) { // Head of the list
                list->int_to_ptr_lists[value_to_remove] = current_list_node->next;
            } else {
                prev_list_node->next = current_list_node->next;
            }
            free(current_list_node);
            return true;
        }
    }
    return false; // Shouldn't be reached
}

// Frees the structure
void destroy_ptr_int_list(PtrIntList* list) {
    if (!list) return;

    // Free int_to_ptr_lists
    if (list->int_to_ptr_lists) {
        for (int i = 0; i <= list->max_int_value; ++i) {
            VoidPtrNode* current = list->int_to_ptr_lists[i];
            while (current != NULL) {
                VoidPtrNode* temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(list->int_to_ptr_lists);
    }

    // Free hash table entries
    if (list->ptr_to_int_table) {
        for (int i = 0; i < list->hash_table_capacity; ++i) {
            HashTableEntry* current = list->ptr_to_int_table[i];
            while (current != NULL) {
                HashTableEntry* temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(list->ptr_to_int_table);
    }
    free(list);
}

void enter_scope_impl(PtrIntList* list) {
    if (!list) {
        fprintf(stderr, "Error: _enter_scope_impl called with NULL list.\n");
        return;
    }
    list->current_scope++;
    if (list->current_scope > list->max_int_value) {
        if (!resize_int_list_array(list, list->current_scope * 2)) {
            fprintf(stderr, "Warning: Failed to resize integer list array during enterScope. Max scope reached.\n");
        }
    }
}

bool register_ptr_impl(PtrIntList* list, void* ptr) {
    if (!list) {
        fprintf(stderr, "Error: _register_ptr_impl called with NULL list.\n");
        return false;
    }
    if (!ptr) {
        fprintf(stderr, "Error: _register_ptr_impl called with NULL ptr.\n");
        return false;
    }
    return add_or_update_association(list, ptr, list->current_scope);
}

bool exit_scope_impl(PtrIntList* list) {
    if (!list) {
        fprintf(stderr, "Error: _exit_scope_impl called with NULL list.\n");
        return false;
    }
    if (list->current_scope == 0) {
        fprintf(stderr, "Warning: Attempted to exit scope 0. Operation skipped.\n");
        return false;
    }

    const int scope_to_exit = list->current_scope;
    const VoidPtrNode* current_node = list->int_to_ptr_lists[scope_to_exit];
    list->int_to_ptr_lists[scope_to_exit] = NULL; // Detach the list head

    while (current_node != NULL) {
        void* ptr_to_free = current_node->ptr;
        VoidPtrNode* temp_next = current_node->next;

        if (!remove_association(list, ptr_to_free)) {
            fprintf(stderr, "Error: Failed to remove association for ptr %p during exitScope.\n", ptr_to_free);
        }
        free(ptr_to_free);
        current_node = temp_next;
    }

    list->current_scope--;
    return true;
}

bool move_ptr_impl(PtrIntList* list, void* ptr, const int new_scope_level) {
    if (!list) {
        fprintf(stderr, "Error: _move_ptr_impl called with NULL list.\n");
        return false;
    }
    if (!ptr) {
        fprintf(stderr, "Error: _move_ptr_impl called with NULL ptr.\n");
        return false;
    }
    if (new_scope_level < 0) {
        fprintf(stderr, "Error: _move_ptr_impl called with negative new_scope_level (%d).\n", new_scope_level);
        return false;
    }
    return add_or_update_association(list, ptr, new_scope_level);
}
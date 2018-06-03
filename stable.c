/*******************************************************************************
 *    stable.c
 *        A symbol table associating generic data to strings implementation.
 ******************************************************************************/

/* Includes & Macros */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stable.h"

#define EXISTING 0
#define NEW 1
#define GROW 1

/* Data Structures */
typedef struct Node {
    char* key;                                  // key
    EntryData *val;                             // value
    struct Node *next;                          // next item on the linked list
} Node;

struct stable_s {
    int n;                                      // number of items on st
    int size;                                   // st capacity
    Node **table;                               // pointer to array of pointers
};

/**
 *  stable_create():
 *      Creates a new empty symbol table with size 1 and returns a pointer to it
 */
SymbolTable
stable_create() {
    SymbolTable st = malloc(sizeof(struct stable_s));
    if (!st) {
        fprintf(stderr, "stable_create(): cannot create new stable!\n");
        return NULL;
    }
    st -> table = malloc(sizeof(Node*) * 20357);
    if (!st -> table) {
        fprintf(stderr, "stable_create(): cannot create new st -> table!\n");
        return NULL;
    }
    st -> n = 0;
    st -> size = 20357;
    st -> table[0] = NULL;
    return st;
}

/**
*  create_with_capacity():
*      Creates a new empty symbol table with given size.
*/
static Node**
create_table(int capacity) {
    Node **table = malloc(sizeof(Node*) * capacity);
    if (!table) {
        fprintf(stderr, "create_create(): cannot create new table!\n");
        return NULL;
    }
    table[0] = NULL;
    return table;
}

/**
 *  stable_destroy():
 *      Destroy a given symbol table.
 */
void
stable_destroy(SymbolTable st) {
    for (int i = 0; i < st -> size; i++) {
        if (st -> table[i] != NULL) {
            free(st -> table[i] -> key);
            free(st -> table[i] -> val);
            free(st -> table[i]);
        }
    }
    free(st -> table);
    free(st);
}

/**
 *  hash():
 *      Returns a hash of the given key.
 */
static int
hash(int st_size, const char *key) {
    unsigned long int hash = 0;
    int i = 0;
    int len = strlen(key);
    
    /* Hashing pelo método da divisão */
    while (hash < ULONG_MAX && i < len) {
        hash = (hash * 256 + key[i]) % st_size;
        i++;
    }
//    printf("key = %s, hashval = %d\n", key, hash);
    return hash;
}

/**
 *  newnode():
 *      Returns a pointer to a new node.
 */
static Node *
newnode(const char *key) {
    /* allocates space for *char, EntryData and *Node */
    Node *n = malloc(sizeof(Node));
    /* error checking */
    if (!n) {
        fprintf(stderr, "newnode(): cannot malloc new node!\n");
        return NULL;
    }
    /* copies key; error check? */
    n -> key = malloc(sizeof(char) * strlen(key));
    strcpy(n -> key, key);
    /* allocate spsace for EntryData */
    n -> val = malloc(sizeof(EntryData));
    if (!n -> val) {
        fprintf(stderr, "newnode(): cannot malloc space for data!\n");
        return NULL;
    }
    /* makes sure the next node is null */
    n -> next = NULL;
    return n;
}

/**
 *  newresult():
 *      Returns a InsertionResult struct.
 */
static InsertionResult *
newresult(int flag, Node *node) {
    /* allocates space for struct */
    InsertionResult *r = malloc(sizeof(InsertionResult));
    if (!r) {
        fprintf(stderr, "newresult(): cannot malloc space for result!\n");
        return NULL;
    }
    r -> new = flag;
    r -> data = node -> val;
    return r;
}

/**
 *  rehash():
 *      Keeps the alfa factor under 10
 */
static void
rehash(SymbolTable st, int grow) {
    SymbolTable new_st = malloc(sizeof(struct stable_s));
    if (grow) {
        new_st -> table = create_table(st -> size * 2);
        new_st -> n = 0;
        new_st -> size = st -> size * 2;
        
        for (int i = 0; i < st -> size; i++) {
            Node *current = st -> table[i];
            while (current != NULL) {
                stable_insert(new_st, current -> key);
                /* COPIAR DADOS DO INSERTION RESULT PRA NOVA ST */
                current = current -> next;
            }
        }
    }
    st -> size = st -> size * 2;
    st -> n = new_st -> n;
    st -> table = new_st -> table;
    return;
}

/**
 *  stable_insert():
 *      Insert a new entry on the symbol table given its key.
 *      If there is already an entry with the given key, then a struct
 *      InsertionResult is returned with new == 0 and data pointing to the
 *      data associated with the entry. Otherwise, a struct is returned with
 *      new != 0 and data pointing to the data field of the new entry.
 *
 *      If there is not enough space on the table, or if there is a memory
 *      allocation error, then crashes with an error message.
 */
InsertionResult
stable_insert(SymbolTable st, const char *key) {
    /* keeps alpha under 10 by rehashing */
//    if (st -> size > 0 && st -> n / st -> size >= 10) {
//        rehash(st, GROW);
//    }
    int hashval = hash(st -> size, key);
    /* if there's an entry with the given hash */
    if (st -> table[hashval] != NULL) {
        Node *current = st -> table[hashval];
        /* node points at the first entry */
        while (current != NULL) {
            /* is this the key we're looking for? */
            if (strncmp(current -> key, key, sizeof(*key)) == 0) {
//                printf("key = %s, hashval = %d\n", key, hashval);
                return *newresult(EXISTING, current);
            }
            /* it's not */
            if (current -> next == NULL) {
                /* we have found empty space, let's put it here */
                current -> next = newnode(key);
                st -> n++;
                return *newresult(NEW, current -> next);
            }
            /* go to next node on the linked list */
            else current = current -> next;
        }
    }
    /* first entry is NULL */
    st -> table[hashval] = newnode(key);
    st -> n++;
    return *newresult(NEW, st -> table[hashval]);
}

/**
 *  stable_find():
 *      Find the data associated with a given key.
 *
 *      Given a key, returns a pointer to the data associated with it, or a
 *      NULL pointer if the key is not found.
 */
EntryData *
stable_find(SymbolTable st, const char *key) {
    int hashval = hash(st -> size, key);
    /* if there's an entry with the given hash */
    if (st -> table[hashval] != NULL) {
        Node *current = st -> table[hashval];
        /* node points at the first entry */
        while (current != NULL) {
            /* is this the key we're looking for? */
            if (strncmp(current -> key, key, sizeof(*key)) == 0)
                return current -> val;
            /* it's not */
            if (current -> next == NULL)
                /* there's nowhere else to look, return NULL */
                return NULL;
            /* go to next node on the linked list */
            else current = current -> next;
        }
    }
    /* nothing on the first entry */
    return NULL;
}

/**
 *  stable_visit():
 *      Visit each entry on the table.
 *
 *      The visit function is called on each entry, with pointers to its key
 *      and data. If the visit function returns zero, then the iteration
 *      stops.
 *      Returns zero if the iteration was stopped by the visit function,
 *      nonzero otherwise.
 */
int
stable_visit(SymbolTable st, int (*visit)(const char *key, EntryData *data)) {
    /* let's iterate every node on the st */
    Node *current;
    for (int i = 0; i < st -> size; i++) {
        current = st -> table[i];
        if (current != NULL) {
            /* node points at the first entry */
            while (current != NULL) {
                /* calls the visit function */
                if ((*visit)(current -> key, current -> val) == 0)
                    return 0;
                /* go to next node on the linked list */
                else current = current -> next;
            }
        }
    }
    return 1;
}

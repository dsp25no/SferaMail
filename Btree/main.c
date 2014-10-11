//
//  main.c
//  Project-DBMS
//
//  Created by Дмитрий Павлов on 27.09.14.
//  Copyright (c) 2014 Дмитрий Павлов. All rights reserved.
//

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "lib.h"

typedef struct Btree Btree;
typedef struct Btree_node Btree_node;
typedef struct DBT DBT;
typedef struct DB DB;
typedef struct DBC DBC;

int
write_to_file(Btree_node *node)
{
    int fd = open(node->tree->name_of_file, O_RDWR);
    lseek(fd, node->tree->offset_of_data + node->Id * node->tree->chunk_size, SEEK_SET);
    write(fd, node, sizeof(node));
    write(fd, node->keys, node->keys->size);
    write(fd, node->data, node->data->size);
    free(node);
    return 1;
}

int
read_from_file(Btree_node *node)
{
    int fd = open(node->tree->name_of_file, O_RDWR);
    lseek(fd, node->tree->offset_of_data + node->Id * node->tree->chunk_size, SEEK_SET);
    read(fd, node, sizeof(node));
    DBT *data = malloc(sizeof(*data));
    DBT *keys = malloc(sizeof(*keys));
    keys->size = node->offset_key[2 * t - 1] - node->offset_key[0];
    data->size = node->offset_data[2 * t - 1] - node->offset_data[0];
    keys->data = calloc(1, keys->size);
    data->data = calloc(1, data->size);
    read(fd, keys, keys->size);
    read(fd, data, data->size);
    node->data = data;
    node->keys = keys;
    return 1;
}

Btree_node
*allocate_node()
{
    
}

struct DB
*dbcreate(const char *file, const struct DBC conf)
{
    int fd = truncate(file, conf.db_size);
    DB *data_base = calloc(1, sizeof(*data_base));
    Btree *tree = calloc(1, sizeof(*tree));
    data_base->btree = tree;
    tree->name_of_file = file;
    tree->read = read_from_file;
    tree->write = write_to_file;
//tree->offset =
    Btree_node *head = calloc(1, sizeof(*head));
    head->tree = tree;
    tree->head = head;
    tree->is_empty = 1;
    tree->chunk_size = conf.chunk_size;
/*
* Write header of file
*/
    return data_base;
}

DBT
*search(Btree_node *node, DBT *key)
{
    int i = 0;
    int compare;
    while (i < node->n && (compare = memcmp(key->data, node->keys->data + node->offset_key[i],
                                      min(key->size, node->offset_key[i + 1] - node->offset_key[i])))) {
        i++;
    }
    if(i < node->n && !compare) {
        DBT *data = malloc(sizeof(*data));
        data->size = node->offset_data[i + 1] - node->offset_data[i];
        data->data = malloc(data->size);
        memcpy(data->data, node->data->data + node->offset_data[i], data->size);
    }
    if(node->is_leaf)
        return NULL;
//!!!!!!!!!!!!!!!!!Need FREE!!!!!!!!!!!!!!!!!!!!!!!!
    node->Id = node->childs_id[i];
    read_from_file(node);
    return search(node, key);
}

void //!!!!!!!!!!!!!!!!!
split_child(Btree_node *x, Btree_node *y, int i)
{
    Btree_node *z = allocate_node();
    z->tree = y->tree;
    z->is_leaf = y->is_leaf;
    z->n = t - 1;
/***    z->data->data = malloc(); 
 SIZE*/
    for(int j = 0; j < t - 1; j++) {
        z->offset_key[j] = y->offset_key[j + t] - y->offset_key[t];
        z->offset_data[j] = y->offset_data[j + t] - y->offset_data[t];
    }
    z->data->size = y->offset_data[2 * t - 2] - y->offset_data[t];
    memmove(z->data->data, y->data->data + y->offset_data[t], z->data->size);
    z->keys->size = y->offset_key[2 * t - 2] - y->offset_key[t];
    memmove(z->keys->data, y->keys->data + y->offset_key[t], z->keys->size);
    if (!z->is_leaf) {
        for (int j = 0; j < t - 1; j++) {
            z->childs_id[j] = y->childs_id[j + t];
        }
    }
    y->n = t - 1;
    for (int j = x->n - 1; j > i; j--) {
        x->childs_id[j + 1] = x->childs_id[j];
    }
    x->childs_id[i + 1] = z->Id;
    int size_key = y->offset_key[t] - y->offset_key[t - 1];
    memmove(x->keys->data + x->offset_key[i + 1] + size_key, x->keys->data + x->offset_key[i + 1],
            x->offset_key[x->n - 1] - x->offset_key[i + 1]);
    memmove(x->keys->data + x->offset_key[i + 1], y->keys->data + y->offset_key[t - 1], size_key);
    int size_data = y->offset_data[t] - y->offset_data[t - 1];
    memmove(x->data->data + x->offset_data[i + 1] + size_data, x->data->data + x->offset_data[i + 1],
            x->offset_data[x->n - 1] - x->offset_data[i + 1]);
    memmove(x->data->data + x->offset_data[i + 1], y->data->data + y->offset_data[t - 1], size_data);
    for(int j = x->n - 1; j > i; j--) {
        x->offset_data[j + 1] = x->offset_data[j] + size_data;
        x->offset_key[j + 1] = x->offset_key[j] + size_key;
    }
    z->parent_Id = x->Id;
    
    x->n++;
    write_to_file(x);
    write_to_file(y);
    write_to_file(z);
}

void
insert_nonfull(Btree_node *x, DBT *key, DBT *value)
{
    int i = x->n - 1;
    if (x->is_leaf) {
        while (i >= 0 && memcmp(key, x->keys + x->offset_key[i], min(key->size, x->offset_key[i + 1] - x->offset_key[i] == -1))) {
            x->offset_key[i + 1] = x->offset_key[i] + key->size;
            x->offset_data[i + 1] = x->offset_data[i] + value->size;
            i--;
        }
        if (i + 2 != x->n - 1) {
            memmove(x->keys + x->offset_key[i + 2], x->keys + x->offset_key[i + 1],
                    x->offset_key[x->n + 1] - x->offset_key[i + 2]);
            memmove(x->data + x->offset_data[i + 2], x->data + x->offset_data[i + 1],
                    x->offset_data[x->n + 1] - x->offset_data[i + 2]);
        }
        x->n++;
        write_to_file(x);
    }
    else {
        while (i >= 0 && memcmp(key, x->keys + x->offset_key[i], min(key->size, x->offset_key[i + 1] - x->offset_key[i])) == -1) {
            i--;
        }
        i++;
        Btree_node *n = malloc(sizeof(*n));
        n->Id = x->childs_id[i];
        n->tree = x->tree;
        read_from_file(n);
        if (n->n == 2 * t - 1) {
            split_child(x, n, i);
                if(memcmp(key, x->keys + x->offset_key[i], min(key->size, x->offset_key[i + 1] - x->offset_key[i])) == 1)
                    i++;
        }
        insert_nonfull(n, key, value);
    }
}

void
insert(Btree *tree, DBT *key, DBT *value)
{
    Btree_node *r = tree->head;
    if (r->n == 2 * t - 1) {
        Btree_node *s = allocate_node();
        s->is_leaf = 0;
        s->tree = tree;
        tree->head = s;
        s->n = 0;
        s->parent_Id = -1;
        r->parent_Id = s->Id;
        insert_nonfull(s, key, value);
    }
    else
        insert_nonfull(r, key, value);
}

int
main(int argc, const char * argv[]) {
}

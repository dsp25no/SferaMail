#include <stddef.h>

#define t 4

/* check `man dbopen` */
struct DBT {
    void  *data;
    size_t size;
};

struct DB {
    /* Public API */
    /* Returns 0 on OK, -1 on Error */
    int (*close)(struct DB *db);
    int (*del)(const struct DB *db, const struct DBT *key);
    /* * * * * * * * * * * * * *
     * Returns malloc'ed data into 'struct DBT *data'.
     * Caller must free data->data. 'struct DBT *data' must be alloced in
     * caller.
     * * * * * * * * * * * * * */
    int (*get)(const struct DB *db, const struct DBT *key, struct DBT *data);
    int (*put)(const struct DB *db, const struct DBT *key, const struct DBT *data);
    /* For future uses - sync cached pages with disk
     * int (*sync)(const struct DB *db)
     * */
    /* Private API */
    /*     ...     */
    struct Btree *btree;
}; /* Need for supporting multiple backends (HASH/BTREE) */

struct DBC {
    /* Maximum on-disk file size
     * 512MB by default
     * */
    size_t db_size;
    /* Maximum chunk (node/data chunk) size
     * 4KB by default
     * */
    size_t chunk_size;
    /* For future uses - maximum cached memory size
     * 16MB by default
     * size_t mem_size; */
};

/* don't store metadata in the file */
struct DB *dbcreate(const char *file, const struct DBC conf);
struct DB *dbopen  (const char *file, const struct DBC conf);

int db_close(struct DB *db);
int db_del(const struct DB *, void *, size_t);
int db_get(const struct DB *, void *, size_t, void **, size_t *);
int db_put(const struct DB *, void *, size_t, void * , size_t  );
/* For future uses - sync cached pages with disk
 * int db_sync(const struct DB *db);
 * */
struct Btree {
    const char *name_of_file;
    struct Btree_node *head;
    char is_empty;
    int offset_of_data;
    int offset_of_list;
    int chunk_size;
    int (*write) (struct Btree_node *);
    int (*read) (struct Btree_node *);
    int (*alloc) (struct Btree_node *);
};

struct Btree_node {
    struct Btree *tree;
    int Id;
    int parent_Id;
    struct DBT *data;
    struct DBT *keys;
    int n;
    int offset_of_key;
    int offset_of_data;
    int offset_key[2 * t];
    int offset_data[2 * t];
    int childs_id[2 * t];
    char is_leaf;
};
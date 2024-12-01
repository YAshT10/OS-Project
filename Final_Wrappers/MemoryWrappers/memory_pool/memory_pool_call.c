#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct PoolInstance {
    void *source;       
    void **free_list;    
    size_t block_size;   
    size_t maxval;  
    size_t num_free;   
} PoolInstance;

//init for the pool, makes the pool structure
PoolInstance *mp_init(size_t block_size, size_t maxval) {
    PoolInstance *mp = malloc(sizeof(PoolInstance));
    if (!mp) return NULL;

    mp->block_size = block_size;
    mp->maxval = maxval;
    mp->num_free = maxval;
    mp->source = malloc(block_size * maxval);
    if (!mp->source) {
        free(mp);
        return NULL;
    }

    mp->free_list = malloc(sizeof(void *) * maxval);
    if (!mp->free_list) {
        free(mp->source);
        free(mp);
        return NULL;
    }

    for (size_t i = 0; i < maxval; i++) mp->free_list[i] = (char *)mp->source + i * block_size;
    return mp;
}

//allocates address from the pool
void *mp_alloc(PoolInstance *mp) {
    if (mp->num_free == 0) return NULL;  
    mp->num_free--;
    return mp->free_list[mp->num_free]; //reduces free count and returns the last block for allocation 
}

//frees a location in the pool
void mp_free(PoolInstance *mp, void *block) {
    if (mp->num_free != mp->maxval) mp->free_list[mp->num_free] = block;
    mp->num_free++;
}
//destros the pool
void mp_destroy(PoolInstance *mp) {
    free(mp->source);
    free(mp->free_list);
    free(mp);
}

int main() {
    PoolInstance *mp = mp_init(64, 100);  
    void *block1 = mp_alloc(mp);
    void *block2 = mp_alloc(mp);
    memset(block1, 0, 64);
    memset(block2, 0, 64);
    mp_free(mp, block1);
    mp_free(mp, block2);
    mp_destroy(mp);
    return 0;
}

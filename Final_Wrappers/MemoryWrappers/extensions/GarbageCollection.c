#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct GarbageCollectorInstance {
    void *data;                
    size_t size;               
    bool reachable;               
    struct GarbageCollectorInstance *link;    
} GarbageCollectorInstance;

typedef struct GarbageCollector {
    GarbageCollectorInstance *head;          
} GarbageCollector;

GarbageCollector gc = {NULL};

void *gc_alloc(size_t size) {
    GarbageCollectorInstance *obj = malloc(sizeof(GarbageCollectorInstance));
    obj->data = malloc(size);
    obj->size = size;
    obj->reachable = false;
    obj->link = gc.head;
    gc.head = obj;
    return obj->data;
}

void gc_mark(void *ptr) {
    for (GarbageCollectorInstance *obj = gc.head; obj; obj = obj->link) if (obj->data == ptr) {obj->reachable = true; break;}
}

void gc_collect() {
    GarbageCollectorInstance **indirect = &gc.head;
    while (*indirect){
        GarbageCollectorInstance *obj = *indirect;
        if (!obj->reachable) {free(obj->data); *indirect = obj->link; free(obj);}
        else {obj->reachable = false; indirect = &obj->link;}
    }
}


void gc_cleanup() {
    GarbageCollectorInstance *current = gc.head;
    while (current) {
        GarbageCollectorInstance *link = current->link;
        free(current->data);
        free(current);
        current = link;
    }
    gc.head = NULL;
}

## Vectors
A vector is a dynamically allocated array that automagically changes its size when needed. Items are stored in a single block of memory, just like a regular array; on top of that it takes care of the book keeping when adding/removing items. It's by far the most common collection type in mainstream programming languages.

Rather than storing pointers to values, we'll expose the memory directly to enable value based access; this means that the vector needs to know the size of its items.

```C
struct hc_vector {
  size_t item_size, capacity, length;
  uint8_t *start, *end;
  struct hc_malloc *malloc;
};
```

Example:
```C
struct hc_vector v;
hc_vector_init(&v, &hc_malloc_default, sizeof(int));
hc_vector_grow(&v, 10);
  
const int n = 10;
    
for (int i = 0; i < n; i++) {
  *(int *)hc_vector_push(&v) = i;
}
```

The `hc_vector_grow()` call in the preceding example is not strictly needed, but helps reduce allocations; without it the vector would need to duble the size of its memory block 3 times (allocating 2, 4, 8 and finally 16*32 bytes) to store 10 integers.

```C
void hc_vector_grow(struct hc_vector *v, int capacity) {
  v->capacity = capacity; 
  size_t size = v->item_size * (v->capacity+1);
  uint8_t *new_start = _hc_acquire(v->malloc, size);

  if (v->start) {
    memmove(new_start, v->start, v->length * v->item_size);
    _hc_release(v->malloc, v->start); 
  }
  
  v->start = new_start;
  v->end = v->start + v->item_size*v->length;
}
```

A macro is provided to simplify looping.

```C
#define _hc_vector_do(v, _v, var)
  struct hc_vector *_v = v;
  for (void *var = _v->start;
       var < (void *)_v->end;
       var += _v->item_size)

#define hc_vector_do(v, var)
  _hc_vector_do(v, hc_unique(vector), var)
```

Example:
```C
hc_vector_do(&v, it) {
  int v = *(int *)it;
  ...
}
```

Alternatively you can use `hc_vector_get()` with a manual loop, which is slightly slower since it needs to call a function and calculate the pointer for every iteration.

```C
for (int i = 0; i < n; i++) {
  int v = *(int *)hc_vector_get(&v, i);
  ...
}  
```

While not the primary use case for vectors, it's sometimes useful to be able to insert/delete blocks of items. `hc_vector_insert()` moves the tail if needed and returns a pointer to the start of the inserted block.

```C
void *hc_vector_insert(struct hc_vector *v, int i, int n) {
  const int m = v->length+n;
  if (m > v->capacity) { hc_vector_grow(v, m); } 
  uint8_t *const p = hc_vector_get(v, i);

  if (i < v->length) {
    memmove(p + v->item_size*n, p, (v->length - i) * v->item_size);
  }
  
  v->length += n;
  v->end += n*v->item_size;
  return p;
}
```

`hc_vector_delete()` similarly moves the tail if needed.

```C
void hc_vector_delete(struct hc_vector *v, int i, int n) {
  const int m = i+n;
  assert(v->length <= m);
  
  if (m < v->length) {
    uint8_t *const p = hc_vector_get(v, i);
    memmove(p, p + n*v->item_size, i + (v->length-n) * v->item_size);
  }

  v->length -= n;
  v->end -= n*v->item_size;
  return true;
}
```
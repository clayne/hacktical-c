## Vectors
A vector can be thought of as a dynamically allocated array that automagically changes its size on use. Items are stored in a single block of memory, just like a regular array; but you can add/remove items and it will take care of all the book keeping. It's by far the most common collection in modern languages.

Rather than storing pointers to values, we'll expose the memory directly to allow copying values in place. This means that the vector needs to know the size of its items.

```C
struct hc_vector v;
hc_vector_init(&v, sizeof(int));
hc_vector_grow(&v, 10);
  
const int n = 10;
    
for (int i = 0; i < n; i++) {
  *(int *)hc_vector_push(&v) = i;
}
```

The `hc_vector_grow()` call in the preceding example is not strictly needed, but helps reduce allocations; without it the vector would need to duble the size of its memory block 3 times (allocating 2, 4, 8 and finally 16*32 bytes) to store 10 integers.

```C
void hc_vector_grow(struct hc_vector *v, int capacity) {
  v->capacity = capacity ? capacity : 2;

  v->items = realloc(v->items,
		     hc_align(v->item_size*(v->capacity+1), v->item_size));

  v->start = hc_align(v->items, v->item_size);
  v->end = v->start + v->item_size*v->length;
}
```

A macro is provided to simplify looping.

```C
hc_vector_do(&v, it) {
  int v = *(int *)it;
  ...
}
```

Alternatively you could use `hc_vector_get()` with a manual loop, which is slightly slower (since it needs to calculate the pointer for every iteration).

```C
for (int i = 0; i < n; i++) {
  int v = *(int *)hc_vector_get(&v, i);
  ...
}  
```
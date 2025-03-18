## Intrusive Doubly Linked Lists
Linked lists have a somewhat sketchy reputation these days. Which is a shame, because it's a very flexible and cheap technique for keeping track of a sequence of values.

Singly linked lists are tricky to rearrange and remove items from since you always need access to the previous item, which can only be obtained by starting from the head. Doubly linked lists solve this problem, since you have direct access to both the previous and next item. 

The other common objection is memory locality. Since list nodes are separate from the actual values, iterating means chasing pointers at every step. Modern computer architectures derive a lot of their speed from caching large blocks of data, as opposed to repeatedly accessing main memory; which makes ordinary linked lists slow compared to arrays.

Intrusive means that the list's infrastructure is stored inside the values. This means no extra allocations for list nodes and potentially no pointer chasing if values are allocated as a single block of memory.

At this point you might wonder what is the point of a list of values stored in a single block of memory. The point is that the allocation strategy has little to do with what sequences we choose to put values in. One common strategy in C is to allocate a bunch of values as a single block of memory, as opposed to asking the memory allocator for one value at a time; which reduces memory fragmentation.

This is what a list node looks like; as promised there is no trace of the value, just the links.

```C
struct hc_list {
  struct hc_list *prev, *next;
};
```

And this is a value which is set up to be part of two lists.

```C
struct my_item {
  struct hc_list one_list;
  struct hc_list another_list;
  int value;
};
```

We can now allocate as many items as we need in one block, and add them to a maximum of two lists at a time. Lists are simply nodes that are not part of any values.

```C
  struct hc_list one_list, another_list;
  hc_list_init(&one_list);
  hc_list_init(&another_list);

  const int n = 10;
  struct my_item items[n];
  
  for (int i = 0; i < n; i++) {
    items[i].value = i;
    hc_list_push_back(&one_list, &items[i].one_list);
    hc_list_push_back(&another_list, &items[i].another_list);
  }
```

Iterating a list means starting from the head and stepping through the links until we reach the head again. Since this is something we'll likely do a lot, extracting the pattern as a macro makes sense. Note that simply switching links form `next` to `prev` allows iterating the list in reverse, which is sometimes useful.

```C
#define _hc_list_do(l, i, _next)			
  for (struct hc_list *i = (l)->next, *_next = i->next;
       i != (l);						
       i = _next, _next = i->next)

#define hc_list_do(l, i)
  _hc_list_do(l, i, hc_unique(next))
```

We use the following macro to go out from `struct hc_list` to `struct my_item`.

```C
#define hc_baseof(p, t, m) ({			
      uint8_t *_p = (uint8_t *)p;		
      _p ? ((t *)(_p - offsetof(t, m))) : NULL;
    })
```

Like so.

```C
  hc_list_do(&one_list, i) {
    struct my_item it = hc_baseof(i, struct my_item, one_list);
    ...
  }
```

To remove an item from a list; we don't even need access to the actual list, just the item is enough.

```C
hc_list_delete(&items[0].one_list);
```
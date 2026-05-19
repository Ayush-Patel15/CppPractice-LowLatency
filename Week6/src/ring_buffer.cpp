/*
Ring Buffer - Foundation of the SPSC (single producer single consumer) Queue

This is a simple circular array with two indices or pointers - tail(where to write) and head(where to read)

Initial state — empty:
┌────┬────┬────┬────┬────┬────┬────┬────┐
│    │    │    │    │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘
 0    1    2    3    4    5    6    7
 ↑
head=tail=0

After pushing A, B, C:
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ A  │ B  │ C  │    │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘
 ↑              ↑
head=0         tail=3

After popping A:
┌────┬────┬────┬────┬────┬────┬────┬────┐
│    │ B  │ C  │    │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘
      ↑         ↑
     head=1    tail=3

Push — write to buffer[tail], increment tail.
Pop — read from buffer[head], increment head.
Empty — head == tail.
Full — (tail + 1) % N == head.
The ring wraps around — when tail or head reaches N, it goes back to 0. The buffer is reused continuously without any allocation.

#################################################################################################
There's no need of CAS in the SPSC ring buffer queue. As:

- Only the producer ever writes tail
- Only the consumer ever writes head
- Each variable has exactly one writer

Threrefore, no chance of race condition on the write operations
*/


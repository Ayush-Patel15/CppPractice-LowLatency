/*
isolcpus in C++ are isolated CPU's. This particular cores or CPU's are separated from the OS scheduler and mark as isolated to perform
only the task, specified to themselves. However, the Interrupt Requests (IRQs) from the hardware can still preempt and stop the execution
on that thread. For removing the IRQ's - We need to allocate a separate core for the IRQ's also. OS scheduled process (like serviced, cronjobs,
background and foreground processes) and harware interrupts are two wholetogether separate things.

Without isolcpus:
Core 0: trading thread + kernel threads + any other process + IRQ's
Core 1: trading thread + kernel threads + any other process

With isolcpus=2,3:
Core 0: general purpose — OS schedules anything here
Core 1: Interrupt Requests (IRQ)
Core 2: ISOLATED — only explicitly pinned threads run here
Core 3: ISOLATED — only explicitly pinned threads run here

In Linux systems, there's a tick clock that runs on a defined interval on all the CPU cores, and this can also hinder the isolated behaviour.
So, need to mark the isolated CPU's as no tick clock also (known as nohz_full tick)

Basically, there are 3 rules to make a core isolated:
- mark it as isolated, from OS scheduler processes
- mark it as isolated, fromt the CPU tick (nohz_full)
- Assign IRQ's a different core, and keep the current core isolated from it.

THIS IS SYSTEM DEPENDENT i.e. DIFFERS HARDWARE TO HARDWARE (so, linux has separate commands to set it, and mac os has different)
*/

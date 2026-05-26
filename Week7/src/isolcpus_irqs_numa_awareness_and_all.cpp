/*
What IRQs Are
An Interrupt Request (IRQ) is a hardware signal that tells the CPU to stop what it's doing and handle an event immediately. 
Each IRQ has a number. The OS maintains an interrupt affinity — which cores handle which IRQs. By default the OS spreads IRQs across 
all cores for load balancing. This means your trading core regularly gets interrupted by network packets, timer ticks, and disk events.

Common sources:
Network card    → packet received
NIC             → DMA transfer complete
Timer           → scheduler tick
Disk controller → I/O complete
USB controller  → device event

By default Linux runs irqbalance — a daemon that periodically rebalances IRQs across cores for optimal throughput. 
This undoes your manual IRQ affinity settings.
Stop it:
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance

THE COMPLTE ISOLATION SETUP

1). kernel boot parameters
isolcpus=2,3,4,5        → remove from scheduler
nohz_full=2,3,4,5       → disable timer tick on isolated cores
rcu_nocbs=2,3,4,5       → move RCU callbacks off isolated cores

2). Disable irqbalance
sudo systemctl disable irqbalance
sudo systemctl stop irqbalance

3). Move the IRQ off the isolated CPU's

4). Set NIC interrupt affinity specifically, (separetely)

5). Disable real-time throttling

6). In the code: pinThreadToCore, and setRealTimeScheduling
*/

/*
NUMA AWARE PLACEMENT: As, NUMA (Non Uniform Memory Access) can cause latency, because of the different NUMA nodes or say sockets.
So, always try to keep the relevant data, and it's producers and consumers in the same NUMA node. So, the local memory data access
can work. As, the DMA i.e. direct market data, read by NIC, goes directly to RAM or save up in RAM. So, if it's a same numa node
- it's access will be faster.
*/

// Set up with a numa node
void setUpATradingThread(int core_number, int rt_priority, int numa_node){
    // pin a thread to core
    pinThreadToCore(core_number);

    // Set rt scheduling
    setRealTimeScheduling(rt_priority);

    // Set a preferred numa node - if available, using libnumas
    numa_set_preffered(numa_node);
}
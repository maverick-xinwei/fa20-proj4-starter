# numc

What has been learned so far from 61c

1. floating point
2. can only free the pointer directly allocated by malloc, cannot free some pointer in the middle.
3. RISC V instruction
4. RISC V assembly, so we know what is a pointer and a double pointer.
5. venus riscv simulator.
6. Logisim GUI simulator. 
7. Pipeline: insert NOP to avoid hazard.
8. Cache:  direct, associative , set-associative
9. L2 cache has worse hit rate just because it's accessed only when L1 is missed.
10. Cache coherency miss when running multiple core.
11. TBL is in front of Cache because Cache should use the physical addresses.  multiple programs can have same virtual addresses.
12. virtual memory:
  page miss: not in TLB
  page fault: page is not in DRAM, so should be loaded by OS from Disk to DRAM.  During this time, OS can choose to start a different program
              while waiting for page loading via context switching.  AXI Stash is try to reduce the probability that page fault happens. 

13. SIMD: same instruction runs multiple data at the same cycle.
14. Interupt vs Exception :  Interupt is from outside and needs to be handled by process.  While exception happens internally, and need OS to be involved to react on it.
15. mmu simulator :  Camera  cache and memeory ...
16. atomic access:  A access to an address can only be done by one thread, not by multiple threads/cores in parallel. This has to be done by hardware(e.g. AXI) This is the reason why we should use language defined semaphore , rather than declare a variable and try to use it as a semaphore.
17. mapReduce
18. openmp : pragmas to enable parallel looping
19. Unroll loop to boost the performance:  looping more time means executing more jal and beq.
20. proj4 numc
    1. use  disutils with a setup.py to compile C files to a python module
    2. install it to local python (maybe venv)
    3. python c api. 
        how to unpack arguments,
        tp_as_* functions
        ...

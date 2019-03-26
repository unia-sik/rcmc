VHDL implementations of RC/MC


core1
-------------------
RV64I, PaterNosterBE router, PIMP-2 network interface 

core2
-------------------
Same as core1, but with multiplier and FPU: RV64IMFD




cmp.sh
----------
Simulate an ELF file with MacSim and GHDL and compare the execution.
Different working directories are possible to allow parallel execution.


cmp_multiple.sh
---------------
Compare all ELF files given as arguments in the command line



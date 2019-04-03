RISC-V Instruction extension for Fine Grained Message Passing
=============================================================



Instruction encoding
--------------------


31..25  24..20  19..15  14..12  11..7   6..0
--------------------------------------------------------------------
....... rs2     rs1     000     00000   1011011         SND rs1, rs2
....... .....   rs1     001     00000   1011011         SRDY rs1
....... .....   .....   010     rd      1011011         RCVN rd
....... .....   .....   011     rd      1011011         RCVP rd
....... rs2     rs1     100     00000   1011011         IBRR rs1, rs2

disp    .....   .....   000     disp    1111011         BSF disp
disp    .....   .....   001     disp    1111011         BSNF disp
disp    .....   .....   010     disp    1111011         BRE disp
disp    .....   .....   011     disp    1111011         BRNE disp
disp    .....   rs1     100     disp    1111011         BR rs1 disp
disp    .....   rs1     101     disp    1111011         BNR rs1 disp
disp    .....   .....   110     disp    1111011         BBRR disp

0000000 rs2     rs1     000     00000   1101011         SEND rs1, rs2
0000000 00000   00000   001     rd      1101011         CONG rd
0000000 00000   rs1     100     rd      1101011         RECV rd, rs1
0000000 00000   rs1     101     rd      1101011         PROBE rd, rs1
0000000 00000   00000   110     rd      1101011         WAIT rd
0000000 00000   00000   111     rd      1101011         ANY rd



| CSR   | name          | read/write | PIMP version |
| ----- |-------------- | ---------- | ------------ |
| 0xc70 | MAXCID        | (readonly) | 1, 2, 3+     |
| 0xc71 | CID           | (readonly) | 1, 2         |
| 0xc72 | NOCDIM        | (readonly) |       3+     |
| 0xc73 | SENDRDY       | (readonly) | 1            |
| 0xc74 | NEXTRECV      | (readonly) | 1            |
| 0xc75 | XYZ           | (readonly) |       3+     |





PIMP-3 interface
================


PIMP-3 base instructions
------------------------

### SND rs1, rs2 (send)
Send the payload in rs2 to core rs1. If the send buffer is full, throw an
exception.

### RCVN rd (receive node)
Write the core number of the sender of the oldest incomming message to rd.
Throw an exception if receive buffer is empty.

### RCVP rd (receive payload)
Write the payload of the oldest incomming message to rd and remove it from the
receive buffer. Throw an exception if receive buffer is empty.

### BSF disp (branch if send buffer full)
Branch to disp, if the send buffer is full.

### BSNF disp (branch if send buffer not full)
Branch to disp, if there is space left in the send buffer.

### BRE disp (branch if receive buffer empty)
Branch to disp, if the receive buffer is empty.

### BRNE disp (branch if receive buffer not empty)
Branch to disp, if there is a message waiting in the receive buffer.


PIMP-3 ready flit extension
---------------------------

### SRDY rs1
Send a READY flit to core rs1. Stall it the send buffer is full.
Works in the same way like SEND, but flit is marked and has no data.

### BNR rs1 [IMM]
Branch if not ready: Checks if core rs1 is marked as ready.
When it is ready, go on to next instruction.
When not, jump to immediate address (0 to jump to itself).


PIMP-3 for one-to-one
---------------------

### IBRR rs1, rs2
Initialise a barrier in the rectangular area between core rs1 and rs2.

### BBRR disp
Branch to disp, if at least one node has not reached the barrier yet.





PIMP-3 CSRs
-----------

### MAXCID
Total number of cores.

### XYZ
Coordinates within the NoC as replacement for the deprecated Core ID.

### NOCDIM
Information about the physical topology. Cubes with up to 4 dimensions are 
supported. The returned 64-bit value is a vector of 4 16 bit value. Each value
gives the edge length of one dimension (in numbers of cores):

  * bits 63..48: edge length in 4th dimension
  * bits 47..32: edge length in z-direction
  * bits 31..16: edge length  in y-direction
  * bits 15..0: edge lenght in x-direction

For RV32, only 2 dimensions are supported.








PIMP-2 interface (deprecated)
=============================

Most important difference to PIMP-3 is the numbering of the nodes. The nodes
are numbered consecutively from 0 to MAXCID-1. If the NoC width is not a power
of 2, expensive divisions are required to determine x and y coordinates.


PIMP-2 instructions
-------------------

### SEND rs1, rs2 
Send the data in rs2 to core rs1. Stall if the send buffer is full.

### RECV rd, rs1
Write a data flit that arrived from core rs1 to register rd. If there is no
data flit in the receive buffer of rs1, stall until a flit arrives.

### PROBE rd, rs1
Write 0 to rd if there is no data flit from core rs1 in the receive buffer.
Otherwise write 1 or another value. Extension: return the number of flits from
core rs1 that are in the receive buffer.

### CONG[estion] rd
Write 0 to rd if the send buffer is full. Otherwise 1 or another value.
Extension: return the number of free entries in the send buffer.
Could also be implemented as CSR, but there are problems.

### ANY rd
Check if any flit has arrived and return the core number of the source core.
Otherwise return a number larger then max rank, e.g. -1
Could also be implemented as CSR, but there are problems.

### WAIT rd
Wait until a flit from any other core arrives and write the number of the core
to rd. If the receive buffer is empty, stall.
Replace by a ANY instruction and a loop.



PIMP-2 CSRs
-----------

### MAXCID
Total number of cores.

### CID
Core ID of the current core. In the range 0 ... (MAXCID-1).






PIMP-1 interface (deprecated)
=============================

SEND, RECV and PROBE like in PIMP-2. Other instructions (CONG, ANY, WAIT) not available.

PIMP-1 only CSRs
----------------

### SENDRDY
0 if send buffer is full. Other value if not full. Value may indicate how many
free entries are remaining.
Replaced by CONG instruction.

### NEXTRECV
Rank of the sender of the first flit in the receive buffer. 
Negative if receive buffer is empty.
Replaced by ANY instruction.


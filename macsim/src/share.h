/** 
 * $Id: share.h 744 2015-02-04 14:47:59Z klugeflo $
 * Shared definitions, constants and macros that should remain unchanged
 *
 * McSim project
 */
#ifndef _SHARE_H
#define _SHARE_H



// ---------------------------------------------------------------------
// general 
// ---------------------------------------------------------------------


#include <stdbool.h>
#include <stddef.h>	// for size_t
#include <stdint.h>
#include <stdio.h>	// for FILE
#include <assert.h>


typedef uint64_t uint64_min_t;
typedef uint32_t uint32_min_t;
typedef uint32_t uint16_min_t;
typedef int64_t int64_min_t;
typedef int32_t int32_min_t;
typedef int32_t int16_min_t;



#define BITRANGE(v,h,l)		(((v)>>(l))&((1<<((h)-(l)+1))-1))
#define BITRANGE_S(v,h,l)	((((int32_t)(v))<<(31-(h)))>>(31-(h)+(l))) 
#define SIGN_EXTEND_32(v,b)	(((int32_t)((int32_t)(v)<<(32-b)))>>(32-(b)))
#define SIGN_EXTEND_64(v,b)	(((int64_t)((int64_t)(v)<<(64-b)))>>(64-(b)))




// ---------------------------------------------------------------------
// simulator specific
// ---------------------------------------------------------------------

// core states
#define CS_OFF			0       // core will not be simulated
#define CS_STOPPED		1       // stopped by dedicated instruction
#define CS_UNKNOWN_INSTRUCTION	2       // stopped due to unknown instruction
#define CS_BREAKPOINT           3       // breakpoint (currently unused)
#define CS_SEND_BLOCKED		4	// waiting for permission to send
#define CS_RECV_BLOCKED		5	// waiting for an incoming  message
#define CS_FGMP_REQUEST		5	// waiting one cycle for fgmp response
#define CS_RUNNING		6
    // If the core state is greater than CS_RUNNING, it is stalled for the 
    // corresponding number of cycles

#define CS_READY(cs)		((cs)>=CS_SEND_BLOCKED)


// instruction classes
#define IC_STOP				-31	// execution cannot continue
#define IC_LOAD				-1       // memory read instruction
#define IC_STORE			-2       // memory write instruction
#define IC_LMS				-3       // load-modify-store instruction
#define IC_BRANCH_BACKWARD_NOTTAKEN	-4       // branch backward not taken
#define IC_BRANCH_BACKWARD_TAKEN	-5       // branch backward taken
#define IC_BRANCH_FORWARD_NOTTAKEN	-6       // branch forward not taken
#define IC_BRANCH_FORWARD_TAKEN		-7       // branch forward taken
#define IC_JUMP				-8	// unconditional jump to fixed address
#define IC_IJUMP			-9	// indirect jump (address given by register)
#define IC_CALL				-10      // subroutine call
#define IC_RETURN			-11      // return from subroutine
#define IC_MUL				-12      // integer multiplication
#define IC_DIV				-13      // integer division
#define IC_MAC				-14      // multiply-and-accumulate instruction
#define IC_FP				-15      // floating-point instruction
#define IC_SEND				-16      // send 
#define IC_RECV				-17      // receive
#define IC_BLOCKED			-18	// send or receive blocked
#define IC_ARITH			-19      // simple arithmetic or logic instruction
#define IC_EXCEPTION			-20      // exception was thrown
#define IC_ICALL                        -21      // indirect subroutine call (address in register)

#define IC_PLATFORM_DEPENDENT		-1000    // begin of platform dependent classes




typedef int_fast16_t instruction_class_t;
typedef int_fast32_t rank_t; // must be signed, because probe_any can return -1
typedef uint_fast32_t addr_t;

#define CYCLE_T_MAX INT64_MAX
typedef int_fast64_t cycle_t;



#define MAX_RANK        1024
extern rank_t conf_max_rank;
extern rank_t conf_noc_width;
extern rank_t conf_noc_height;

#define X_FROM_RANK(r) ((r)%conf_noc_width)
#define Y_FROM_RANK(r) ((r)/conf_noc_width)


// for configurable PaterNoster
extern unsigned conf_bypass_y;
extern unsigned conf_bypass_x;
extern unsigned conf_stall_y;
extern unsigned conf_stall_x;
extern unsigned conf_inject_y;
extern unsigned conf_inject_x;
#define CONF_BYPASS_NONE        0
#define CONF_BYPASS_UNBUF       1
#define CONF_BYPASS_BUF         2
#define CONF_BYPASS_2UNBUF      3
#define CONF_BYPASS_2BUF        4
#define CONF_STALL_CHEAP        0
#define CONF_STALL_EXP          1
#define CONF_INJECT_NONE           0
#define CONF_INJECT_REQUEST        1
#define CONF_INJECT_ALTERNATE      2
#define CONF_INJECT_THROTTLE       3

extern double conf_inj_rate;
extern uint64_t conf_inj_prob;



#define FLIT_LEN 8
typedef uint64_t flit_t; // payload of a flit
typedef struct {
	rank_t src;
	rank_t dest;
	char payload[FLIT_LEN];
	bool h_marked;
} flit_container_t; // complete flit with routing data


// ---------------------------------------------------------------------
// simulator logging output
// ---------------------------------------------------------------------

#define LOG_LEVEL_OFF		4
#define LOG_LEVEL_WARNING	3
#define LOG_LEVEL_INFO		2
#define LOG_LEVEL_DEBUG		1
#define LOG_LEVEL_ANY		0

// logging levels (configurable)
extern int log_intercon;
extern int log_core;
extern int log_general;

// output streams
extern FILE *stream_err; // errors and warnings
extern FILE *stream_usr; // output of the user interface (prompt)
extern FILE *stream_log; // logging of simulation
extern FILE *stream_all; // direct debug output of all cores intermixed
extern FILE *streams[MAX_RANK]; // streams to direct debug output of each core
extern bool use_file_stream[MAX_RANK]; // contains bools to decide whether to use stdout or an output stream for each core



// Init I/O and streaming pipes
void init_streaming();

// Fatal error (>stderr), terminate program
void fatal(const char *fmt, ...) __attribute__((__noreturn__));

// Warning to stderr if log_level is high enough
void warning(int log_level, const char *fmt, ...);

// Info to stdout if log_level is high enough
void info(int log_level, const char *fmt, ...);

// Debug output to stdout if log_level is high enough
void debug(int log_class, const char *fmt, ...);

// printf for user interface -> stream_usr
void user_printf(const char *fmt, ...);

// printf from the cores
void core_printf(rank_t rank, const char *fmt, ...);


// Allocate memory, fatal error if allocation fails
void *fatal_malloc(size_t size);

// Read whole file to memory and return pointer to it
unsigned char *read_whole_file(const char *filename, unsigned long int *size_ptr);


#endif // _SHARE_H

/** 
 * $Id: share.c 332 2012-03-22 15:59:22Z metzlast $
 * Shared definitions, constants and macros that should remain unchanged
 *
 * McSim project
 */
#include "share.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


// configuration
rank_t conf_max_rank = MAX_RANK;
rank_t conf_noc_width;
rank_t conf_noc_height;

unsigned conf_bypass_y;
unsigned conf_bypass_x;
unsigned conf_stall_y;
unsigned conf_stall_x;
unsigned conf_inject_y;
unsigned conf_inject_x;
double conf_inj_rate;
uint64_t conf_inj_prob;

unsigned long conf_send_fifo_size;
unsigned long conf_recv_fifo_size;
unsigned long conf_corner_fifo_size;

// output streams
FILE *stream_err;	// errors and warnings
FILE *stream_usr;	// output of the user interface (prompt)
FILE *stream_log;	// logging of simulation
FILE *stream_all;	// direct debug output of all cores intermixed
FILE *streams[MAX_RANK]; 
bool use_file_stream[MAX_RANK];

// logging levels (configurable)
int log_intercon = LOG_LEVEL_WARNING;	// interconnect
int log_core = LOG_LEVEL_ANY;		// core
int log_general = LOG_LEVEL_ANY;	// general (e.g. core stops)



void init_streaming()
{
    unsigned i;

    stream_err = stderr;
    stream_usr = stdout;
    stream_log = stdout;
    stream_all = 0;

    for(i = 0; i < MAX_RANK; i++) {
        use_file_stream[i]=false;
        streams[i] = 0;
    }
}


// Fatal error -> stream_err
void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stream_err, "FATAL: ");
    vfprintf(stream_err, fmt, ap);
    fprintf(stream_err, "\n");
    exit(1);
}

// Warning if log_level is high enough -> stream_err
void warning(int log_level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (log_level <= LOG_LEVEL_WARNING)
    {
	fprintf(stream_err, "WARNING: ");
	vfprintf(stream_err, fmt, ap);
	fprintf(stream_err, "\n");
    }
}

// Info if log_level is high enough -> stream_log
void info(int log_level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (log_level <= LOG_LEVEL_INFO) 
	vfprintf(stream_log, fmt, ap);
}

// Debug output if log_level is high enough -> stream_log
void debug(int log_level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (log_level <= LOG_LEVEL_DEBUG) 
	vfprintf(stream_log, fmt, ap);
}

// printf for user interface -> stream_usr
void user_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stream_usr, fmt, ap);
}


static const char rank_to_bold[12] = "000100111111";
static const char rank_to_colour[12] = "123456123567";

// printf from the cores
void core_printf(rank_t rank, const char *fmt, ...)
{
    // Forward output to stream_all. If it is empty, the output is redirected to
    // stdout with escape sequences for colors
    if (stream_all) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stream_all, fmt, ap);
    } else {
        // print to console
        va_list ap;
        va_start(ap, fmt);
        fprintf(stdout, "\033[%c;3%cm",
            rank_to_bold[rank%12], rank_to_colour[rank%12]);
        vfprintf(stdout, fmt, ap);
        fprintf(stdout, "\033[0m");
        fflush(stdout);
    }

    if (use_file_stream[rank]) {
        // print to file for each core separately
        va_list ap;
        va_start(ap, fmt);
        vfprintf(streams[rank], fmt, ap);
    }
}




// Allocate memory, fatal error if allocation fails
void *fatal_malloc(size_t size)
{
    void *p = malloc(size);
    if (p==0)
	fatal("Out of memory\n");
    return p;
}

// Read whole file to memory and return pointer to it
unsigned char *read_whole_file(const char *filename, unsigned long int *size_ptr)
{
    FILE *f;
    unsigned long int size;
    uint8_t *buf;
    
     // Read file to buf
    if (!(f=fopen(filename, "rb")))
	return 0; // file not found
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if (!(buf=(uint8_t *)malloc(size)))
    {
	fclose(f);
	return 0; // not enough memory
    }
    fseek(f, 0, SEEK_SET);
    if(fread(buf, 1, size, f) != size)
    {
	fclose(f);
	free(buf);
	return 0; // read error
    }
    *size_ptr = size;
    fclose(f);
    return buf;
}




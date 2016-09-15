#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>


#define MAX_INPUT_LINE 1024



// Breakpoints

#define MAX_BREAKPOINTS 8

struct
{
    node_t              *node;
    uint_fast32_t       addr;
    uint8_t             iw[8];
} breakpoints[MAX_BREAKPOINTS];

unsigned breakpoint_count = 0;


static bool add_breakpoint(node_t *node, addr_t addr)
{
    if (node->bp_addr!=NO_BREAKPOINT) {
        user_printf("Only one breakpoint per core\n");
        return false;
    } else {
        node->bp_addr = addr;
        return true;
    }
}

static bool remove_breakpoint(node_t *node, uint_fast32_t addr)
{
    node->bp_addr = NO_BREAKPOINT;
    return true;
}







node_t *nodes[MAX_RANK];	// list of all nodes
uint_fast16_t conf_noc_type;     // type of the NoC


// Print reason why the process was stopped and return core state
uint_fast32_t reason_for_stop(node_t *node)
{
    uint_fast32_t cs = node->state;
    switch (cs)
    {
    case CS_STOPPED:
        info(log_general, "%02x@%08x: Core stopped at cycle %ld.\n",
             node->rank, node->pc, node->cycle);
        break;
    case CS_UNKNOWN_INSTRUCTION:
        info(log_general, "%02x@%08x: Unknown instruction %x.\n",
             node->rank, node->pc, node->instruction_word);
        break;
    default:
        fatal("%02x@%08x: Core stopped in unknown state %d.\n",
              node->rank, node->pc, node->state);
    }
    return cs;
}


// Simulate one instruction
// return the number of cycles that it takes
//
// FIXME: Should node->cycle be also incremented if node was stopped?
//        Save stop time in seperate variable?
void simulate_one_instruction(node_t *node)
{
    uint_fast32_t cs = node->state;

    if (cs>CS_RUNNING) {        // multicycle instruction
        node->cycle++;
        node->state = cs-1;
        if (cs==CS_RUNNING+1) node->pc = node->nextpc;
    } else if (CS_READY(cs)) {
        instruction_class_t ic = node->one_cycle(node);

        // Timing
        if (ic>0) {
            if (ic==1) node->pc = node->nextpc;
            node->cycle++;
            node->state = CS_RUNNING-1+ic; // store multiple cycles in state
        } else if (ic==IC_BLOCKED) {
            // If send or receive blocks, don't increase the PC.
            // Thereby the instruction is executed again and again, until
            // the blocking is over.
            node->cycle++;
        } else if (ic==IC_STOP) {
            reason_for_stop(node);
        } else fatal("Unknown instruction class %d.", ic);
    }
}


// Simulate all cores and the periodical interconnect for one cycle only
void simulation_step()
{
    // simulate interconnect
    noc_route_all(nodes, conf_max_rank);

    // simulate all cores for one cycle
    rank_t r;
    for (r=0; r<conf_max_rank; r++) {
        simulate_one_instruction(nodes[r]);
    }
}


// Simulate all cores and the periodical interconnect
void simulation()
{
    bool ready[conf_max_rank];
    rank_t r;
    cycle_t min=CYCLE_T_MAX;
    cycle_t max=0;

    for (r=0; r<conf_max_rank; r++)
    {
        // determine, which cores are still running
        if (!CS_READY(nodes[r]->state)) {
            ready[r] = 0;
        } else {
            ready[r] = 1;

            cycle_t c = nodes[r]->cycle;
            if (c<min) min=c;
            if (c>max) max=c;
        }
    }

    bool any_ready = 0;
    do {
        simulation_step();

        // check for breakpoints
        any_ready = 0;
        for (r=0; r<conf_max_rank; r++) {
            if (CS_READY(nodes[r]->state)) {
                if (nodes[r]->pc == nodes[r]->bp_addr) return;
                any_ready = 1;
            }
        }
    } while (any_ready);
}


// Return a pointer to the first non-whitepace, 0 if empty
// Cut trailing whitespaces
char *trim_whitespace(char *line)
{
    char *s=line;
    while (*s<=32)
    {
        if (*s==0) return 0;
        s++;
    }

    // replace trailing whitespaces by 0
    char *t = s + strlen(s)-1;
    while (t>=s && *t<=32)
    {
        *t = 0;
        t--;
    }
    return s;
}


// redirect a stream to a file
bool redirect_stream(FILE **stream, char *filename)
{
    if (!filename)
    {
        user_printf("Filename expected\n");
        return 0;
    }
    FILE *f = *stream;
    if (f && f!=stdout)
        fclose(f);
    f = fopen(filename, "w");
    if (!f)
        fatal("Could not open '%s'", filename);
    *stream = f;
    return 1;
}




node_t *node;
static addr_t last_dump_addr_end = 0xa0000000;
static addr_t last_disasm_addr_end = 0;


// execute command in line
void process_line(char command, char *argument)
{
    switch (command)
    {
    case '#': // comment in configuration file
        break;

    case 'a': // load one file to all cores
    {
        rank_t r;
        if (!argument)
        {
            user_printf("Filename expected.\n");
            break;
        }
        user_printf("Loading '%s' to all cores...\n", argument);
        for (r=0; r<conf_max_rank; r++)
            core_finish_context(nodes[r]);
        core_init_all(nodes, conf_max_rank);
        for (r=0; r<conf_max_rank; r++) {
            memory_load_file(nodes[r], argument);
        }
        break;
    }

    case 'b': // set a breakpoint
    {
        unsigned bp;
        int count;
        if (!argument)
        {
            user_printf("Address expected.\n");
            break;
        }
        count = sscanf(argument, "%x", &bp);
        if (count>0) add_breakpoint(node, bp);
        break;
    }

    case 'c': // switch to other core and print its context
    {
        unsigned c;
        int count;
        if (argument)
        {
            count = sscanf(argument, "%x", &c);
            if (count==1) // if no core give, print context of current core
            {
                if (c>=conf_max_rank)
                {
                    user_printf("Only %d cores available.\n", conf_max_rank);
                    break;
                }
                else
                {
                    node = nodes[c];
                    user_printf("Switched to core %d.\n", c);
                }
            }
        }
        core_print_context(node);
        break;
    }

    case 'd': // dump memory
    {
        unsigned from, to;
        int count;
        if (argument)
            count = sscanf(argument, "%x %x", &from, &to);
        else
            count = 0;
        if (count<2)
        {
            if (count<1) // 0 or -1
                from = last_dump_addr_end;
            if (from<0xffffffc0)
        	to = from+0x3f;
    	    else
        	to = 0xffffffff;
        }
        if (to < from)
            user_printf("End ahead of beginning!\n");
        else
        {
            memory_print_dump(node, from, to);
            last_dump_addr_end = to+1;
        }
        break;
    }

    case 'e': // load an file to only ONE core
    {
        if (!argument)
        {
            user_printf("Filename expected.\n");
            break;
        }
        user_printf("Loading '%s' to core %02x...\n", argument, node->rank);
        core_finish_context(node);
        core_init_context(node);
        memory_load_file(node, argument);
        break;
    }

    case 'g': // goto breakpoint
    {
        unsigned long bp;
        int count = 0;
        if (argument)
        {
            count = sscanf(argument, "%lx", &bp);
            if (count>0) 
                if(!add_breakpoint(node, bp)) break;
        }

        clock_t duration = clock();

        simulation();

        duration = clock() - duration;
        if ((duration/CLOCKS_PER_SEC)>0)
            user_printf("Simulation took %.1fs, %ld cycles/s\n",
                        (double)duration/(double)CLOCKS_PER_SEC,
                        (node->cycle*CLOCKS_PER_SEC) / duration);

        if (count>0)
            remove_breakpoint(node, bp);
        break;
    }

    case 'h': // help
        user_printf(
            "Configuration\n"
            "A [name]      set ISA of all cores (armv6m, riscv)\n"
            "I [float]     set injection rate for synthetic traffic patters\n"
            "R [name]      set routing algorithm (fixedlat, pnbe0, pnbe1, "
                            "pnbe2, caerus, pnoa, pnaa, pnoo, pnood, pnbase, "
                            "pnjm0)\n"
            "N [rank]      set number of cores (e.g. 64 or 8x8)\n"

            "\nDebugging\n"
            "a [file].elf  load one file to all cores\n"
            //"b [brkpoint]  set breakpoint (on current core)\n"
            "c [rank]      switch to other core and print its context\n"
            "d [from [to]] dump memory\n"
            "e [file].elf  load file to current core only\n"
            "g [brkpoint]  run up to breakpoint (on current core)\n"
            "h             display this help\n"
            "i [file]      interpret commands in file\n"
            "l [file]      redirect logging to a file\n"
            "m [file]      redirect output of current core to a file\n"
//            "n             simulate next instruction (step over calls)"
            "o [file]      redirect cumulative output of all cores to a file\n"
            "p             print context of all cores\n"
            "r             print NoC context\n"
            "q             quit\n"
            "s [cycles]    simulate n cycles (step into calls)\n"
            "t [cycles]    simulate n cycles and show the trace\n"
            "u [from [to]] unassemble\n"
            "x [path] [max cycle] [step] run program to end (capped by max cycle) and store context if (cycle %% step) == 0. path is the path for the output context files.\n"
            "<enter>       simulate one cycle (step into calls)\n"
        );
        break;

    case 'i':
    {
        if (!argument)
        {
            user_printf("Filename expected\n");
            break;
        }
        user_printf("Interpreting commands in '%s' ...\n", argument);

        FILE *f = fopen(argument, "r");
        if (!f)
        {
            user_printf("Could not open '%s'\n", argument);
            break;
        }

        char line[MAX_INPUT_LINE];
        while (fgets(line, MAX_INPUT_LINE, f))
        {
            char *l = trim_whitespace(line); // ignore leading whitespace
            process_line(l[0], trim_whitespace(l+1));
        }
        fclose(f);
        break;
    }

    case 'l': // redirect logging output
        redirect_stream(&stream_log, argument);
        break;

    case 'm': // redirect output of current core to extra file
			if (redirect_stream(&streams[node->rank], argument))
			{
				use_file_stream[node->rank] = true;
				user_printf("Set output to %s for core %d\n", argument,node->rank);
			}
        break;

    case 'n': // next instruction (step over calls)
    {
        addr_t bp = node->pc+4; // TODO: analyse instruction length correctly
        add_breakpoint(node, bp);

        clock_t duration = clock();
        simulation();
        duration = clock() - duration;
        if ((duration/CLOCKS_PER_SEC)>0)
            user_printf("Simulation took %.1fs, %ld cycles/s\n",
                        (double)duration/(double)CLOCKS_PER_SEC,
                        (node->cycle*CLOCKS_PER_SEC) / duration);

        remove_breakpoint(node, bp);
        core_print_context(node);
        break;
    }

    case 'o': // redirect cumulative output of all cores
        redirect_stream(&stream_all, argument);
        break;

    case 'p': // print contexts
    {
        //DIRTY
        rank_t r;
        if (nodes[0]->core_type==CT_traffic) {
            cycle_t count=0;
            cycle_t total=0;
            for (r=0; r<conf_max_rank; r++) {
                count += nodes[r]->core.traffic.stat_flit_count;
                total += nodes[r]->core.traffic.stat_total_latency;
            }
            printf("%lu flits injected total latency %lu average latency %g\n",
                count, total, (double)total / (double)count);
        } else {
            for (r=0; r<conf_max_rank; r++) {
                user_printf("\n=== core %02x ===\n", r);
                core_print_context(nodes[r]);
            }
        }
        break;
    }

    case 'q': // quit
    {
        rank_t r;
        cycle_t exectime = 0;
        noc_destroy_all(nodes, conf_max_rank);
        for (r=0; r<conf_max_rank; r++) {
            if (nodes[r]->cycle > exectime) exectime = nodes[r]->cycle;
            core_finish_context(nodes[r]);
        }
        printf("\nExecution Time: %lu cycles\n", exectime);
        for (r=0; r<MAX_RANK; r++) {
            free(nodes[r]);
        }
        exit(0);
    }

    case 'r': // print NoC context
        noc_print_context(nodes, conf_max_rank);
        break;

    case '\n': // execute one instruction (step into calls)
        simulation_step();
        core_print_context(node);
        break;

    case 's': // execute n instruction (step into calls)
    case 't': // execute n instructions and show the trace
        if(argument) {
            int32_t cycles, i;
            int count = sscanf(argument, "%d", &cycles);
            if (count==0) { // if no cycle count given, execute only one
                simulation_step();
                core_print_context(node);
            } else {
                if(cycles > 0) {
                    user_printf("Simulating %d cycles\n", cycles);
                    for (i = 0; i < cycles; i++) {
                        simulation_step();
                        if (command=='t') core_print_context(node);
                    }
                    if (command=='s') core_print_context(node);
                } else {
                    user_printf("error: cannot simulate %d cycles. "
                        "Use cycle count larger than 0.\n", cycles);
                }
            }
        } else {
            simulation_step();
            core_print_context(node);
        }
        break;

    case 'u': // unassemble instructions
    {
        unsigned from, to;
        int count;
        if (argument)
            count = sscanf(argument, "%x %x", &from, &to);
        else
            count = 0;
        if (count<2)
        {
            if (count<1) // 0 or -1
                from = last_disasm_addr_end;
            if (from<0xffffffc0)
        	to = from+0x1f;
    	    else
        	to = 0xffffffff;
        }
        if (to < from)
            user_printf("End ahead of beginning!\n");
        else
        {
	    char dstr[MAX_DISASM_STR];
            last_disasm_addr_end = from;
            while (last_disasm_addr_end <= to)
            {
        	int i, len;
        	uint8_t buf[MAX_BYTES_PER_INSTRUCTION];
        	memory_read(node, last_disasm_addr_end, buf, MAX_BYTES_PER_INSTRUCTION);
        	len = core_disasm(node, last_disasm_addr_end, dstr);
        	printf("%8x: ", (unsigned)last_disasm_addr_end);
        	for (i=0; i<MAX_BYTES_PER_INSTRUCTION; i++)
        	{
        	    if (i<len)
        		printf("%02x", buf[i]);
        	    else
        		printf("  ");
        	}
        	printf("\t%s\n", dstr);
        	last_disasm_addr_end += len;
    	    }
        }
        break;
    }


    // configuration

    case 'A': // set architecture of the cores
    {
        rank_t r;
        uint_fast16_t ct;
        uint_fast16_t tp;

        if (strcmp(argument, "armv6m")==0)         ct=CT_armv6m;
//        else if (strcmp(argument, "mips32"))    ct=CT_mips32;
//        else if (strcmp(argument, "or32"))      ct=CT_or32;
//        else if (strcmp(argument, "patmos"))    ct=CT_patmos;
        else if (strcmp(argument, "riscv")==0)     ct=CT_riscv;
//        else if (strcmp(argument, "tricore"))   ct=CT_tricore;
        else if (strncmp(argument, "traffic", 7)==0) {
            ct = CT_traffic;
            tp = argument[7];
        } else {
            user_printf("Unknown architecture '%s'.\n", argument);
            break;
        }
        user_printf("Changing ISA to %s\n", argument);

        for (r=0; r<conf_max_rank; r++)
            core_finish_context(nodes[r]);
        for (r=0; r<MAX_RANK; r++) { // if number is increased later
            nodes[r]->core_type = ct;
            if (ct==CT_traffic) nodes[r]->core.traffic.type = tp;
        }
        core_init_all(nodes, conf_max_rank);
        break;
    }

    case 'I': // set injection rate
    {
        double f;
        if (!argument || sscanf(argument, "%lf", &f) != 1) {
            user_printf("Injection rate expected.\n");
        } else if (f<=0.0 || f>=1.0) {
            user_printf("Injection rate must be between 0.0 and 1.0 (exclusive)");
        } else {
            user_printf("Injection rate set to %g.\n", f);
            conf_inj_rate = f;
            conf_inj_prob = llrint(0x1.0p64 * conf_inj_rate);
        }
        break;
    }

    case 'R': // set NoC routing algorithm
    {
        uint_fast16_t nt;
        char *s;
        if (strcmp(argument, "fixedlat")==0) {
            nt=NT_fixedlat;
            s="Fixed latency";
        } else {
            if (strcmp(argument, "pnbe0")==0) {
                nt=NT_pnbe0;
                s="PaterNoster Best Effort backup 0";
            } else if (strcmp(argument, "pnbe1")==0) {
                nt=NT_pnbe1;
                s="PaterNoster Best Effort prototype 1";
            } else if (strcmp(argument, "pnbe2")==0) {
                nt=NT_pnbe2;
                s="PaterNoster Best Effort prototype 2";
            } else if (strcmp(argument, "caerus")==0) {
                nt=NT_caerus;
                s="Caerus";
            } else if (strcmp(argument, "pnoa")==0) {
                nt=NT_pnoa;
                s="PaterNoster One-to-All";
            } else if (strcmp(argument, "pnaa")==0) {
                nt=NT_pnaa;
                s="PaterNoster All-to-All";
            } else if (strcmp(argument, "pnood")==0) {
        	nt=NT_gs_one_to_one_dyn;
        	s="PaterNoster One-to-One Dynamic buffer";
            } else if (strcmp(argument, "pnoo")==0) {
        	nt=NT_gs_one_to_one;
        	s="PaterNoster One-to-One";
            } else if (strcmp(argument, "pnbase")==0) {
        	nt=NT_paternoster_skeleton;
        	s="PaterNoster base";
            } else if (strcmp(argument, "pnjm0")==0) {
                nt = NT_pnjm0;
                s = "PaterNoster from first paper";
            } else if (strcmp(argument, "minbd")==0) {
                nt = NT_minbd;
                s = "MinBD";
            } else if (strcmp(argument, "perfect")==0) {
                nt = NT_perfect;
                s = "Perfect NoC without any latency";
            } else if (argument[0]=='C') {
                nt = NT_pnconfig;
                s = "Configurable PaterNoster";
                switch (argument[1]) {
                    case 'n': conf_bypass_y = CONF_BYPASS_NONE; break;
                    case 'u': conf_bypass_y = CONF_BYPASS_UNBUF; break;
                    case 'b': conf_bypass_y = CONF_BYPASS_BUF; break;
                    case 'w': conf_bypass_y = CONF_BYPASS_2UNBUF; break;
                    case 'd': conf_bypass_y = CONF_BYPASS_2BUF; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[2]) {
                    case 'n': conf_bypass_x = CONF_BYPASS_NONE; break;
                    case 'u': conf_bypass_x = CONF_BYPASS_UNBUF; break;
                    case 'b': conf_bypass_x = CONF_BYPASS_BUF; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[3]) {
                    case 'c': conf_stall_y = CONF_STALL_CHEAP; break;
                    case 'e': conf_stall_y = CONF_STALL_EXP; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[4]) {
                    case 'c': conf_stall_x = CONF_STALL_CHEAP; break;
                    case 'e': conf_stall_x = CONF_STALL_EXP; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[5]) {
                    case '0': conf_inject_y = CONF_INJECT_NONE; break;
                    case 'r': conf_inject_y = CONF_INJECT_REQUEST; break;
                    case 'a': conf_inject_y = CONF_INJECT_ALTERNATE; break;
                    case 't': conf_inject_y = CONF_INJECT_THROTTLE; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[6]) {
                    case '0': conf_inject_x = CONF_INJECT_NONE; break;
                    case 'r': conf_inject_x = CONF_INJECT_REQUEST; break;
                    case 'a': conf_inject_x = CONF_INJECT_ALTERNATE; break;
                    case 't': conf_inject_x = CONF_INJECT_THROTTLE; break;
                    default: fatal("Unknown configuration '%s'.\n", argument);
                }
            } else {
                user_printf("Unknown NoC '%s'.\n", argument);
                break;
            }
            if (conf_noc_width != conf_noc_height) {
                user_printf("This NoC only supports a square number of cores");
                break;
            }
        }

        user_printf("Changing NoC to %s\n", s);
        conf_noc_type = nt;
        noc_destroy_all(nodes, conf_max_rank);
            // free memory allocated for the previous NoC
        noc_init_all(nodes, conf_noc_type, conf_noc_width, conf_noc_height);
            // initialize the new NoC
        break;
    }

    case 'N': // set number of cores
    {
        rank_t r;
        unsigned x, y;
        char ch;
        if (!argument) {
            user_printf("Number of cores expected.\n");
            break;
        }
        int n = sscanf(argument, "%d%c%d", &x, &ch, &y);
        if (n==1) {
            if (x>MAX_RANK) {
                user_printf("No more than %d cores can be simulated.\n", MAX_RANK);
                break;
            }
            for (y=1; y<MAX_RANK; y++) if (y*y >= x) break;
            // either nxn nodes or Nx1 nodes
            if (y*y==x) {
                conf_noc_width = y;
                conf_noc_height = y;
            } else {
                conf_noc_width = x;
                conf_noc_height = 1;
                user_printf("No square number, assumming %dx1 nodes.\n", x);
            }
        } else if (n==3 && (ch=='x' || ch=='X')) {
            if (x>MAX_RANK) {
                user_printf("No more than %d cores can be simulated.\n", MAX_RANK);
                break;
            }
            conf_noc_width = x;
            conf_noc_height = y;
        } else {
            user_printf("Number of cores expected.\n");
            break;
        }

        user_printf("Resetting %dx%d cores...\n", conf_noc_width, conf_noc_width);
        noc_destroy_all(nodes, conf_max_rank);
        for (r=0; r<conf_max_rank; r++) {
            core_finish_context(nodes[r]);
        }
        conf_max_rank = conf_noc_width*conf_noc_height;

        core_init_all(nodes, conf_max_rank);
        noc_init_all(nodes, conf_noc_type, conf_noc_width, conf_noc_height);
        break;
    }

    case 'x': // run to end and store each context to a file
    {
        char path[512];
        char filename[512];
        rank_t r;
        bool all_stopped;
        int cycle_to;
        int cycle_stepsize;
        if (!argument) {
            user_printf("Output file path for context files expected.\n");
            break;
        }

        int n = sscanf(argument, "%s %d %d", path, &cycle_to, &cycle_stepsize);
        if (n != 3) {
            user_printf("Expected syntax: -x \"<path> <max cycle> <step size for output>\"\n");
            break;
        }

        // As long as there is a running core,
        // simulate one step and write the context to file.
        while (true) {
            cycle_t cycle = 0;
            for (r = 0; r < conf_max_rank; r++) {
                // Do not store the state for stopped cores.
                if (nodes[r]->state == CS_STOPPED ||
                        nodes[r]->state == CS_UNKNOWN_INSTRUCTION) {
                    continue;
                }
                cycle = nodes[r]->cycle;
                if (!(cycle % cycle_stepsize)) {
                    sprintf(filename, "%smacsim_cpu_%ld_cycle_%ld", path, r, nodes[r]->cycle);
                    core_dump_context(filename, nodes[r]);
                }
            }

            if (!(cycle % cycle_stepsize)) {
                sprintf(filename, "%smacsim_noc_cycle_%ld", path, cycle);
                noc_dump_context(filename, nodes, conf_max_rank);
            }

            all_stopped = true;
            // Check if all cores have stopped running.
            for (r = 0; r < conf_max_rank; r++) {
                if (nodes[r]->state != CS_STOPPED &&
                        nodes[r]->state != CS_UNKNOWN_INSTRUCTION) {
                    all_stopped = false;
                    break;
                }
            }
            if (all_stopped || cycle >= cycle_to) {
                for (r = 0; r < conf_max_rank; r++) {
                    // Write the stop cycle to file.
                    sprintf(filename, "%smacsim_cpu_%ld_end_cycle", path, r);
                    FILE *out = fopen(filename, "w");
                    if (out == NULL) {
                        fatal("Could not open '%s'", filename);
                    }
                    fprintf(out, "%ld", nodes[r]->cycle);
                    fclose(out);
                }

                break;
            }
            simulation_step();
        }
        break;
    }


    default:
        user_printf("Unknown command '%c'\n", command);
    }
}




int main(int argc, char *argv[])
{
    rank_t r;
    char line[MAX_INPUT_LINE];

    init_streaming();
    conf_inj_rate = 0.1;
    conf_inj_prob = llrint(0x1.0p64 * conf_inj_rate);
    conf_max_rank = 4;
    conf_noc_width = 2;
    conf_noc_height = 2;
    conf_noc_type = NT_perfect;
    for (r=0; r<MAX_RANK; r++)
    {
        nodes[r] = malloc(sizeof(node_t));
        nodes[r]->rank = r;
    }
    for (r=0; r<conf_max_rank; r++)
    {
        nodes[r]->core_type = CT_armv6m;
    }
    core_init_all(nodes, conf_max_rank);

    noc_init_all(nodes, conf_noc_type, conf_noc_width, conf_noc_height);
    node = nodes[0];

    // process command line
    unsigned i;
    for (i=1; i<argc; i++)
    {
        char *s = argv[i];
        if (s[0]!='-')
        {
            printf("This is MacSim. A cycle accurate manycore simulator.\n"
                   "Usage: %s [-<commands>]\n", argv[0]);
            return 1;
        }
        
        // deal with spaces between command and arguments, ie "-a foo.elf"
        if (s[2]==0 && i+1<argc && argv[i+1][0]!='-')
    	    process_line(s[1], argv[++i]);
    	else
    	    process_line(s[1], trim_whitespace(s+2));
    }

    // react on user input
    while (1)
    {
        user_printf(">");
        if (fgets(line, MAX_INPUT_LINE, stdin))
            ; // to avoid warning
        char *l = trim_whitespace(line); // ignore leading whitespace
        if (!l)
            // if only enter pressed, simulate one cycle
            process_line('s', 0);
        else
            process_line(l[0], trim_whitespace(l+1));
    }
}

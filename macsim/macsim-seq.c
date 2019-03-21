#include "memory.h"
#include "trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h> // for mkdir()
#include "string_map.h"

#define MAX_INPUT_LINE 1024

unsigned long long NT_READ_AHEAD = 16;


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

void set_client_arguments(char *argstr)
{
    size_t len = strlen(argstr)+1;
    char argbuf[len+1];

    int argc=0, i=0, j=0;
    char ch = argstr[i++];
    while (i<len) {
        while ((i<len) && ((ch==' ') || (ch=='\t')))
            ch = argstr[i++];
        argc++;
        while ((i<len) && ((ch!=' ') && (ch!='\t'))) {
            argbuf[j++] = ch;
            ch = argstr[i++];
        }
        argbuf[j++] = 0;
    }

    rank_t r;
    for (r=0; r<conf_max_rank; r++) {
        nodes[r]->set_argv(nodes[r], argc, argbuf);
    }
}

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
// FIXME: Should node->cycle be also incremented if node was stopped? yes
//        Save stop time in seperate variable?
void simulate_one_instruction(node_t *node)
{
    uint_fast32_t cs = node->state;

    if ((node->rank ) == 0) {
//         printf("%ld: %ld %ld\n", node->rank, node->cycle, node->pc);
    }

    if (node->rank == 0) {
//         printf("%ld\n", node->cycle);
    }
    if (cs>CS_RUNNING) {        // multicycle instruction
        node->cycle++;
        node->exit_cycle++;
        node->state = cs-1;
        if (cs==CS_RUNNING+1) node->pc = node->nextpc;
    } else if (CS_READY(cs)) {
        instruction_class_t ic = node->one_cycle(node);

        // Timing
        if (ic>0) {
            statistic_insert_instr(&node->stats, node->pc, ic);
            if (ic==1) node->pc = node->nextpc;
            node->cycle++;
            node->exit_cycle++;
            node->state = CS_RUNNING-1+ic; // store multiple cycles in state
        } else if (ic==IC_BLOCKED) {
            statistic_insert_instr(&node->stats, node->pc, 1);
            // If send or receive blocks, don't increase the PC.
            // Thereby the instruction is executed again and again, until
            // the blocking is over.
            node->cycle++;
            node->exit_cycle++;
        } else if (ic==IC_STOP) {
            reason_for_stop(node);
        } else fatal("Unknown instruction class %d.", ic);
    } else {
        //continue to increase clock
        //this is very important for the noc, because
        //the local execution may be finished, but the noc
        //needs to work until all flits were transported
        node->cycle++;
    }
}


// Simulate all cores and the periodical interconnect for one cycle only
void simulation_step()
{
    // simulate interconnect
    noc_route_all(nodes, conf_max_rank);

    // only netrace simulation: inject messages
    if (nodes[0]->core_type==CT_netrace)
        netrace_inject_messages(nodes);

    // simulate all cores for one cycle
    rank_t r;
    for (r=0; r<conf_max_rank; r++) {
        simulate_one_instruction(nodes[r]);
    }
}


uint_fast64_t get_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec + 1000000000 * (uint_fast64_t)ts.tv_sec;
}



// Simulate all cores and the periodical interconnect
void simulation()
{
    rank_t r;
    cycle_t min=CYCLE_T_MAX;
    cycle_t max=0;

    // special case for fast netrace simulation
    if (nodes[0]->core_type==CT_netrace)
    {
        uint_fast64_t timestamp = get_timestamp();
        while (netrace_inject_messages(nodes)) {
            rank_t r;
            for (r=0; r<conf_max_rank; r++)
                simulate_one_instruction(nodes[r]);
            noc_route_all(nodes, conf_max_rank);

#define LOG_PERIOD 20L
            if ((nodes[0]->cycle & ((1<<LOG_PERIOD)-1))==0) {
                uint_fast64_t now = get_timestamp();
                fprintf(stderr, "%lu cyc/s %lu Mcyc",
                        ((1<<LOG_PERIOD)*1000000000L) / (now-timestamp),
                        nodes[0]->cycle / 1000000L );

                netrace_print_context(nodes[0]);

                timestamp = now;
            }
        }
        return;
    }


    for (r=0; r<conf_max_rank; r++)
    {
        // determine, which cores are still running
        if (CS_READY(nodes[r]->state)) {
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


// number of cycles per measurement
#define conf_eval_cycles 100000


// compute average latency of one injection rate and return true injection rate
double print_one_point(double injrate, double *latency)
{
    cycle_t i;
    rank_t r;

    noc_destroy_all(nodes, conf_max_rank);
    for (r=0; r<conf_max_rank; r++) {
        core_finish_context(nodes[r]);
    }
    core_init_all(nodes, conf_max_rank);
    noc_init_all(nodes, conf_noc_type, conf_noc_width, conf_noc_height);



    conf_inj_rate = injrate; // superflous
    conf_inj_prob = llrint(0x1.0p64 * injrate);
    for (i = 0; i < conf_eval_cycles; i++) simulation_step();

    cycle_t count=0;
    cycle_t total=0;
    for (r=0; r<conf_max_rank; r++) {
        count += nodes[r]->core.traffic.stat_flit_count;
        total += nodes[r]->core.traffic.stat_total_latency;
    }
    double real_injrate = (double)count / (double)(conf_eval_cycles*conf_max_rank);
    *latency = (double)total / (double)count;
    core_printf(0, "%.7f %.7f %7.1f %lu %lu\n",
        injrate, real_injrate, *latency, count, total);

    return real_injrate;
}


// compute a latency vs throughtput diagram
// by systematically incresing the injection rate
//
// possible improvements:
//   * increase accuracy by avoiding floating point math by using conf_inj_prob
//     instead of conf_inj_rate
//   * let user change conf_eval_cycles (duration of one measurement)
//   * warm up phase before actual measurement
//   * cool down: compute latency of remaining messages after end of interval
void latency_vs_throughput()
{
    core_printf(0, "target/real injrate latency #flits total latency\n");

    // phase 1:
    // Start with 1 flit/NoC/cycle and compute zero-load latency.
    double step = 1.0 / conf_max_rank; // one flit/cycle in the whole NoC
    double rise_ir = 0.0; // injrate before a significant rise or saturation
    double lat;
    double prev_lat;
    double desired_ir = step;
    double ir = print_one_point(desired_ir, &lat);
    double zero_load_latency = lat;

    // phase 2:
    // Increase injection rate in steps of 1 flit/NoC/cycle,
    // until the measured injrate is more than 5% below the desired value
    while ((ir/desired_ir)>0.95) {
        prev_lat = lat;
        desired_ir += step;
        ir = print_one_point(desired_ir, &lat);
        if (rise_ir==0.0 && (lat > 2.0*prev_lat)) rise_ir = desired_ir-step;
            // remember the injrate, if significant rise of latency
    }
    if (rise_ir==0.0) rise_ir = desired_ir-step;

    // phase 3:
    // We reached the saturation throughput.
    // Now inspect the interal between the last injrate before the raise
    // and the saturation injrate logarithmically
    float saturation_ir = ir;
    step = saturation_ir - rise_ir;
    desired_ir = rise_ir;
    while (step > 0.0001) {
        step = step * 0.5;
        desired_ir += step;
        ir = print_one_point(desired_ir, &lat);
    }

    // phase 4:
    // Further increase injrate to see if we can achieve a higher saturation
    // throughput. Stop when the latency is 1000x the zero load latency.
    step = 1.0 / conf_max_rank; // one flit/cycle in the whole NoC
    while (lat < 1000.0*zero_load_latency && desired_ir<1.0) {
        desired_ir += step;
        ir = print_one_point(desired_ir, &lat);
        if (ir > saturation_ir) saturation_ir = ir;
    }

    core_printf(0, "saturation throughput: %g flits/core/cycle\n", saturation_ir);
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

void long_to_binary(int64_t value,char *binary)
{
    int i;
    for(i=0; i<64; i++) {
        if((value & 0x01) != 0)
            binary[63-i] = '1';
        else
            binary[63-i] = '0';
        value >>= 1;
    }
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
        if (!argument)
        {
            user_printf("Filename expected.\n");
            break;
        }
        if (nodes[0]->core_type == CT_netrace) {
            user_printf("Loading netrace trace file '%s'\n", argument);
            netrace_open_file(argument);
        } else {
            rank_t r;
            user_printf("Loading '%s' to all cores\n", argument);
            for (r=0; r<conf_max_rank; r++)
                core_finish_context(nodes[r]);
            core_init_all(nodes, conf_max_rank);
            for (r=0; r<conf_max_rank; r++) {
                memory_load_file(nodes[r], argument);
            }
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

    case 'g': // goto breakpoint or determine traffic saturation
    {
        clock_t duration = clock();
        if (node->core_type == CT_traffic) {
            latency_vs_throughput();
        } else {
            unsigned long bp;
            int count = 0;
            if (argument) {
                count = sscanf(argument, "%lx", &bp);
                if (count>0 && !add_breakpoint(node, bp)) break;
            }

            simulation();

            if (count>0)
                remove_breakpoint(node, bp);
            break;
        }
        duration = clock() - duration;
        if ((duration/CLOCKS_PER_SEC)>0)
            user_printf("Simulation took %.1fs, %ld cycles/s\n",
                        (double)duration/(double)CLOCKS_PER_SEC,
                        (node->cycle*CLOCKS_PER_SEC) / duration);
        break;
    }

    case 'h': // help
        user_printf(
            "Configuration\n"
            "A [name]      set ISA of all cores (armv6m, riscv)\n"
            "I [float]     set injection rate for synthetic traffic patters\n"
            "R [name]      set routing algorithm (fixedlat, pnbe0, pnbe1, pnbe2, caerus, \n"
            "              pnoa0, pnoa1, pnaa, pnoo, pnood, pnbase, pnjm0)\n"
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
            "k traffic     shows the traffic of the current core\n"
            "k instr       show the instruction-statistic of the current core\n"
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
        cycle_t minExecTime = INT32_MAX;
        cycle_t maxExecTime = 0;
        cycle_t totalExecTime = 0;
        noc_destroy_all(nodes, conf_max_rank);
        for (r=0; r<conf_max_rank; r++) {
            if (nodes[r]->exit_cycle < minExecTime) {
                minExecTime = nodes[r]->exit_cycle;
            }
            if (maxExecTime < nodes[r]->exit_cycle) {
                maxExecTime = nodes[r]->exit_cycle;
            }
            totalExecTime += nodes[r]->exit_cycle;

            core_finish_context(nodes[r]);
        }
        printf("\nMax. Execution Time: %lu cycles\n", maxExecTime);
        printf("Min. Execution Time: %lu cycles\n", minExecTime);
        printf("Total Execution Time: %lu cycles\n", totalExecTime);
        printf("Avg. Execution Time: %f cycles\n", (double)totalExecTime / conf_max_rank);
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

    case 'k':
        if (!argument) {
            printf("Usage: -k instr | traffic \n");
            break;
        }

        if (strcmp(argument, "instr") == 0) {
            string_map_t cycleMap;
            string_map_init(&cycleMap);
            printf("  %8s | %12s | %8s | %s\n", "Address", "#Executions", "#Cycles", "Instruction");
            for (int i = 0; i < node->stats.instrs_length; i++) {
                if (node->stats.instrs[i].hits > 0) {
                    char dstr[MAX_DISASM_STR];
                    memset(dstr, 0, MAX_DISASM_STR);
                    core_disasm(node, i, dstr);
                    printf("0x%08x | %12ld | %8ld | %s\n", i, node->stats.instrs[i].hits, node->stats.instrs[i].cycles, dstr);

                    for (int j = 0; j < MAX_DISASM_STR; j++) {
                        if (dstr[j] == ' ' || dstr[j] == '\t') {
                            dstr[j] = '\0';
                        }
                    }
                    uint64_t currentCycle = 0;
                    string_map_get(&cycleMap, dstr, &currentCycle);
                    string_map_insert(&cycleMap, dstr, currentCycle + node->stats.instrs[i].cycles);
                }
            }
            string_map_print(&cycleMap);
            printf("Total: %ld\n", node->exit_cycle);
            string_map_free(&cycleMap);
        }
        if (strcmp(argument, "traffic") == 0) {
            uint64_t bytesInput = 0;
            uint64_t bytesOutput = 0;
            for (int i = 0; i < node->stats.send_flits_length; i++) {
                if (node->stats.send_flits[i] > 0) {
                    printf("%ld --> %d: %d flits\n", node->rank, i, node->stats.send_flits[i]);
                    bytesOutput += node->stats.send_flits[i];
                }
            }
            for (int i = 0; i < node->stats.recv_flits_length; i++) {
                if (node->stats.recv_flits[i] > 0) {
                    printf("%d --> %ld: %d flits\n", i, node->rank, node->stats.recv_flits[i]);
                    bytesInput += node->stats.recv_flits[i];
                }
            }

            bytesInput *= 8;
            bytesOutput *= 8;

            printf("Bandwidth:\n");
            printf("Input:  %9ld Bytes | %9.3f Byte/Cycle\n", bytesInput, (double)bytesInput / node->exit_cycle);
            printf("Output: %9ld Bytes | %9.3f Byte/Cycle\n", bytesOutput, (double)bytesOutput / node->exit_cycle);

        }
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
        rank_t new_max_rank = conf_max_rank;
        rank_t r;
        uint_fast16_t ct;
        uint_fast16_t tp;

        if (strcmp(argument, "armv6m")==0)         ct=CT_armv6m;
        else if (strcmp(argument, "armv3")==0)     ct=CT_armv3;
//        else if (strcmp(argument, "mips32"))    ct=CT_mips32;
//        else if (strcmp(argument, "or32"))      ct=CT_or32;
//        else if (strcmp(argument, "patmos"))    ct=CT_patmos;
        else if (strcmp(argument, "riscv")==0 || strcmp(argument, "rv64i")==0) {
            ct=CT_riscv;
        }
        else if (strcmp(argument, "rvmpb")==0) {
            ct=CT_rvmpb;
        }
//        else if (strcmp(argument, "tricore"))   ct=CT_tricore;
        else if (strcmp(argument, "netrace")==0)   ct=CT_netrace;
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
        conf_max_rank = new_max_rank;
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

        user_printf("Resetting %dx%d cores...\n", conf_noc_width, conf_noc_height);
        noc_destroy_all(nodes, conf_max_rank);
        for (r=0; r<conf_max_rank; r++) {
            core_finish_context(nodes[r]);
        }
        conf_max_rank = conf_noc_width*conf_noc_height;
        conf_recv_fifo_size = 2*conf_max_rank;

        core_init_all(nodes, conf_max_rank);
        noc_init_all(nodes, conf_noc_type, conf_noc_width, conf_noc_height);
        break;
    }

    case 'P': // set command line arguments of simulated program                            
          set_client_arguments(argument);                                                   
          break; 

    case 'R': // set NoC routing algorithm
    {
        uint_fast16_t nt;
        char *s;
        if (strcmp(argument, "fixedlat")==0) {
            nt=NT_fixedlat;
            s="Fixed latency";
        } else if (strcmp(argument, "manhattan")==0) {
            nt=NT_manhattan;
            s="Manhattan latency";
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
            } else if (strcmp(argument, "pnoa0")==0) {
                nt=NT_pnoa0;
                s="PaterNoster One-to-All";
            } else if (strcmp(argument, "pnoa1")==0) {
                nt=NT_pnoa1;
                s="PaterNoster One-to-All (incl. receive READY in hardware)";
            } else if (strcmp(argument, "pnaa")==0) {
                nt=NT_pnaa;
                s="PaterNoster All-to-All";
            } else if (strcmp(argument, "pnood")==0) {
                nt=NT_pnoo;
                s="PaterNoster One-to-One Dynamic buffer";
            } else if (strcmp(argument, "pnoo")==0) {
                nt=NT_pnoo;
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
            } else if (strcmp(argument, "debug")==0) {
                nt = NT_debug;
                s = "Perfect NoC with debug output";

            } else if (argument[0]=='P') {
                nt = NT_pnconfig;
                s = "PaterNoster best effort";
                switch (argument[1]) {
                case 'N':
                    conf_bypass_y = CONF_BYPASS_NONE;
                    break;
                case 'U':
                    conf_bypass_y = CONF_BYPASS_UNBUF;
                    break;
                case 'B':
                    conf_bypass_y = CONF_BYPASS_BUF;
                    break;
                case 'W':
                    conf_bypass_y = CONF_BYPASS_2UNBUF;
                    break;
                case 'D':
                    conf_bypass_y = CONF_BYPASS_2BUF;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[2]) {
                case 'N':
                    conf_bypass_x = CONF_BYPASS_NONE;
                    break;
                case 'U':
                    conf_bypass_x = CONF_BYPASS_UNBUF;
                    break;
                case 'B':
                    conf_bypass_x = CONF_BYPASS_BUF;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[3]) {
                case 'G':
                    conf_stall_y = CONF_STALL_CHEAP;
                    break;
                case 'S':
                    conf_stall_y = CONF_STALL_EXP;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[4]) {
                case 'G':
                    conf_stall_x = CONF_STALL_CHEAP;
                    break;
                case 'S':
                    conf_stall_x = CONF_STALL_EXP;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[5]) {
                case '0':
                    conf_inject_y = CONF_INJECT_NONE;
                    break;
                case 'R':
                    conf_inject_y = CONF_INJECT_REQUEST;
                    break;
                case 'A':
                    conf_inject_y = CONF_INJECT_ALTERNATE;
                    break;
                case 'T':
                    conf_inject_y = CONF_INJECT_THROTTLE;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
                }
                switch (argument[6]) {
                case '0':
                    conf_inject_x = CONF_INJECT_NONE;
                    break;
                case 'R':
                    conf_inject_x = CONF_INJECT_REQUEST;
                    break;
                case 'A':
                    conf_inject_x = CONF_INJECT_ALTERNATE;
                    break;
                case 'T':
                    conf_inject_x = CONF_INJECT_THROTTLE;
                    break;
                default:
                    fatal("Unknown configuration '%s'.\n", argument);
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

/*
    case 'T': // load netrace file
    {
        rank_t r;
        if (!argument)
        {
            user_printf("Filename expected.\n");
            break;
        }
        user_printf("Loading netrace from '%s'.\n", argument);
        nt_open_trfile(argument);
        conf_netrace_on = 1;
        // clean all NoC buffers?
        break;
    }
*/

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
                if (nodes[r]->state == CS_STOPPED || nodes[r]->state == CS_UNKNOWN_INSTRUCTION) {
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

    case 'X':
        // same as -x, but use directory tree
        // run to end and store each context to a file
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
        cycle_t cycle = 0;
        while (true) {
//            assert(cycle == nodes[0]->cycle);
            unsigned long cdir = cycle / 1000;
            unsigned cfile = cycle % 1000;

            for (r = 0; r < conf_max_rank; r++) {
                if (cfile==0) {
                    sprintf(filename, "%smacsim_cpu_%ld/%lu", path, r, cdir);
                    mkdir(filename, ACCESSPERMS);
                    sprintf(filename, "%smodelsim_cpu_%ld/%lu", path, r, cdir);
                    mkdir(filename, ACCESSPERMS);
                }

                // Do not store the state for stopped cores.
                if (nodes[r]->state == CS_STOPPED ||
                        nodes[r]->state == CS_UNKNOWN_INSTRUCTION) {
                    continue;
                }

                if (!(cycle % cycle_stepsize)) {
                    sprintf(filename, "%smacsim_cpu_%ld/%lu/cycle_%lu",
                            path, r, cdir, cycle);
                    core_dump_context(filename, nodes[r]);
                }
            }

            if (cfile==0) {
                sprintf(filename, "%smacsim_noc/%lu", path, cdir);
                mkdir(filename, ACCESSPERMS);
                sprintf(filename, "%smodelsim_noc/%lu", path, cdir);
                mkdir(filename, ACCESSPERMS);
            }
            if (!(cycle % cycle_stepsize)) {
                sprintf(filename, "%smacsim_noc/%lu/cycle_%lu",
                        path, cdir, cycle);
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
                    sprintf(filename, "%smacsim_cpu_%ld/end_cycle", path, r);
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
            cycle++;
        }
        break;
    }



    case 'G':
    {
        unsigned n;
        int count = sscanf(argument, "%u", &n);
        if (count!=1) fatal("Send buffer size expected");
        conf_send_fifo_size = n;
        break;
    }

    case 'H':
    {
        unsigned n;
        int count = sscanf(argument, "%u", &n);
        if (count!=1) fatal("Receive buffer size expected");
        conf_recv_fifo_size = n;
        break;
    }

    case 'J':
    {
        unsigned n;
        int count = sscanf(argument, "%u", &n);
        if (count!=1) fatal("Corner buffer size expected");
        conf_corner_fifo_size = n;
        break;
    }


    // can be removed
    case 'z':
    {
        int32_t cycles;
        int count = sscanf(argument, "%d", &cycles);
        if (count!=1) fatal("Number of ahead cycles expected");
        NT_READ_AHEAD = cycles;
        break;
    }

    case 'y': //Run the loaded program and log core register changes
    {
        char path[512];
        FILE *out;
        rank_t r;
        bool all_stopped;
        int64_t all_registers[conf_max_rank*32];
        uint64_t all_fregs[conf_max_rank*32];
        for(r=0; r<conf_max_rank*32; r++) {
            all_registers[r] = 0;
            all_fregs[r] = 0;
        }
        if (!argument) {
            user_printf("Output path and filename for context file expected.\n");
            break;
        }

        int n = sscanf(argument, "%s", path);
        if (n != 1) {
            user_printf("Expected syntax: -y \"<path>/filename\"\n");
            break;
        }

        out = fopen(path,"w");
        if( out == NULL) {
            fatal("Could not open the specified log file");
        }
        fclose(out);
        // While a core is still running, simulate one step and log register changes
        cycle_t cy = 0;

        out = fopen(path,"a");
        if( out == NULL) {
            fatal("Could not open the specified log file");
        }

        do {
            // Simulate one step
            simulation_step();


            fprintf(out,"\n#%lu\n",cy);
            cy++;
            // Log registers
            for(r=0; r<conf_max_rank; r++) {
                // Check register change not before the end of the latency,
                // because the FPGA also writes them at the end.
                // Early change is necessary for the following opcodes
                //   0x63 branch (don't mind, does not change registers)
                //   0x67 jalr
                //   0x6b recv (others don't mind)
                //   0x6f jar
                //   0x03 ld
                //   0x07 fld
                if ((nodes[r]->state <= CS_RUNNING)
                        || ((nodes[r]->instruction_word&0x73)==0x63)
                        || ((nodes[r]->instruction_word&0x7b)==0x03))
                {
                    // don't dump during stalls
                    int i;
//                    char binary[64];
                    for(i=1; i<32; i++) {	// ignore the content of reg0
                        if(nodes[r]->core.riscv.reg[i] != all_registers[r*32+i]) {
//                            long_to_binary(nodes[r]->core.riscv.reg[i], binary);
                            fprintf(out,"°%ld x%u %lx\n", r, i, nodes[r]->core.riscv.reg[i]);
                        }
                        all_registers[r*32+i] = nodes[r]->core.riscv.reg[i];
                    }
                    for (i=0; i<32; i++) {
                        uf64_t x;
                        x.f = nodes[r]->core.riscv.freg[i];
                        if (x.u != all_fregs[r*32+i]) {
                            fprintf(out,"°%ld f%u %lx\n", r, i, x.u);
                        }
                        all_fregs[r*32+i] = x.u;
                    }
                }
            }
            // Check if a core is still running
            all_stopped = true;
            for(r=0; r<conf_max_rank; r++) {
                if (CS_READY(nodes[r]->state)) {
                    all_stopped = false;
                    break;
                }
            }
        } while(!all_stopped);

        fclose(out);

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
    conf_send_fifo_size = 8;
    conf_recv_fifo_size = 8;
    conf_corner_fifo_size = 8;
    for (r=0; r<MAX_RANK; r++)
    {
        nodes[r] = malloc(sizeof(node_t));
        nodes[r]->rank = r;
    }
    for (r=0; r<conf_max_rank; r++)
    {
        nodes[r]->core_type = CT_riscv;
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

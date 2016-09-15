/** 
 * $Id: memory.c 380 2012-09-12 09:09:28Z mischejo $
 * Memory simulation
 *
 * McSim project
 */
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <byteswap.h>



#define BITS_LEVEL_0	12
#define BITS_LEVEL_1	10
#define BITS_LEVEL_2	(32-BITS_LEVEL_1-BITS_LEVEL_0)

#define PAGE_SIZE_L0	((1<<BITS_LEVEL_0))
#define PAGE_SIZE_L1	((1<<BITS_LEVEL_1))
#define PAGE_SIZE_L2	((1<<BITS_LEVEL_2))

#define DUMP_BYTES_PER_LINE	16


// Init memory
void memory_init(node_t *node, uint16_min_t mt, uint64_min_t ms)
{
    node->memory_type = mt;
    node->memory_size = ms;
    switch (mt)
    {
	case MT_PAGED_32BIT:
	    node->memory.map = fatal_malloc(sizeof(char*[PAGE_SIZE_L2]));
	    memset(node->memory.map, 0, sizeof(char*[PAGE_SIZE_L2]));
	    break;
	case MT_LINEAR:
	    node->memory.u8 = fatal_malloc(node->memory_size);
	    memset(node->memory.u8, 0, node->memory_size);
	    break;
	default:
	    fatal("Unknown memory type %d", node->memory_type);
    }
}


// Finish memory, i.e. free allocated memory blocks
void memory_finish(node_t *node)
{
    switch (node->memory_type)
    {
	case MT_PAGED_32BIT:
	{
	    uint32_min_t o2;
	    for (o2=0; o2<PAGE_SIZE_L2; o2++)
	    {
		uint8_t **l1 = node->memory.map[o2];
		if (l1)
		{
		    uint32_min_t o1;
		    for (o1=0; o1<PAGE_SIZE_L1; o1++)
		    {
			uint8_t *l0 = l1[o1];
			if (l0) free(l0);
		    }
		    free(l1);
		}
	    }
	    free(node->memory.map);
	    break;
	}
	case MT_LINEAR:
	    free(node->memory.u8);
	    break;
	default:
	    fatal("Unknown memory type %d", node->memory_type);
    }

}


// read via Pagetable, Variable length
void memory_read_var_p(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len)
{
    uint8_t *l0;
    uint32_min_t o;

    while (1)
    {
	o = addr>>(BITS_LEVEL_0+BITS_LEVEL_1);
	uint8_t **l1 = node->memory.map[o];
        if (!l1)
        {
    	    l1 = fatal_malloc(sizeof(char*[PAGE_SIZE_L1]));
	    memset(l1, 0, sizeof(char*[PAGE_SIZE_L1]));
    	    node->memory.map[o] = l1;
	}

	o = (addr>>BITS_LEVEL_0) & (PAGE_SIZE_L1-1);
	l0 = l1[o];
        if (!l0)
        {
    	    l0 = fatal_malloc(PAGE_SIZE_L0);
	    memset(l0, 0, PAGE_SIZE_L0);
    	    l1[o] = l0;
	}

	o = addr & (PAGE_SIZE_L0-1);
	uint32_min_t rem = PAGE_SIZE_L0 - o;
	if (len <= rem) break;

	memcpy(dest, l0+o, rem);
	dest += rem;
	len -= rem;
	addr += rem;
    }
    memcpy(dest, l0+o, len);
}


// write via Pagetable, Variable length
void memory_write_var_p(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len)
{
    uint8_t *l0;
    uint32_min_t o;

    while (1)
    {
	o = addr>>(BITS_LEVEL_0+BITS_LEVEL_1);
	uint8_t **l1 = node->memory.map[o];
        if (!l1)
        {
    	    l1 = fatal_malloc(sizeof(char*[PAGE_SIZE_L1]));
	    memset(l1, 0, sizeof(char*[PAGE_SIZE_L1]));
    	    node->memory.map[o] = l1;
	}

	o = (addr>>BITS_LEVEL_0) & (PAGE_SIZE_L1-1);
	l0 = l1[o];
        if (!l0)
        {
    	    l0 = fatal_malloc(PAGE_SIZE_L0);
	    memset(l0, 0, PAGE_SIZE_L0);
    	    l1[o] = l0;
	}

	o = addr & (PAGE_SIZE_L0-1);
	uint32_min_t rem = PAGE_SIZE_L0 - o;
	if (len <= rem) break;

	memcpy(l0+o, src, rem);
	src += rem;
	len -= rem;
	addr += rem;
    }
    memcpy(l0+o, src, len);
}


// read from linear table, Variable length
void memory_read_var_l(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len)
{
    LINEAR_MEMORY_SIZE_CHECK
    memcpy(dest, node->memory.u8+addr, len);
}


// write to linear table, Variable length
void memory_write_var_l(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len)
{
    LINEAR_MEMORY_SIZE_CHECK
    memcpy(node->memory.u8+addr, src, len);
}


// Generic memory read
void memory_read(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len)
{
    switch (node->memory_type)
    {
	case MT_PAGED_32BIT:	memory_read_var_p(node, addr, dest, len); return;
	case MT_LINEAR:		memory_read_var_l(node, addr, dest, len); return;
	default:
	    fatal("Unknown memory type %d", node->memory_type);
    }
}


// Generic memory write
void memory_write(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len)
{
    switch (node->memory_type)
    {
	case MT_PAGED_32BIT:	memory_write_var_p(node, addr, src, len); return;
	case MT_LINEAR:		memory_write_var_l(node, addr, src, len); return;
	default:
	    fatal("Unknown memory type %d", node->memory_type);
    }
}


// Print a memory dump
void memory_print_dump(node_t *node, addr_t from, addr_t to)
{
    addr_t addr;
    unsigned remain = to-from+1;
    unsigned line_len, j;
    uint8_t line[DUMP_BYTES_PER_LINE];
    
    addr = from;
    while (remain>0)
    {
	line_len = remain>DUMP_BYTES_PER_LINE ? DUMP_BYTES_PER_LINE : remain;
        memory_read((node_t *)node, addr, line, line_len);

	printf("%08x:", (unsigned)addr);
	for (j=0; j<line_len; j++)
	{
	    printf("%c%02x", (j==DUMP_BYTES_PER_LINE/2) ? '-' : ' ', line[j]);
	}
	printf("  ");
	for (j=0; j<line_len; j++)
	{
	    uint8_t b = line[j];
	    printf("%c", (b>=32) && (b<128) ? b : '.');
	}
	printf("\n");
	
	remain -= line_len;
	addr += line_len;
    }
}


// General disassembler, if no other available
// Just prints 32 bit words
int no_disasm(node_t *node, addr_t pc, char *dstr)
{
    uint32_t iw;
    memory_read(node, node->pc, (uint8_t *)&iw, 4);
    snprintf(dstr, 32, ".word\t0x%08x", iw);
    return 4;
}





#define ELF_32LE 0
#define ELF_32BE 1
#define ELF_64LE 2
#define ELF_64BE 3

#define ELF_GET_EH(__et, __eh, __v) \
    ((__et&2)==0 ? ((__et&1)==0 ? ENDIAN_32le(((Elf32_Ehdr *)__eh)->__v) \
                                : ENDIAN_32be(((Elf32_Ehdr *)__eh)->__v)) \
                 : ((__et&1)==0 ? ENDIAN_64le(((Elf64_Ehdr *)__eh)->__v)   \
                                : ENDIAN_64be(((Elf64_Ehdr *)__eh)->__v)))

#define ELF_GET_PH(__et, __ph, __v) \
    ((__et&2)==0 ? ((__et&1)==0 ? ENDIAN_32le(((Elf32_Phdr *)__ph)->__v) \
                                : ENDIAN_32be(((Elf32_Phdr *)__ph)->__v)) \
                 : ((__et&1)==0 ? ENDIAN_64le(((Elf64_Phdr *)__ph)->__v)   \
                                : ENDIAN_64be(((Elf64_Phdr *)__ph)->__v)))


// Load little endian ELF binary into memory
void memory_load_elf(node_t *node, unsigned et, uint8_t *buf) 
{
    uint_fast64_t i;
    node->pc            = ELF_GET_EH(et, buf, e_entry);
    uint_fast64_t phnum = ELF_GET_EH(et, buf, e_phnum);

    // go through every progam header entry, each one describes one segment
    for (i=0; i<phnum; i++) {
        uint_fast64_t phoff     = ELF_GET_EH(et, buf, e_phoff);
        uint_fast64_t phentsize = ELF_GET_EH(et, buf, e_phentsize);
        uint8_t *ph = buf + phoff + i*phentsize;

        uint_fast64_t type   = ELF_GET_PH(et, ph, p_type);
        uint_fast64_t filesz = ELF_GET_PH(et, ph, p_filesz);
        uint_fast64_t paddr  = ELF_GET_PH(et, ph, p_paddr);
        uint_fast64_t offset = ELF_GET_PH(et, ph, p_offset);
        uint_fast64_t memsz  = ELF_GET_PH(et, ph, p_memsz);

        if (type == PT_LOAD) { // only one type of segment must be loaded
            // load code to physical address
            if (filesz > 0)
                memory_write(node, paddr, buf+offset, filesz);

            // fill rest of segment with 0
            if (filesz < memsz) {
                uint8_t *zeros = (uint8_t *)fatal_malloc(memsz-filesz);
                memset(zeros, 0, memsz-filesz);
                memory_write(node, paddr+filesz, zeros, memsz-filesz);
                free(zeros);
            }
        }
    }
}


// Detect file format and load it into memory
// Supported: ELF, raw binary
void memory_load_file(node_t *node, const char *filename)
{
    unsigned long int size;
    uint8_t *buf;

    buf = read_whole_file(filename, &size);
    if (buf==0) fatal("Could not read file");

    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)buf;
    if (elf_header->e_ident[0]==ELFMAG0 &&
        elf_header->e_ident[1]==ELFMAG1 &&
        elf_header->e_ident[2]==ELFMAG2 &&
        elf_header->e_ident[3]==ELFMAG3)
    {
        // ELF recognised
        unsigned elf_type;
        switch (buf[EI_CLASS]) {
        case ELFCLASS32:
            switch (buf[EI_DATA]) {
            case ELFDATA2LSB: elf_type = ELF_32LE; break;
            case ELFDATA2MSB: elf_type = ELF_32BE; break;
            default: fatal("Unknown ELF byte order");
            }
            break;
        case ELFCLASS64:
            switch (buf[EI_DATA]) {
            case ELFDATA2LSB: elf_type = ELF_64LE; break;
            case ELFDATA2MSB: elf_type = ELF_64BE; break;
            default: fatal("Unknown ELF byte order");
            }
            break;
        default: fatal("Unknown ELF word size");
        }
        memory_load_elf(node, elf_type, buf);
    } else {
        // write raw file directly to start of memory
        memory_write((node_t *)node, 0, buf, size);
    }
    free(buf);

    // ugly workaround
    if (node->core_type==CT_armv6m)
        node->pc = node->pc & ~1;
}


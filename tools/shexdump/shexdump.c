/* shexdump
 * Read an ELF-file and convert it to a hex format
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32

#define OF_ALTERA_HEX   0
#define OF_VERILOGHEX   1
#define OF_ALTERA_ROM   2

#define OF_MAREK_ARRAY  3
#define OF_MAREK_VECTOR	4

unsigned output_format = OF_ALTERA_HEX;


// Fatal error -> stream_err
void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "FATAL: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
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


void altera_rom_write_block(FILE *f, unsigned bytewidth, unsigned blocklen,
    uint_fast32_t addr, const unsigned char *buf, uint_fast32_t len)
{
    uint_fast32_t i;
    unsigned addrwidth = 0;
    while ((1<<addrwidth) < blocklen) addrwidth++;


    fprintf(f, "LIBRARY ieee;\n"
      "USE ieee.std_logic_1164.all;\n"
      "USE ieee.numeric_std.all;\n"
      "ENTITY imem IS PORT (\n"
      "  address : IN STD_LOGIC_VECTOR(%d DOWNTO 0);\n"
      "  clock : IN STD_LOGIC := '1';\n"
      "  q : OUT STD_LOGIC_VECTOR(%d DOWNTO 0));\n"
      "END imem;\n"
      "ARCHITECTURE SYN OF imem IS\n"
      "  TYPE TMem IS ARRAY(0 TO %d) OF STD_LOGIC_VECTOR(%d DOWNTO 0);\n"
      "  SIGNAL Mem : TMem := (\n",
      addrwidth-1, 8*bytewidth-1, blocklen-1, 8*bytewidth-1);
    for (i=0; i<len; i+=bytewidth) {
        unsigned long long u = *(unsigned long long *)(&buf[i]);
        fprintf(f, "    x\"%0*llX\"%c\n", 2*bytewidth, u & ((1LL<<(8*bytewidth))-1),
            i==blocklen-1 ? ' ' : ',');
        addr++;
    }
    i = i/bytewidth;
    while (i<blocklen) {
        fprintf(f, "    x\"%0*llX\"%c\n", 2*bytewidth, 0LL,
            i==blocklen-1 ? ' ' : ',');
        addr++;
        i++;
    }
    fprintf(f, "  );\n"
        "BEGIN\n"
        "  PROCESS (clock)\n"
        "  BEGIN\n"
        "    IF RISING_EDGE(clock) THEN\n"
        "      q <= Mem(TO_INTEGER(UNSIGNED(address)));\n"
        "    END IF;\n"
        "  END PROCESS;\n"
        "END;\n"
    );
}






// deprecated -- replaced by marek_vector_* for new versions
void marek_array_write_prefix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    fprintf(f, "LIBRARY ieee;\n"
      "USE ieee.std_logic_1164.all;\n"
      "USE ieee.numeric_std.all;\n"
      "use work.libproc.all;\n"
      "ENTITY dmem IS PORT (\n"
      "  clk   : in  std_logic;\n"
      "  ici   : in  icache_in_type;\n"
      "  ico   : out icache_out_type;\n"
      "  dci   : in  dcache_in_type;\n"
      "  dco   : out dcache_out_type;\n"
      "  clken : in  std_logic);\n"
      "END dmem;\n"
      "ARCHITECTURE SYN OF dmem IS\n"
      "  TYPE TMem IS ARRAY(0 TO %d) OF STD_LOGIC_VECTOR(%d DOWNTO 0);\n"
      "  SIGNAL Registers : TMem := (\n",
      blocklen-1, 8*bytewidth-1);
}

// deprecated -- replaced by marek_vector_* for new versions
void marek_array_write_block(FILE *f, unsigned bytewidth, unsigned blocklen,
    uint_fast32_t addr, const unsigned char *buf, uint_fast32_t len)
{
    uint_fast32_t i;
    unsigned addrwidth = 0;
    while ((1<<addrwidth) < blocklen) addrwidth++;


    for (i=0; i<len; i+=bytewidth) {
        unsigned long long u = *(unsigned long long *)(&buf[i]);
        fprintf(f, "%lu => x\"%0*llx\"%c\n", addr, 2*bytewidth, u & ((1LL<<(8*bytewidth))-1),
            i==blocklen-1 ? ' ' : ',');
        addr++;
    }
    i = i/bytewidth;
}

// deprecated -- replaced by marek_vector_* for new versions
void marek_array_write_suffix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    fprintf(f, "  others=>(others=>'0'));\n"
"begin\n"
"  process(clk)\n"
"  variable i : integer;\n"
"  begin\n"
"    if rising_edge(clk) and clken = '1' then\n"
"      i := to_integer(unsigned(ici.addr(23 downto 2)))*4;\n"
"      ico.data <= Registers(i+3)&Registers(i+2)&Registers(i+1)&Registers(i+0);\n"
"      i := to_integer(unsigned(dci.addr(23 downto 3)))*8;\n"
"      if i < %u then\n"
"        if dci.we = '1' then\n"
"					if dci.byteen(7) = '1' then\n"
"						Registers(i+7) <= dci.wdata(63 downto 56);\n"
"					end if;\n"
"					if dci.byteen(6) = '1' then\n"
"						Registers(i+6) <= dci.wdata(55 downto 48);\n"
"					end if;\n"
"					if dci.byteen(5) = '1' then\n"
"						Registers(i+5) <= dci.wdata(47 downto 40);\n"
"					end if;\n"
"					if dci.byteen(4) = '1' then\n"
"						Registers(i+4) <= dci.wdata(39 downto 32);\n"
"					end if;\n"
"					if dci.byteen(3) = '1' then\n"
"						Registers(i+3) <= dci.wdata(31 downto 24);\n"
"					end if;\n"
"					if dci.byteen(2) = '1' then\n"
"						Registers(i+2) <= dci.wdata(23 downto 16);\n"
"					end if;\n"
"					if dci.byteen(1) = '1' then\n"
"						Registers(i+1) <= dci.wdata(15 downto 8);\n"
"					end if;\n"
"					if dci.byteen(0) = '1' then\n"
"						Registers(i+0) <= dci.wdata(7 downto 0);\n"
"					end if;\n"
"        end if;\n"
"        dco.rdata <= Registers(i+7)&Registers(i+6)&Registers(i+5)&Registers(i+4)\n"
"          &Registers(i+3)&Registers(i+2)&Registers(i+1)&Registers(i+0);\n"
"      else\n"
"        dco.rdata <= (others => '0');\n"
"      end if;\n"
"    end if;\n"
"  end process;\n"
"end SYN;\n"
    , blocklen);
}


void marek_vector_write_prefix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    fprintf(f, "LIBRARY ieee;\n"
      "USE ieee.std_logic_1164.all;\n"
      "\n"
      "PACKAGE dmem_content IS\n"
      "type MEM is array (0 to %d) of bit_vector(7 downto 0);\n"
      "constant INITB : bit_vector(0 to %d) := x\"",
      blocklen-1, 8*blocklen-1);
}


void marek_vector_write_block(FILE *f, unsigned bytewidth, unsigned blocklen,
    uint_fast32_t addr, const unsigned char *buf, uint_fast32_t len)
{
    uint_fast32_t i;
    
    for (i=0; i<len; i+=bytewidth) {
        unsigned long long u = *(unsigned long long *)(&buf[i]);
        fprintf(f, "%0*llx",2*bytewidth, u & ((1LL<<(8*bytewidth))-1));
        addr++;
    }
    while ( i < blocklen) {
	    fprintf(f, "00");
	    addr++;
	    i++;
    }
}


void marek_vector_write_suffix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    fprintf(f, "\";\n"
"function init_mem(content : bit_vector) return MEM;\n"
"constant INITA : MEM := init_mem(INITB);\n"
"END dmem_content;\n\n"

"package body dmem_content is\n"
"    function init_mem(content : bit_vector) return MEM is\n"
"        variable tmp : MEM;\n"
"        variable i : integer := 0;\n"
"        variable j : integer := 0;\n"
"        begin\n"
"            for X in content\'range loop\n"
"                tmp(i)(7-j) := content(X);\n"
"                j := j + 1;\n"
"                if (j = 8) then\n"
"                    i := i + 1;\n"
"                    j := 0;\n"
"                end if;\n"
"            end loop;\n"
"            return tmp;\n"
"    end function;\n"
"end package body;");

}



// Write one word of a specific bit width in a single hex line. Big Endian!
// Altera specific
void altera_write_bigendian(FILE *f, unsigned addr, unsigned bytewidth,
    const unsigned char *buf)
{
    unsigned i;
    unsigned crc = bytewidth + (addr>>8) + (addr&255) + 0;

    fprintf(f, ":%02X%04X00", bytewidth, addr);
    for (i=0; i<bytewidth; i++) {
        unsigned b = buf[bytewidth-1-i];
        fprintf(f, "%02X", b);
        crc += b;
    }
    fprintf(f, "%02X\r\n", (-crc)&0xff);
}


static unsigned char zero[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void altera_hex_write_block(FILE *f, unsigned bytewidth, unsigned blocklen,
    uint_fast32_t addr, const unsigned char *buf, uint_fast32_t len)
{
    uint_fast32_t i;
    for (i=0; i<len; i+=bytewidth) {
        altera_write_bigendian(f, addr, bytewidth, buf+i);
        addr++;
    }

    i = i/bytewidth;
    while (i<blocklen) {
        altera_write_bigendian(f, addr, bytewidth, zero);
        addr++;
        i++;
    }
    fprintf(f, ":00000001FF\r\n");
}


// Modelsim uses the Verilog hex format which is rarely documented
// Source: IEEE Verilog Standard chapter 17.2.9 "Loading memory data from a
//         file" ($readmemb system task)
// @addr word word word
// All numbers in hexadecimal (case ignored) and addr is optional

void veriloghex_write_littleendian(FILE *f, unsigned addr, unsigned bytewidth,
    const unsigned char *buf)
{
    int i;
    long long v=0;
    for (i=0; i<bytewidth; i++)
        v = v | ((long long)buf[i]<<(8*i));
    fprintf(f, "@%u %0*llx\n", addr, 2*bytewidth, v);
}


void veriloghex_write_block(FILE *f, unsigned bytewidth, unsigned long blocklen,
    uint_fast32_t addr, const unsigned char *buf, unsigned long len)
{
    uint_fast32_t i;
    fprintf(f, "// %lu words of %u bits each\n", len, 8*bytewidth);
    for (i=0; i<len; i+=bytewidth) {
        veriloghex_write_littleendian(f, addr, bytewidth, buf+i);
        addr++;
    }

    i = i/bytewidth;
    while (i<blocklen) {
        veriloghex_write_littleendian(f, addr, bytewidth, zero);
        addr++;
        i++;
    }
}











void write_prefix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    switch (output_format) {
        case OF_MAREK_ARRAY:
            marek_array_write_prefix(f, bytewidth, blocklen);
            break;
	case OF_MAREK_VECTOR:
            marek_vector_write_prefix(f, bytewidth, blocklen);
            break;
    }
}

void write_block(FILE *f, unsigned bytewidth, unsigned blocklen,
    uint_fast32_t addr, const unsigned char *buf, uint_fast32_t len)
{
    switch (output_format) {
        case OF_VERILOGHEX:
            veriloghex_write_block(f, bytewidth, blocklen, addr, buf, len);
            break;
        case OF_ALTERA_ROM:
            altera_rom_write_block(f, bytewidth, blocklen, addr, buf, len);
            break;
        case OF_MAREK_ARRAY:
            marek_array_write_block(f, bytewidth, blocklen, addr, buf, len);
            break;
	case OF_MAREK_VECTOR:
            marek_vector_write_block(f, bytewidth, blocklen, addr, buf, len);
            break;
        case OF_ALTERA_HEX:
        default:
            altera_hex_write_block(f, bytewidth, blocklen, addr, buf, len);
    }
}

void write_suffix(FILE *f, unsigned bytewidth, unsigned blocklen)
{
    switch (output_format) {
        case OF_MAREK_ARRAY:
            marek_array_write_suffix(f, bytewidth, blocklen);
            break;
	case OF_MAREK_VECTOR:
            marek_vector_write_suffix(f, bytewidth, blocklen);
            break;
    }
}







void load_elf(const char *filename, unsigned bytewidth, unsigned blocklen)
{
    unsigned long int size;
    uint_fast8_t *buf;
    unsigned bigendian = 0;
    Elf64_Half segno;
    
    buf = read_whole_file(filename, &size);
    if (buf==0)
	fatal("Could not read file");

    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)buf;
    Elf64_Ehdr *elf_header64 = (Elf64_Ehdr *)buf;
    if (elf_header->e_ident[0]!=ELFMAG0 ||
	elf_header->e_ident[1]!=ELFMAG1 ||
	elf_header->e_ident[2]!=ELFMAG2 ||
	elf_header->e_ident[3]!=ELFMAG3)
    {
	fatal("No ELF file");
    }

/*
    switch (elf_header->e_ident[EI_DATA]) {
    case ELFDATA2LSB:
        bigendian = 0;
        break;
    case ELFDATA2MSB:
        bigendian = 1;
        break;
    default:
        fatal("Unknown endianess");
    }
*/

    write_prefix(stdout, bytewidth, blocklen);

    switch (elf_header->e_ident[EI_CLASS]) {
    case ELFCLASS32: {
        for (segno=0; segno<elf_header->e_phnum; segno++)
        {
            Elf32_Phdr *ph = (Elf32_Phdr *)(buf + elf_header->e_phoff 
                + (segno * elf_header->e_phentsize));

            if ((ph->p_type == PT_LOAD) && (ph->p_filesz > 0)) {
                write_block(stdout, bytewidth, blocklen,
                    ph->p_paddr, buf+ph->p_offset, ph->p_filesz);
            }
        }
        break;
    }

    case ELFCLASS64: {
        for (segno=0; segno<elf_header64->e_phnum; segno++)
        {
            Elf64_Phdr *ph = (Elf64_Phdr *)(buf + elf_header64->e_phoff 
                + (segno * elf_header64->e_phentsize));

            if ((ph->p_type == PT_LOAD) && (ph->p_filesz > 0)) {
                write_block(stdout, bytewidth, blocklen,
                    ph->p_paddr, buf+ph->p_offset, ph->p_filesz);
            }
        }
        break;
    }

    default:
        fatal("Usupported word size (%d)", elf_header->e_ident[EI_CLASS]);
    }
    free(buf);

    write_suffix(stdout, bytewidth, blocklen);
}




int main(int argc, char *argv[])
{
    unsigned blocklen=0;
    unsigned i=1;

    if (argc<3)
    {
        printf("Usage: %s  [-a|-v|-r|-M] <filename> <bytes per word> [length in words]\n\n"
            "Read a RISC-V ELF-file and print it to stdout in a hex format.\n"
            " -a Altera hex format (default)\n"
            " -v Verilog hex format for Modelsim\n"
            " -r VHDL architecure that inferes to a ROM block\n"
            "<bytes per word> bytes which are packed into one big endian line.\n"
            "If there are less than <length in words> lines, lines are filled with 0.\n",
            argv[0]);
        return 1;
    }
    if (strcmp(argv[i], "-v")==0) {
        output_format = OF_VERILOGHEX;
        i++;
    } else if (strcmp(argv[i], "-a")==0) {
        output_format = OF_ALTERA_HEX;
        i++;
    } else if (strcmp(argv[i], "-r")==0) {
        output_format = OF_ALTERA_ROM;
        i++;
    } else if (strcmp(argv[i], "-M")==0) {
        output_format = OF_MAREK_ARRAY;
        i++;
    } else if (strcmp(argv[i], "-V")==0) {
	output_format = OF_MAREK_VECTOR;
	i++;
    }
    
    if (argc>=i+3)
        blocklen = atoi(argv[i+2]);

    load_elf(argv[i], atoi(argv[i+1]), blocklen);
}

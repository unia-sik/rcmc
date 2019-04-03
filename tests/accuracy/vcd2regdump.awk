#!/usr/bin/awk -f
# Convert the VCD output of GHDL to the MacSim regdump format
#
# Usage: awk -v drift=3 -v coreid=generate -v width=4 -f vcdregdump.awk 
#
#   drift=  cycle difference between GHDL and MacSim simulation
#   coreid= how to determine the number of the cores
#   width=  width of the quadratic NoC
#
#   core1, core2:   awk -v drift=8 -v coreid=nocunit
#   pnoo:           awk -v drift=3 -v coreid=generate

BEGIN {
    if (coreid != "generate" && coreid != "nocunit") {
        printf "Unknown method to determine core ID" > /dev/stderr
        exit 1
    }
    if (width<1) {
        printf "NoC width not given" > /dev/stderr
        exit 1
    }

    cycle = - drift
    clock = "UNDEF"
    depth = 0
    core = 0            # on a single core there is no nocunit => core=0
}


/^\$scope module [^ ]+ \$end$/ {
    depth++;
    scope[depth] = $3;

    if (coreid == "nocunit") {
        if ($3 ~ /^nocunit/) {
            i=22;
            core=0;
            while (1) {
                digit = substr($0, i, 1);
                i++;
                if (digit=="0") {core=2*core}
                else if (digit=="1") {core=2*core+1}
                else break;
            }
        }
    } else if (coreid == "generate") {
        if ($3 ~ /\([0-9]+\)/) {
            # $scope module ( )
            i = substr(substr($3, 1, length($3)-1), 2);
            if (depth==1) {
                core_y = i
                core_x = 0;
                core = -1
            } else if (depth==2) {
                core = core_y*width + i;
            }
            #printf "module (%d) depth %d => core %d\n", i, depth, core
        }
    }
}


$0=="$upscope $end" {
    depth--
}


/^\$var reg 1 [^ ]+ clk \$end$/ {
    if (scope[depth]=="clock") {
        clock = "1" $4
    }
}


/^\$var reg 64 [^ ]+ reg[0-9]+\[63:0\] \$end$/ {
    if (scope[depth]=="reg1") {
        regno = gensub(/reg([0-9]+)\[63:0\]/, "\\1", "1", $5)
        reg[core][regno] = $4;
        hash[$4] = "°" core " x" regno
    }
}


/^\$var reg 64 [^ ]+ freg[0-9]+\[63:0\] \$end$/ {
    if (scope[depth]=="freg1") {
        regno = gensub(/freg([0-9]+)\[63:0\]/, "\\1", "1", $5)
        reg[core][regno] = $4;
        hash[$4] = "°" core " f" regno
    }
}


$0==clock {
    if (cycle >= 0) { print "\n#" cycle }
    cycle++;
}

#$0=="$enddefinitions $end" {
#    for (c=0; c<16; c++) {
#        for (r=0; r<32; r++) {
#            print "°" c " x" r " [" reg[c][r] "]";
#        }
#    }
#}

/^b/ {
    core_and_reg = hash[$2]
    if (core_and_reg != "" && cycle>=0) {
        
        # split into two numbers, because large numbers are displayed as float
        hex1 = 0
        hex2 = 0
        for (i=0; i<32; i++) {
            hex1 = 2*hex1 + substr($1, 2+i, 1)
            hex2 = 2*hex2 + substr($1, 34+i, 1)
        }
        if (hex1==0) {
            printf "%s %x\n", core_and_reg, hex2
        } else {
            printf "%s %x%08x\n", core_and_reg, hex1, hex2
        }
    }
}
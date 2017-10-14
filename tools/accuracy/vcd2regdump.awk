#!/usr/bin/awk -f

BEGIN {
    cycle = -8          # ignore first 8 cycles (don't know why)
    clock = "UNDEF"
    depth = 0
}


/^\$scope module [^ ]+ \$end$/ {
    depth++
    scope[depth] = $3

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
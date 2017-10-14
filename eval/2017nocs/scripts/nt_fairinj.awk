#!/usr/bin/awk -f
# input all_netrace.dat
# x: configurations
# y: satthru for different corner buffer sizes

BEGIN {
    FS = ",";
#    print "config cb2 cb4 cb16 cb32";
}

NR != 1 {
    bm = $1;
    router = $2;
    exec = $3;
    a[bm][router] = exec;
}

END {
  ps[0]="blackS";
  ps[1]="fluidS";
  ps[2]="swapL";
  ps[3]="bodyL";
  ps[4]="dedupM";
  ps[5]="vipsM";

  rs[0]="UUGG00";
  rs[1]="UUGG0R";
  rs[2]="UUGGR0";
  rs[3]="UUGGRR";
#  rs[0]="NNGG00";
#  rs[1]="NNGG0R";
#  rs[2]="NNGGR0";
#  rs[3]="NNGGRR";


  printf "benchmark"
  for (r in rs) {
    printf " %s", rs[r];
  }


  for (p in ps) {
    bm = ps[p];
    norm = a[bm]["UUGG00"]-a[bm]["perfect"];

    printf "\n%s", ps[p]
    for (r in rs) {
      router = rs[r];
      printf " %f", ((a[bm][router]-a[bm]["perfect"])/norm)
    }
  }
  printf "\n"
}


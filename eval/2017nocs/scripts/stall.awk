#!/usr/bin/awk -f
# input all.csv
# x: configurations
# y: satthru for different corner buffer sizes

BEGIN {
    FS = ",";
#    print "config cb2 cb4 cb16 cb32";
}

NR != 1 {
    pattern = $1;
    dim = $2;
    cb = $3;
    router = $4 $5 $6 $7 $8 $9;
    satthru = $10;
    a[pattern][dim][router][cb] = satthru;
    routers[router] = router;
}

END {
  ps[0]="N";
  ps[1]="O";
  ps[2]="R";
  ps[3]="S";
  ps[4]="T";
  ps[5]="U";
  
  plong[0]="neighbor";
  plong[1]="tornado";
  plong[2]="bitrev";
  plong[3]="shuffle";
  plong[4]="transpose";
  plong[5]="uniform";
#  plong[6]="bitcomp"; # B
#  plong[7]="upperleft"; # L
  
  rs[0]="NNSS00";
  rs[1]="NNSG00";
  rs[2]="NNGS00";
  rs[3]="NNGG00";
  rs[4]="BBSS00";
  rs[5]="BBSG00";
  rs[6]="BBGS00";
  rs[7]="BBGG00";


  printf "pattern"
  for (r in rs) {
    printf " %s", rs[r];
  }


  for (p in ps) {
    pattern = ps[p];
    dim=4;
    cb=3;

    printf "\n%s", plong[p]
    for (r in rs) {
      router = rs[r];
      printf " %f", a[pattern][dim][router][cb]
    }
  }
  printf "\n"
}


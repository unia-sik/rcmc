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
    router = toupper($4$5$6$7$8$9);
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
  
  rs[0]="NNCC00";
  rs[1]="NUCC00";
  rs[2]="NBCC00";
  rs[3]="UNCC00";
  rs[4]="UUCC00";
  rs[5]="UBCC00";
  rs[6]="BNCC00";
  rs[7]="BUCC00";
  rs[8]="BBCC00";

  rlong[0]="NNGG00";
  rlong[1]="NUGG00";
  rlong[2]="NBGG00";
  rlong[3]="UNGG00";
  rlong[4]="UUGG00";
  rlong[5]="UBGG00";
  rlong[6]="BNGG00";
  rlong[7]="BUGG00";
  rlong[8]="BBGG00";


  printf "pattern"
  for (r in rs) {
    printf " %s", rlong[r];
  }


  for (p in ps) {
    pattern = ps[p];
    dim=4;
    cb=3;

    NN = a[pattern][dim]["NNGG00"][cb];
    printf "\n%s", plong[p]
    for (r in rlong) {
      router = rlong[r];
      printf " %f", a[pattern][dim][router][cb]
    }
  }
  printf "\n"
}


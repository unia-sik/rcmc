#!/usr/bin/awk -f
# input all.csv
# uniform satthru of BBGG00 for x=network size and y=cornerbuffer size
# x: configurations
# y: satthru for different corner buffer sizes

BEGIN {
    FS = ",";
#    print "config cb2 cb4 cb16 cb32";
}

NR != 1 {
    dim = $1;
    cb = $2;
    router = $3 $4 $5 $6y $7 $8;
    alms = $9;
    if (router=="UUGGRR") {
      a[dim][cb] = alms;
    }
}

END {
  xs[0]="1";
  xs[1]="2";
  xs[2]="3";
  xs[3]="4";
  xs[4]="5";

  xt[1]="cb2";
  xt[2]="cb4";
  xt[3]="cb8";
  xt[4]="cb16";
  xt[5]="cb32";



  printf "dim"
  for (x in xt) {
    printf " %s", xt[x];
  }


  for (y=2; y<8; y++) {
    printf "\n%dx%d", y, y
    for (x in xt) {
      printf " %d", a[y][x]
#      printf " %f", (a[y][x]*y*y)
    }
  }
  printf "\n"
}


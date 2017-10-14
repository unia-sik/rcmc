#!/usr/bin/awk -f
# input all.tmp
# cmpute average router size and remove unnecessary data

BEGIN {
    FS = ",";
}

NR != 1 {
    dim = $1;
    cb = $2;
    if ($5=="c") {ys="G"} else {ys="S"}
    if ($6=="c") {xs="G"} else {xs="S"}
#    router = toupper($3$4 ys xs $7$8);
    router = toupper($3 "," $4 "," ys "," xs "," $7 "," $8);
    a[dim][router][cb] += $9;
    c[dim][router][cb] ++;
    routers[router] = router;
}

END {
  ps[0]="N";
  ps[1]="O";
  ps[2]="R";
  ps[3]="S";
  ps[4]="T";
  ps[5]="U";
  for (dim=2; dim<=8; dim++) {
    for (cb=1; cb<=5; cb++) {
      max = 0;
      for (router in routers) {
        if (c[dim][router][cb]!=0) {
#          print dim, cb, router, int(a[dim][router][cb]/c[dim][router][cb])
          print dim "," cb "," router "," int(a[dim][router][cb]/c[dim][router][cb])
        }
      }
    }
  }
}


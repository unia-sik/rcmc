#!/usr/bin/awk -f
# input patterns.csv
# size of routers with 4x4 nodes and 8 corner buffer entries

BEGIN {
    FS = ",";
    print "config alms";
}

NR != 1 {
    dim = $1;
    logcb = $2;
    router = $3 $4 $5 $6y $7 $8;
    alms = $9;
    if (dim==4 && logcb==3) {
      a[router] = alms;
    }
}

END {
  r[0]="NN";
  r[1]="NU";
  r[2]="NB";
  r[3]="UN";
  r[4]="UU";
  r[5]="UB";
  r[6]="BN";
  r[7]="BU";
  r[8]="BB";

  for (i=0; i<9; i++) {
    router=r[i] "GG00";
    print router, a[router];
  }
}


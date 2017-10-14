#!/usr/bin/awk -f
# input all.csv


BEGIN {
    FS = ",";
}

NR != 1 {
  dim = $1;
  cb = $2;
  if (dim==4 && cb==3) {
    router = $3$4$5$6$7$8;
    a[router] = $9;
  }
}

END {
  ps[0]="NNGG";
  ps[1]="BBGG";
  ps[2]="NNGS";
  ps[3]="BBGS";
  ps[4]="NNSG";
  ps[5]="BBSG";
  ps[6]="NNSS";
  ps[7]="BBSS";
  
  plong[0]="NNGG..";
  plong[1]="BBGG..";
  plong[2]="NNGS..";
  plong[3]="BBGS..";
  plong[4]="NNSG..";
  plong[5]="BBSG..";
  plong[6]="NNSS..";
  plong[7]="BBSS..";
  
  rs[0]="00";
  rs[1]="0R";
  rs[2]="R0";
  rs[3]="RR";

  rlong[0]="....00";
  rlong[1]="....0R";
  rlong[2]="....R0";
  rlong[3]="....RR";


  printf "base"
  for (r in rs) {
    printf " %s", rlong[r];
  }


  for (p in ps) {
    printf "\n%s", plong[p]
    for (r in rs) {
      router = ps[p] rs[r]
      printf " %d", a[router]
    }
  }
  printf "\n"
}


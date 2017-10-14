#!/usr/bin/awk -f
# input all_netrace.csv
# reduce all_netrace.csv data by picking special routers an normalising to perfect NoC

BEGIN {
    FS = ",";
#    print "config cb2 cb4 cb16 cb32";
}

NR != 1 {
  bm = $1;
  router = $2;
  cycles = $3;
  benchmarks[bm] = bm;
  a[bm][router] = cycles;
}

END {
  xt[0]="MinBD";
  xt[1]="NNGG00"; # minimale Hardware
  xt[2]="BBSS0R"; # PaterNoster 1.0
  xt[3]="UBSSRR"; # CAERUS
  xt[4]="UUGG00"; # ohne fair
  xt[5]="UUSGRR"; # bester mit fair injection
#  xt[6]="BBGGRR"; # buffere mit fair injection

  printf "benchmark MinBD NNGG00 PaterNoster Caerus UUGG00 UUGGRR"

  for (b in benchmarks) {
    printf "\n%s", b;
    perfect = a[b]["perfect"];
    minbd = a[b]["MinBD"]-perfect;
    for (r in xt) {
      router=xt[r];
      printf " %.9f", (100*(a[b][router]-perfect)/minbd)
    }
  }
  printf "\n"
}


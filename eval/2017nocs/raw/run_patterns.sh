#!/bin/bash
# Run synthetic traffic patterns on MacSim for all router configurations

MACSIM=../../../macsim/build/macsim-seq
OUT=patterns.csv

#declare pattern=(U T R S O N)
#declare dims=(2x2 3x3 4x4 5x5 6x6 7x7)
#declare cbsize=(2cb 4cb 8cb 16cb 32cb)
declare -A bypassx=([N]=CONF_BYPASS_NONE [U]=CONF_BYPASS_UNBUF [B]=CONF_BYPASS_BUF)
declare -A bypassy=([N]=CONF_BYPASS_NONE [U]=CONF_BYPASS_UNBUF [B]=CONF_BYPASS_BUF)
declare -A stallx=([G]=CONF_STALL_CHEAP [S]=CONF_STALL_EXP)
declare -A stally=([G]=CONF_STALL_CHEAP [S]=CONF_STALL_EXP)
declare -A injectx=([0]=CONF_INJECT_NONE [R]=CONF_INJECT_REQUEST) 
declare -A injecty=([0]=CONF_INJECT_NONE [R]=CONF_INJECT_REQUEST)

echo "pattern,width,cb,ybypass,xbypass,ystall,xstall,yinj,xinj,satthrough" > $OUT

for p in U T R S O N ; do
  for WIDTH in 2 3 4 5 6 7 ; do
    for LOGCB in 1 2 3 4 5 ; do

      for i in "${!bypassx[@]}"; do
        for j in "${!bypassy[@]}"; do
          for k in "${!stallx[@]}"; do
            for l in "${!stally[@]}"; do
              for m in "${!injectx[@]}"; do
                for n in "${!injecty[@]}"; do

                  CB=`echo "2 ^ $LOGCB" | bc`
                  $MACSIM -Atraffic$p -N${WIDTH}x${WIDTH} -RP$i$j$k$l$m$n -J$CB -c0 -m run_patterns.tmp -g -q

                  SAT=`grep 'saturation' run_patterns.tmp | cut -d' ' -f3`
                  echo "$p,$WIDTH,$LOGCB,$i,$j,$k,$l,$m,$n,$SAT" >> $OUT

                done
              done
            done
          done
        done
      done
    done
  done
done

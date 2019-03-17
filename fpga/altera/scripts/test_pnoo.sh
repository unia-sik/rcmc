#!/bin/bash

CONFIG="../GHDL_PNOO/src/constants.vhd"

function replaceNatural {
    current=$(egrep "constant $1\ *:" $CONFIG | grep -o "[[:digit:]]*")    
    sed -i "s/$1 *: *natural := $current/$1 : natural := $2/" $CONFIG
}


function replaceVersion {
    current=$(egrep -o "CONF_PNOO(_|[[:alnum:]])*;" $CONFIG)   
    sed -i "s/$current/$1;/" $CONFIG
}

mkdir -p logs
mkdir -p summary

#for core_num in 2 4 8
#do    
#    core_real=$core_num
##     for core_real in 8
##     do        
#        for version in CONF_PNOO_DOUBLE_CORNER_BUFFER #CONF_PNOO_SORT CONF_PNOO CONF_PNOO_CG CONF_PNOO_DOUBLE_CORNER_BUFFER #CONF_PNOO_DRR CONF_PNOO_EMPTY CONF_PNOO_NO_RDY CONF_PNOO_SRR         #CONF_PNOO_SORT
#        do
#            run="PNOO_${version}_${core_num}x${core_num}_${core_real}"
#            echo "##########################################################"
#            echo "# $run"
#            echo "##########################################################"
#            replaceNatural "Dimension" $core_num
#            replaceNatural "dimNoJoke" $core_real
#            replaceVersion $version
#
#            ./compile.sh "PNOO"
#            
#            fullLog="$(date "+%d_%m_%Y_%H_%M_%S")/"
#            cp -r output_files logs/$fullLog
#            cp output_files/PNOO.fit.summary summary/$run.summary
#            echo "Full log: $fullLog" >> summary/$run.summary
#        done
##     done
#done

for core_num in 32
do    
    for core_real in 8
    do        
        for version in CONF_PNOO_SORT #CONF_PNOO_DOUBLE_CORNER_BUFFER CONF_PNOO_SORT CONF_PNOO CONF_PNOO_CG #CONF_PNOO_SORT #CONF_PNOO CONF_PNOO_DOUBLE_CORNER_BUFFER CONF_PNOO_DRR CONF_PNOO_CG  CONF_PNOO_EMPTY CONF_PNOO_NO_RDY CONF_PNOO_SRR         #CONF_PNOO_SORT
        do
            run="PNOO_${version}_${core_num}x${core_num}_${core_real}"
            echo "##########################################################"
            echo "# $run"
            echo "##########################################################"
            replaceNatural "Dimension" $core_num
            replaceNatural "dimNoJoke" $core_real
            replaceVersion $version

            ./compile.sh "PNOO"
            
            fullLog="$(date "+%d_%m_%Y_%H_%M_%S")/"
            cp -r output_files logs/$fullLog
            cp output_files/PNOO.fit.summary summary/$run.summary
            echo "Full log: $fullLog" >> summary/$run.summary
        done
    done
done

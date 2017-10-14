#!/bin/sh
# Download the netrace trace files

mkdir -p traces
cd traces

URL=http://www.cs.utexas.edu/~netrace/download

wget ${URL}/blackscholes_64c_simlarge.tra.bz2 \
  ${URL}/blackscholes_64c_simmedium.tra.bz2 \
  ${URL}/blackscholes_64c_simsmall.tra.bz2 \
  ${URL}/bodytrack_64c_simlarge.tra.bz2 \
  ${URL}/canneal_64c_simmedium.tra.bz2 \
  ${URL}/dedup_64c_simmedium.tra.bz2 \
  ${URL}/ferret_64c_simmedium.tra.bz2 \
  ${URL}/fluidanimate_64c_simlarge.tra.bz2 \
  ${URL}/fluidanimate_64c_simmedium.tra.bz2 \
  ${URL}/fluidanimate_64c_simsmall.tra.bz2 \
  ${URL}/swaptions_64c_simlarge.tra.bz2 \
  ${URL}/vips_64c_simmedium.tra.bz2 \
  ${URL}/x264_64c_simmedium.tra.bz2 \
  ${URL}/x264_64c_simsmall.tra.bz2


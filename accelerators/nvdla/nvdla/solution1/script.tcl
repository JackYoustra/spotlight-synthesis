############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project nvdla
set_top conv_2d
add_files nvdla/systolic.cpp -cflags "-std=c++0x"
add_files nvdla/systolic.h -cflags "-std=c++0x"
add_files -tb nvdla/nvdla_test.cpp -cflags "-std=c++0x"
open_solution "solution1"
set_part {xc7z010iclg225-1L}
create_clock -period 10 -name default
#source "./nvdla/solution1/directives.tcl"
csim_design -O
csynth_design
cosim_design -O -trace_level all
export_design -format ip_catalog

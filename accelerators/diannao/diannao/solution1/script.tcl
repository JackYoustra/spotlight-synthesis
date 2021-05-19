############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project diannao
set_top conv_2d
add_files diannao/diannao.cpp -cflags "-std=c++0x"
add_files diannao/systolic.h -cflags "-std=c++0x"
add_files -tb diannao/systolic_test.cpp -cflags "-std=c++0x"
open_solution "solution1"
set_part {xc7z010i-clg225-1L}
create_clock -period 10 -name default
config_export -format ip_catalog -rtl verilog
#source "./diannao/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -flow syn -rtl verilog -format ip_catalog

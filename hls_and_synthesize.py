#!/usr/bin/env python
import multiprocessing as mp
import subprocess as sp
import tempfile
import argparse as ap
from pathlib import Path
import os
import time

start = time.process_time()

scratch_solution_root = Path('synthesized_projects/').resolve()
assert (scratch_solution_root.exists())

hls_solution_root = scratch_solution_root / "hls"
assert (hls_solution_root.exists())
vivado_solution_root = scratch_solution_root / "vivado"
assert (vivado_solution_root.exists())

accelerator_locations = Path('./accelerators')
assert (accelerator_locations.exists())

parser = ap.ArgumentParser("Runs vivado test script")
parser.add_argument('-a', '--accelerator', type=str, required=True)
parser.add_argument('-s', '--signal', type=int, required=True)
parser.add_argument('-f', '--filter', type=int, required=True)
parser.add_argument('-i', '--input-channels', type=int, required=True)
parser.add_argument('-o', '--output-channels', type=int, required=True)
parser.add_argument('-c', '--impl-file', type=str, required=True)

args = parser.parse_args()
# argument 1 has hls commands, argument 2 has vivado commands
accelerator = args.accelerator
filt = args.filter
channelIn = args.input_channels
channelOut = args.output_channels
signal = args.signal

# location of all the source files
accelerator_folder = accelerator_locations / "{}/{}".format(accelerator, accelerator)
assert (accelerator_folder.exists())

part = "{xc7z010iclg225-1L}"
flags = "-std=c++0x -DSIGNAL_SIZE={} -DFILTER_SIZE={} -DCHANNEL_IN_COUNT={} -DCHANNEL_OUT_COUNT={}".format(signal, filt, channelIn, channelOut)
solution = "{}_s{}_f{}_ci{}_co{}".format(accelerator, signal, filt, channelIn, channelOut)

hls_solution_dir = hls_solution_root / solution
# don't run this step if the folder doesn't exist
if not hls_solution_dir.exists():
    hls_solution_dir.mkdir()
    # vivado_hls automatically has a project folder
    os.chdir(hls_solution_root)

    hlsString = """open_project {}
set_top conv_2d
add_files {}/{}.cpp -cflags \"{}\"
add_files {}/systolic.h -cflags \"{}\"
add_files -tb {}/systolic_test.cpp -cflags \"{}\"
open_solution "solution1"
set_part {}
create_clock -period 10 -name default
csynth_design
export_design -format ip_catalog
exit
""".format(solution, accelerator_folder, args.impl_file, flags, accelerator_folder, flags, accelerator_folder, flags, part)

    # print(hlsString)
    # capturing output leads to no passthrough. If doesn't work, don't continue, just fail
    with tempfile.NamedTemporaryFile(mode='w') as fp:
        fp.write(hlsString)
        fp.flush()
        sp.run(f"vivado_hls -f {fp.name}", shell=True, check=True)

# Vivado step
os.chdir(vivado_solution_root)
# Not going to do the copy solution, going to instead try and synthesize the string from scratch
# -  copy from reference vivado project (not HLS)
# : solution_project_path = vivado_solution_root / solution
# : shutil.copytree(vivado_reference_path, solution_project_path)

projectname = solution
project_dir = vivado_solution_root / projectname
if not project_dir.exists():
    # not sure, check
    ip_dir = hls_solution_dir / "solution1/impl/ip"
    assert(ip_dir.exists())
    # still faster than -1, no io bound so keep at this height
    jobs = mp.cpu_count()

    # yay fstrings
    vivadoString = f"""create_project project_1 {project_dir} -part xc7z020clg484-1
set_property board_part xilinx.com:zc702:part0:1.4 [current_project]
set_property  ip_repo_paths  {ip_dir} [current_project]
update_ip_catalog
create_bd_design "Zynq_Design"
update_compile_order -fileset sources_1
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
endgroup
startgroup
set_property -dict [list CONFIG.preset {{ZC702}}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_USE_FABRIC_INTERRUPT {{1}} CONFIG.PCW_IRQ_F2P_INTR {{1}} CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {{1}} CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {{0}}] [get_bd_cells processing_system7_0]
endgroup
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {{make_external "FIXED_IO, DDR" apply_board_preset "0" Master "Disable" Slave "Disable" }}  [get_bd_cells processing_system7_0]
startgroup
create_bd_cell -type ip -vlnv xilinx.com:hls:conv_2d:1.0 conv_2d_0
endgroup
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {{ Clk_master {{Auto}} Clk_slave {{Auto}} Clk_xbar {{Auto}} Master {{/processing_system7_0/M_AXI_GP0}} Slave {{/conv_2d_0/s_axi_HLS_DATABUS}} ddr_seg {{Auto}} intc_ip {{New AXI Interconnect}} master_apm {{0}}}}  [get_bd_intf_pins conv_2d_0/s_axi_HLS_DATABUS]
connect_bd_net [get_bd_pins conv_2d_0/interrupt] [get_bd_pins processing_system7_0/IRQ_F2P]
validate_bd_design
save_bd_design
generate_target all [get_files  {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/Zynq_Design.bd]
catch {{ config_ip_cache -export [get_ips -all Zynq_Design_processing_system7_0_0] }}
catch {{ config_ip_cache -export [get_ips -all Zynq_Design_conv_2d_0_0] }}
catch {{ config_ip_cache -export [get_ips -all Zynq_Design_auto_pc_0] }}
catch {{ config_ip_cache -export [get_ips -all Zynq_Design_rst_ps7_0_50M_0] }}
export_ip_user_files -of_objects [get_files {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/Zynq_Design.bd] -no_script -sync -force -quiet
create_ip_run [get_files -of_objects [get_fileset sources_1] {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/Zynq_Design.bd]
launch_runs Zynq_Design_processing_system7_0_0_synth_1 Zynq_Design_conv_2d_0_0_synth_1 Zynq_Design_auto_pc_0_synth_1 Zynq_Design_rst_ps7_0_50M_0_synth_1 -jobs {jobs}
wait_on_run Zynq_Design_processing_system7_0_0_synth_1
wait_on_run Zynq_Design_conv_2d_0_0_synth_1
wait_on_run Zynq_Design_auto_pc_0_synth_1
wait_on_run Zynq_Design_rst_ps7_0_50M_0_synth_1
export_simulation -of_objects [get_files {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/Zynq_Design.bd] -directory {project_dir}/project_1.ip_user_files/sim_scripts -ip_user_files_dir {project_dir}/project_1.ip_user_files -ipstatic_source_dir {project_dir}/project_1.ip_user_files/ipstatic -lib_map_path [list {{modelsim={project_dir}/project_1.cache/compile_simlib/modelsim}} {{questa={project_dir}/project_1.cache/compile_simlib/questa}} {{ies={project_dir}/project_1.cache/compile_simlib/ies}} {{xcelium={project_dir}/project_1.cache/compile_simlib/xcelium}} {{vcs={project_dir}/project_1.cache/compile_simlib/vcs}} {{riviera={project_dir}/project_1.cache/compile_simlib/riviera}}] -use_ip_compiled_libs -force -quiet
make_wrapper -files [get_files {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/Zynq_Design.bd] -top
add_files -norecurse {project_dir}/project_1.srcs/sources_1/bd/Zynq_Design/hdl/Zynq_Design_wrapper.v
launch_runs impl_1 -jobs {jobs}
wait_on_run impl_1
open_run impl_1
report_timing_summary -delay_type min_max -report_unconstrained -check_timing_verbose -max_paths 10 -input_pins -routable_nets -name timing_1
report_power -name {{power_1}}
exit
""".replace("project_1", projectname)
    # print(vivadoString)

    # Save vivado command to file
    with tempfile.NamedTemporaryFile(mode='w') as fp:
        fp.write(vivadoString)
        fp.flush()
        sp.run(f"vivado -mode batch -source {fp.name}", shell=True)
# Integrate SAIF reports - rerun vivado HLS with saif for any board that succeeded with place and route, and run power report

print(f"All done. Total execution time: {time.process_time() - start} seconds")
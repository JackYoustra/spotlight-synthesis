<project xmlns="com.autoesl.autopilot.project" top="conv_2d" name="diannao">
    <includePaths/>
    <libraryPaths/>
    <Simulation>
        <SimFlow name="csim" csimMode="0" lastCsimMode="0"/>
    </Simulation>
    <files xmlns="">
        <file name="../systolic_test.cpp" sc="0" tb="1" cflags=" -std=c++0x -DSIGNAL_SIZE=64 -DFILTER_SIZE=32 -DCHANNEL_IN_COUNT=1 -DCHANNEL_OUT_COUNT=1 -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="diannao/systolic.h" sc="0" tb="false" cflags="-std=c++0x -DSIGNAL_SIZE=64 -DFILTER_SIZE=32 -DCHANNEL_IN_COUNT=1 -DCHANNEL_OUT_COUNT=1" csimflags="" blackbox="false"/>
        <file name="diannao/diannao.cpp" sc="0" tb="false" cflags="-std=c++0x -DSIGNAL_SIZE=64 -DFILTER_SIZE=32 -DCHANNEL_IN_COUNT=1 -DCHANNEL_OUT_COUNT=1" csimflags="" blackbox="false"/>
    </files>
    <solutions xmlns="">
        <solution name="solution1" status="active"/>
        <solution name="solution_s2_f1_ci1_co1" status=""/>
        <solution name="solution_s2_f2_ci1_co1" status=""/>
        <solution name="solution_s4_f1_ci1_co1" status=""/>
        <solution name="solution_s4_f2_ci1_co1" status=""/>
        <solution name="solution_s4_f4_ci1_co1" status=""/>
        <solution name="solution_s8_f1_ci1_co1" status=""/>
        <solution name="solution_s8_f2_ci1_co1" status=""/>
        <solution name="solution_s8_f6_ci1_co1" status=""/>
        <solution name="solution_s8_f4_ci1_co1" status=""/>
        <solution name="solution_s8_f8_ci1_co1" status=""/>
        <solution name="solution_s16_f2_ci1_co1" status=""/>
        <solution name="solution_s16_f4_ci1_co1" status=""/>
        <solution name="solution_s16_f6_ci1_co1" status=""/>
        <solution name="solution_s16_f1_ci1_co1" status=""/>
        <solution name="solution_s16_f8_ci1_co1" status=""/>
        <solution name="solution_s16_f16_ci1_co1" status=""/>
        <solution name="solution_s32_f1_ci1_co1" status=""/>
        <solution name="solution_s32_f4_ci1_co1" status=""/>
        <solution name="solution_s32_f2_ci1_co1" status=""/>
        <solution name="solution_s32_f6_ci1_co1" status=""/>
        <solution name="solution_s32_f8_ci1_co1" status=""/>
        <solution name="solution_s32_f16_ci1_co1" status=""/>
        <solution name="solution_s64_f2_ci1_co1" status=""/>
        <solution name="solution_s32_f32_ci1_co1" status=""/>
        <solution name="solution_s64_f1_ci1_co1" status=""/>
        <solution name="solution_s64_f4_ci1_co1" status=""/>
        <solution name="solution_s64_f6_ci1_co1" status=""/>
        <solution name="solution_s64_f8_ci1_co1" status=""/>
        <solution name="solution_s64_f16_ci1_co1" status=""/>
        <solution name="solution_s64_f32_ci1_co1" status=""/>
    </solutions>
</project>


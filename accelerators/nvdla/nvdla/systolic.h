#ifndef _SYSTOLIC_H
#define _SYSTOLIC_H

#include "hls_stream.h"

#ifndef CHANNEL_IN_COUNT
constexpr auto CHANNEL_IN_COUNT = 3;
#endif

#ifndef CHANNEL_OUT_COUNT
constexpr auto CHANNEL_OUT_COUNT = 7;
#endif

#ifndef SIGNAL_SIZE
constexpr auto SIGNAL_SIZE = 28;
#endif

#ifndef FILTER_SIZE
constexpr auto FILTER_SIZE = 7;
#endif

constexpr auto SIGNAL_WIDTH = SIGNAL_SIZE;
constexpr auto FILTER_WIDTH = FILTER_SIZE;
constexpr auto OUT_WIDTH = SIGNAL_WIDTH - FILTER_WIDTH + 1;

constexpr auto SIGNAL_HEIGHT = SIGNAL_SIZE;
constexpr auto FILTER_HEIGHT = FILTER_SIZE;
constexpr auto OUT_HEIGHT = SIGNAL_HEIGHT - FILTER_HEIGHT + 1;

static_assert(OUT_WIDTH > 0, "Output width should be greater than zero");
static_assert(OUT_HEIGHT > 0, "Output height should be greater than zero");


typedef short data_t;

void conv_2d(data_t signal[CHANNEL_IN_COUNT][SIGNAL_HEIGHT][SIGNAL_WIDTH],
		data_t kernel[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][FILTER_HEIGHT][FILTER_WIDTH],
		data_t out[CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH]);

#endif

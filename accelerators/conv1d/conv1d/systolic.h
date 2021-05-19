#ifndef _SYSTOLIC_H
#define _SYSTOLIC_H

#include "hls_stream.h"

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

void conv_2d(data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH], data_t kernel[FILTER_HEIGHT][FILTER_WIDTH], data_t out[OUT_HEIGHT][OUT_WIDTH]);

#endif

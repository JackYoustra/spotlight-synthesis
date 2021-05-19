#include "systolic.h"
#include <array>
#include <cmath>
#include <iostream>

int div_ceil(int numerator, int denominator)
{
	std::div_t res = std::div(numerator, denominator);
	return res.rem ? (res.quot + 1) : res.quot;
}


// {signal_width (and height), filter_width (and height), in channels, out channels}
// https://pytorch.org/hub/pytorch_vision_resnet/
constexpr std::array<std::array<int, 4>, 3> RESNET_DIMS = {{
	{256, 2, 64, 64},
	{28, 1, 128, 512},
	{7, 3, 512, 512},
}};

void full_conv() {
	int call_total = 0;
	for (const auto& desc : RESNET_DIMS) {
		const auto signal_width = desc[0];
		const auto filter_width = desc[1];
		const auto out_width = signal_width - filter_width + 1;
		const auto channel_in_count = desc[2];
		const auto channel_out_count = desc[3];
		for (int out_row = 0; out_row < out_width;) {
			for (int out_col = 0; out_row < out_width;) {
				// For each tile, we have to get a multiple of each subtile once and only once, padding allowed
				// That's just ceil of min number of tiles necessary to fit (it would take 4 5x5 filter runs to fit one 10x10 filter run, where 11x11 requires 9)
				// Equation for this (x is accelerator dim, z is desired dim, y is iteration) y = ceil(z/x)^2
				// Same goes for signal, but this time just build over different parts of the output
				// Then, just aggregate over out (not modeled
				// So, total number of iterations of conv_2d needed for one resnet layer is ceil(z_signal/x_signal)^2 * ceil(z_filter/x_filter)^2
//					for (int signal_row = 0; signal_row < signal_width;) {
//						for (int signal_col = 0; signal_col < signal_width;) {
//							for (int filter_row = 0; filter_row < filter_width;) {
//								for (int filter_col = 0; filter_col < filter_width;) {
//
//								}
//							}
//						}
//					}

				const int number_of_calls = std::pow(div_ceil(signal_width, SIGNAL_HEIGHT), 2) * std::pow(div_ceil(filter_width, FILTER_HEIGHT), 2);
				call_total += number_of_calls;
			}
		}
	}

	for (int i = 0; i < call_total; i++) {
		data_t out[CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH];
		data_t signal[CHANNEL_IN_COUNT][SIGNAL_HEIGHT][SIGNAL_WIDTH];
		data_t filter[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][FILTER_HEIGHT][FILTER_WIDTH];
		conv_2d(signal, filter, out);
	}
}

int main(int argc, char ** argv) {
	full_conv();
//	correctness();
    return 0;
}

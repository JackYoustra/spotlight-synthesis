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
	for (const auto& desc : RESNET_DIMS) {
		const auto signal_width = desc[0];
		const auto filter_width = desc[1];
		const auto out_width = signal_width - filter_width + 1;
		const auto channel_in_count = desc[2];
		const auto channel_out_count = desc[3];
		for (int i = 0; i < channel_in_count; i++) {
			for (int j = 0; j < channel_out_count; j++) {
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
						data_t out[OUT_HEIGHT][OUT_WIDTH];
						data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH];
						data_t filter[FILTER_HEIGHT][FILTER_WIDTH];
						const int number_of_calls = std::pow(div_ceil(signal_width, SIGNAL_HEIGHT), 2) * std::pow(div_ceil(filter_width, FILTER_HEIGHT), 2);
						for (int c = 0; c < number_of_calls; c++) {
							conv_2d(signal, filter, out);
						}
					}
				}
			}

		}
	}
}

void correctness() {
	data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH];
	data_t filter[FILTER_HEIGHT][FILTER_WIDTH];
	data_t out_sw[OUT_HEIGHT][OUT_WIDTH];
	data_t out_hw[OUT_HEIGHT][OUT_WIDTH];
	for (int i = 0; i < OUT_HEIGHT; i++) {
		for (int j = 0; j < OUT_WIDTH; j++) {
			out_sw[i][j] = rand();
			out_hw[i][j] = rand();
		}
	}
	data_t elem = 0;
	for (int i = 0; i < SIGNAL_HEIGHT; i++) {
		for (int j = 0; j < SIGNAL_WIDTH; j++) {
			if (i == 0 || true) {
				signal[i][j] = elem++;
			} else {
				signal[i][j] = 0;
			}
		}
	}
	elem = 0;
	for (int i = 0; i < FILTER_HEIGHT; i++) {
		for (int j = 0; j < FILTER_WIDTH; j++) {
			if (i == 0 || true) {
				filter[i][j] = elem++;
			} else {
				filter[i][j] = 0;
			}
		}
	}
	for (int col = 0; col < OUT_HEIGHT; ++col) {
		for (int row = 0; row < OUT_WIDTH; ++row) {
			data_t accumulator = 0;
			for (int filter_row = 0; filter_row < FILTER_HEIGHT; filter_row++) {
				for (int filter_col = 0; filter_col < FILTER_WIDTH;
						filter_col++) {
					const auto signal_row = row + filter_row;
					const auto signal_col = col + filter_col;
					accumulator += signal[signal_col][signal_row]
							* filter[filter_col][filter_row];
				}
			}
			out_sw[col][row] = accumulator;
		}
	}
	//conv_2d(&signal[0][0], &filter[0][0], &out_hw[0][0]);
	conv_2d(signal, filter, out_hw);
	bool equal = true;
	for (int i = 0; i < OUT_WIDTH && equal; i++) {
		for (int j = 0; j < OUT_HEIGHT && equal; j++) {
			if (out_sw[j][i] != out_hw[j][i]) {
				equal = false;
			}
		}
	}
	if (!equal || true) {
		std::cout << "(in) SW\n";
		for (int i = 0; i < SIGNAL_HEIGHT; i++) {
			for (int j = 0; j < SIGNAL_WIDTH; j++) {
				std::cout << signal[i][j] << ",";
			}
			std::cout << std::endl;
		}
		std::cout << "(in) Filter\n";
		for (int i = 0; i < FILTER_HEIGHT; i++) {
			for (int j = 0; j < FILTER_WIDTH; j++) {
				std::cout << filter[i][j] << ",";
			}
			std::cout << std::endl;
		}
		std::cout << "(out) SW                         HW\n";
		for (int i = 0; i < OUT_HEIGHT; i++) {
			for (int j = 0; j < OUT_WIDTH; j++) {
				std::cout << out_sw[i][j] << ",";
			}
			std::cout << "      ";
			for (int j = 0; j < OUT_WIDTH; j++) {
				std::cout << out_hw[i][j] << ",";
			}
			std::cout << std::endl;
		}
		//        return 1;
	}
}

int main(int argc, char ** argv) {
	full_conv();
//	correctness();
    return 0;
}

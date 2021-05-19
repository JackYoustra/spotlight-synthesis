#include "hls_stream.h"
#include "systolic.h"

typedef hls::stream<data_t> data_stream_t;

void fill(data_t signal[CHANNEL_IN_COUNT][SIGNAL_HEIGHT][SIGNAL_WIDTH],
		data_stream_t input_map[CHANNEL_OUT_COUNT][CHANNEL_IN_COUNT],
		data_t kernel[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][FILTER_HEIGHT][FILTER_WIDTH],
		data_stream_t kernel_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT]) {
	//	for (int out_channel = 0; out_channel < CHANNEL_OUT_COUNT; out_channel++) {
	//	#pragma HLS UNROLL
	//			for (int in_channel = 0; in_channel < CHANNEL_IN_COUNT; in_channel++) {
	//	#pragma HLS UNROLL
	//				filter_map[out_channel][in_channel] <<
	//			}
	//	}
	for (int in_channel = 0; in_channel < CHANNEL_IN_COUNT; in_channel++) {
#pragma HLS UNROLL
		for (int row = 0; row < SIGNAL_HEIGHT; row++) {
#pragma HLS UNROLL
			for (int col = 0; col < SIGNAL_WIDTH; col++) {
#pragma HLS UNROLL
				data_t element;
				element = signal[in_channel][row][col];
				for (int out_channel = 0; out_channel < CHANNEL_OUT_COUNT; out_channel++) {
#pragma HLS UNROLL
					input_map[out_channel][in_channel] << element;
				}
			}
		}
	}

	for (int in_channel = 0; in_channel < CHANNEL_IN_COUNT; in_channel++) {
#pragma HLS UNROLL
		for (int out_channel = 0; out_channel < CHANNEL_OUT_COUNT; out_channel++) {
#pragma HLS UNROLL
			for (int row = 0; row < SIGNAL_HEIGHT; row++) {
	#pragma HLS UNROLL
				for (int col = 0; col < SIGNAL_WIDTH; col++) {
#pragma HLS UNROLL
					kernel_map[in_channel][out_channel] << kernel[in_channel][out_channel][row][col];
				}
			}
		}
	}
}

// row-column order
void PE(data_stream_t kernel_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT],
		data_stream_t input_map[CHANNEL_OUT_COUNT][CHANNEL_IN_COUNT],
		data_stream_t output_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH],
		int in_channel, int out_channel) {
	data_t kernel_data[FILTER_HEIGHT][FILTER_WIDTH];
	for (int output_row = 0; output_row < OUT_HEIGHT; output_row++) {
		for (int output_col = 0; output_col < OUT_WIDTH; output_col++) {
			data_t local_accum = 0;
			for (int f_row = 0; f_row < FILTER_HEIGHT; f_row++) {
				for (int f_column = 0; f_column < FILTER_WIDTH; f_column++) {
					if (output_row == 0 && output_col == 0) {
						// Initialize kernel data
						kernel_map[in_channel][out_channel] >> kernel_data[f_row][f_column];
					}
					data_t val;
					input_map[out_channel][in_channel] >> val;
					local_accum += val * kernel_data[f_row][f_column];
				}
			}
			if (in_channel == 0) {
				output_map[in_channel][out_channel][output_row][output_col] << local_accum;
			} else {
				data_t prev;
				output_map[in_channel - 1][out_channel][output_row][output_col] >> prev;
				output_map[in_channel][out_channel][output_row][output_col] << local_accum + prev;
			}
		}
	}
}

void process(
		data_stream_t kernel_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT],
		data_stream_t input_map[CHANNEL_OUT_COUNT][CHANNEL_IN_COUNT],
		data_stream_t output_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH]) {

	for (int out_channel = 0; out_channel < CHANNEL_OUT_COUNT; out_channel++) {
#pragma HLS UNROLL
		for (int in_channel = 0; in_channel < CHANNEL_IN_COUNT; in_channel++) {
#pragma HLS UNROLL
			PE(kernel_map, input_map, output_map, in_channel, out_channel);
		}
	}
}

void drain(data_stream_t output_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH],
		data_t out[CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH]) {
	for (int out_channel = 0; out_channel < CHANNEL_OUT_COUNT; out_channel++) {
#pragma HLS UNROLL
		for (int row = 0; row < OUT_HEIGHT; row++) {
#pragma HLS UNROLL
			for (int col = 0; col < OUT_WIDTH; col++) {
#pragma HLS UNROLL
				output_map[CHANNEL_IN_COUNT - 1][out_channel][row][col] >> out[out_channel][row][col];
			}
		}
	}
}

// From Vivienne's handout
void conv_2d(data_t signal[CHANNEL_IN_COUNT][SIGNAL_HEIGHT][SIGNAL_WIDTH],
		data_t kernel[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][FILTER_HEIGHT][FILTER_WIDTH],
		data_t out[CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH]) {
#pragma HLS INTERFACE s_axilite port=signal bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=kernel bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=out bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=return bundle="HLS_DATABUS"
#pragma HLS DATAFLOW
#pragma HLS inline region

	data_stream_t output_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT][OUT_HEIGHT][OUT_WIDTH];
	// Both of the below: stream in elements, HLS isn't so good at doing non stream queues
	data_stream_t kernel_map[CHANNEL_IN_COUNT][CHANNEL_OUT_COUNT];
	data_stream_t input_map[CHANNEL_OUT_COUNT][CHANNEL_IN_COUNT];


#pragma HLS array_partition variable=output_map complete dim=0
#pragma HLS array_partition variable=kernel_map complete dim=0
#pragma HLS array_partition variable=input_map complete dim=0

	fill(signal, input_map, kernel, kernel_map);

	process(kernel_map, input_map, output_map);

	drain(output_map, out);
	return;
}



























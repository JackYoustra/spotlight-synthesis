#include "hls_stream.h"
#include "systolic.h"

typedef hls::stream<data_t> data_stream_t;

void right_edge_PE(data_stream_t &in_signal, data_stream_t &in_filter, data_stream_t &psum_in,
		data_stream_t &psum_out)
{
	// Load the filter, send the filter
	data_t filter[FILTER_WIDTH];
	for(int i = 0; i < FILTER_WIDTH; i++) {
#pragma HLS unroll
		in_filter >> filter[i];
	}
	data_t signal_buffer[FILTER_WIDTH];

	// Preload the signal buffer with the first filter elements
	for(int i = 0; i < FILTER_WIDTH - 1; i++) {
#pragma HLS unroll
		in_signal >> signal_buffer[i];
	}

	unsigned short circular_current = FILTER_WIDTH - 1;

    for(int i = 0; i < OUT_WIDTH; ++i) {
    	// I think I should get rid of this unroll
//	#pragma HLS unroll
    	data_t accumulator = 0;
		psum_in >> accumulator;
		// Load the current signal in a streaming fashion
    	const auto signal_access_index = i + FILTER_WIDTH - 1;
    	in_signal >> signal_buffer[circular_current];

		for(int filter_idx = 0; filter_idx < FILTER_WIDTH; filter_idx++) {
	#pragma HLS unroll
			data_t filter_value = filter[filter_idx];
			const auto signal_idx = i + filter_idx;
			const auto circular_index = signal_idx % FILTER_WIDTH;
			accumulator += signal_buffer[circular_index] * filter_value;
		}
		circular_current++;
		if (circular_current >= FILTER_WIDTH) {
			circular_current = 0;
		}
		psum_out << accumulator;
    }
}

void top_edge_PE(data_stream_t &in_signal, data_stream_t &in_filter, data_stream_t &psum_in,
		data_stream_t &filter_out, data_stream_t &psum_out)
{
	// Load the filter, send the filter
	data_t filter[FILTER_WIDTH];
	for(int i = 0; i < FILTER_WIDTH; i++) {
#pragma HLS unroll
		in_filter >> filter[i];
		filter_out << filter[i];
	}
	data_t signal_buffer[FILTER_WIDTH];

	// Preload the signal buffer with the first filter elements
	for(int i = 0; i < FILTER_WIDTH - 1; i++) {
#pragma HLS unroll
		in_signal >> signal_buffer[i];
	}

	unsigned short circular_current = FILTER_WIDTH - 1;

    for(int i = 0; i < OUT_WIDTH; ++i) {
    	// I think I should get rid of this unroll
//	#pragma HLS unroll
    	data_t accumulator = 0;
		psum_in >> accumulator;
		// Load the current signal in a streaming fashion
    	const auto signal_access_index = i + FILTER_WIDTH - 1;
    	in_signal >> signal_buffer[circular_current];

		for(int filter_idx = 0; filter_idx < FILTER_WIDTH; filter_idx++) {
	#pragma HLS unroll
			data_t filter_value = filter[filter_idx];
			const auto signal_idx = i + filter_idx;
			const auto circular_index = signal_idx % FILTER_WIDTH;
			accumulator += signal_buffer[circular_index] * filter_value;
		}
		circular_current++;
		if (circular_current >= FILTER_WIDTH) {
			circular_current = 0;
		}
		psum_out << accumulator;
    }
}

void PE(data_stream_t &in_signal, data_stream_t &in_filter, data_stream_t &psum_in,
		data_stream_t &signal_out, data_stream_t &filter_out, data_stream_t &psum_out)
{
	// Load the filter, send the filter
	data_t filter[FILTER_WIDTH];
	for(int i = 0; i < FILTER_WIDTH; i++) {
#pragma HLS unroll
		in_filter >> filter[i];
		filter_out << filter[i];
	}
	data_t signal_buffer[FILTER_WIDTH];

	// Preload the signal buffer with the first filter elements
	for(int i = 0; i < FILTER_WIDTH - 1; i++) {
#pragma HLS unroll
		in_signal >> signal_buffer[i];
		// Whenever we take in a signal, output it to the next signal buffer
		signal_out << signal_buffer[i];
	}

	unsigned short circular_current = FILTER_WIDTH - 1;

    for(int i = 0; i < OUT_WIDTH; ++i) {
    	// I think I should get rid of this unroll
//	#pragma HLS unroll
    	data_t accumulator = 0;
		psum_in >> accumulator;
		// Load the current signal in a streaming fashion
    	const auto signal_access_index = i + FILTER_WIDTH - 1;
    	in_signal >> signal_buffer[circular_current];
		// Whenever we take in a signal, output it to the next signal buffer
    	signal_out << signal_buffer[circular_current];

		for(int filter_idx = 0; filter_idx < FILTER_WIDTH; filter_idx++) {
	#pragma HLS unroll
			data_t filter_value = filter[filter_idx];
			const auto signal_idx = i + filter_idx;
			const auto circular_index = signal_idx % FILTER_WIDTH;
			accumulator += signal_buffer[circular_index] * filter_value;
		}
		circular_current++;
		if (circular_current >= FILTER_WIDTH) {
			circular_current = 0;
		}
		psum_out << accumulator;
    }
}

void drain(data_stream_t psums[FILTER_HEIGHT + 1][OUT_HEIGHT], data_t out[OUT_HEIGHT][OUT_WIDTH]) {
	// All elements will now be stored in the first row of sys
	for(int i = 0; i < OUT_WIDTH; i++) {
#pragma HLS unroll
		for(int j = 0; j < OUT_HEIGHT; j++) {
#pragma HLS unroll
			psums[0][j] >> out[j][i];
		}
	}
}

void load_streams(
		data_stream_t in_signal[FILTER_HEIGHT][OUT_HEIGHT],
		data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH],
		data_stream_t in_filter[FILTER_HEIGHT][OUT_HEIGHT],
		data_t kernel[FILTER_HEIGHT][FILTER_WIDTH],
		data_stream_t psums[FILTER_HEIGHT + 1][OUT_HEIGHT]) {
	// The leading left side of the systolic array
	for (int i = 0; i < FILTER_HEIGHT; i++) {
#pragma HLS unroll
		// Inner loop per-element connection
		for (int elem = 0; elem < SIGNAL_WIDTH; elem++) {
#pragma HLS unroll
			in_signal[i][0] << signal[i][elem];
		}
		for (int elem = 0; elem < FILTER_WIDTH; elem++) {
#pragma HLS unroll
			in_filter[i][0] << kernel[i][elem];
		}
	}
	// Start at one - bottom-left corner handled by above
	for (int i = 1; i < OUT_HEIGHT; i++) {
#pragma HLS unroll
		for (int elem = 0; elem < SIGNAL_WIDTH; elem++) {
#pragma HLS unroll
			in_signal[FILTER_HEIGHT - 1][i]
					<< signal[i + FILTER_HEIGHT - 1][elem];
		}
	}
	// Initialize first row of psums to zero
	for (int elem = 0; elem < OUT_WIDTH; elem++) {
#pragma HLS unroll
		for (int col = 0; col < OUT_HEIGHT; col++) {
#pragma HLS unroll
			psums[FILTER_HEIGHT][col] << 0;
		}
	}
}

void create_pe(data_stream_t in_signal[FILTER_HEIGHT][OUT_HEIGHT],
		data_stream_t in_filter[FILTER_HEIGHT][OUT_HEIGHT],
		data_stream_t psums[FILTER_HEIGHT + 1][OUT_HEIGHT]) {
	// Logical dataflow impl. Start from bottom and go to top so don't have data stalls
	for (int systolic_row = FILTER_HEIGHT - 1; systolic_row >= 0;
			systolic_row--) {
#pragma HLS unroll
		//rows: bottom to top
		for (int systolic_col = 0; systolic_col < OUT_HEIGHT; systolic_col++) {
#pragma HLS unroll
			//columns: left to right
			const auto column_end = (systolic_col + 1) == OUT_HEIGHT;
			const auto row_end = (systolic_row - 1) < 0;
			if (column_end) {
				// Up and to the right, as well as to the right, doesn't exist
				// Neither the in_signal nor in_filter should be used as an output target, but don't know how to express that...
				right_edge_PE(in_signal[systolic_row][systolic_col],
						in_filter[systolic_row][systolic_col],
						psums[systolic_row + 1][systolic_col],
						psums[systolic_row][systolic_col]);
			} else if (row_end) {
				// Up and to the right doesn't exist, but just to the right does
				// The in_signal shoudln't be used
				top_edge_PE(in_signal[systolic_row][systolic_col],
						in_filter[systolic_row][systolic_col],
						psums[systolic_row + 1][systolic_col],
						in_filter[systolic_row][systolic_col + 1],
						psums[systolic_row][systolic_col]);
			} else {
				PE(in_signal[systolic_row][systolic_col],
						in_filter[systolic_row][systolic_col],
						psums[systolic_row + 1][systolic_col], // in
						in_signal[systolic_row - 1][systolic_col + 1],
						in_filter[systolic_row][systolic_col + 1],
						psums[systolic_row][systolic_col]); // out
			}
		}
	}
}

// Reference: https://www.rle.mit.edu/eems/wp-content/uploads/2016/02/eyeriss_isscc_2016.pdf
void conv_2d(data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH], data_t kernel[FILTER_HEIGHT][FILTER_WIDTH], data_t out[OUT_HEIGHT][OUT_WIDTH]) {
#pragma HLS INTERFACE s_axilite port=signal bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=kernel bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=out bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=return bundle="HLS_DATABUS"
#pragma HLS DATAFLOW
#pragma HLS inline region

	// Systolic array dims: Filter_height rows, out_height cols

	// The signal and filter elements going into the array at the index of the PE element systolic array
    data_stream_t in_signal[FILTER_HEIGHT][OUT_HEIGHT], in_filter[FILTER_HEIGHT][OUT_HEIGHT];
#pragma HLS array_partition variable=in_signal complete dim=0
#pragma HLS array_partition variable=in_filter complete dim=0
    // The partial sum coming out of the PE element systolic array at the given index
    // Think of extra row as initial psum of zero hold
    data_stream_t psums[FILTER_HEIGHT + 1][OUT_HEIGHT];
#pragma HLS array_partition variable=psums complete dim=0

    static_assert(FILTER_HEIGHT + OUT_HEIGHT - 1 == SIGNAL_HEIGHT, "For the systolic indexing to work, this has to hold");

	load_streams(in_signal, signal, in_filter, kernel, psums);

    // Logical dataflow impl. Start from bottom and go to top so don't have data stalls
	create_pe(in_signal, in_filter, psums);

    drain(psums, out);
	return;
}



























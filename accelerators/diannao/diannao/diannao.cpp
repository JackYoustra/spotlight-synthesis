#include "hls_stream.h"
#include "systolic.h"

typedef hls::stream<data_t> data_stream_t;

// Reduction tree moment
// Reference: https://pages.saclay.inria.fr/olivier.temam/files/eval/CDSWWCT14.pdf
// Reference (slides): https://pdfs.semanticscholar.org/d680/8967f02d277b4d18a196588fae3f92421148.pdf (slide 21, NFU 1 and 2)
void conv_2d(data_t signal[SIGNAL_HEIGHT][SIGNAL_WIDTH], data_t kernel[FILTER_HEIGHT][FILTER_WIDTH], data_t out[OUT_HEIGHT][OUT_WIDTH]) {
#pragma HLS INTERFACE s_axilite port=signal bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=kernel bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=out bundle="HLS_DATABUS"
#pragma HLS INTERFACE s_axilite port=return bundle="HLS_DATABUS"
//#pragma HLS DATAFLOW
	// Systolic array dims: Filter_height rows, out_height cols

	// The signal and filter elements going into the array at the index of the PE element systolic array
//    data_stream_t in_signal[FILTER_HEIGHT][OUT_HEIGHT][SIGNAL_WIDTH], in_filter[FILTER_HEIGHT][OUT_HEIGHT][FILTER_WIDTH];
//#pragma HLS array_partition variable=in_signal complete dim=0
//#pragma HLS array_partition variable=in_filter complete dim=0
//    // The partial sum coming out of the PE element systolic array at the given index
//    // Think of extra row as initial psum of zero hold
//    data_stream_t psums[FILTER_HEIGHT + 1][OUT_HEIGHT][OUT_WIDTH];
//#pragma HLS array_partition variable=psums complete dim=0

    static_assert(FILTER_HEIGHT + OUT_HEIGHT - 1 == SIGNAL_HEIGHT, "For the systolic indexing to work, this has to hold");

//	load_streams(in_signal, signal, in_filter, kernel, psums);

	for (int out_row = 0; out_row < OUT_HEIGHT; out_row++) {
#pragma HLS unroll
		for (int out_col = 0; out_col < OUT_WIDTH; out_col++) {
#pragma HLS unroll
			// Reduction tree portion
			data_t value = 0;
			for (int filter_row = 0; filter_row < FILTER_HEIGHT; filter_row++) {
#pragma HLS unroll
				for (int filter_col = 0; filter_col < FILTER_WIDTH; filter_col++) {
#pragma HLS unroll
					const auto signal_row = out_row + filter_row;
					const auto signal_col = out_col + filter_col;
					value += signal[signal_row][signal_col] * kernel[filter_row][filter_col];
				}
			}
			out[out_row][out_col] = value;
		}
	}

//    // Logical dataflow impl. Start from bottom and go to top so don't have data stalls
//	for (int systolic_row = FILTER_HEIGHT - 1; systolic_row >= 0; systolic_row--) {//rows: bottom to top
//#pragma HLS unroll
//		for (int systolic_col = 0; systolic_col < OUT_HEIGHT; systolic_col++) { //columns: left to right
//#pragma HLS unroll
//    		const auto column_end = (systolic_col + 1) == OUT_HEIGHT;
//    		const auto row_end = (systolic_row - 1) < 0;
//    		if (column_end) {
//    			// Up and to the right, as well as to the right, doesn't exist
//    			// Neither the in_signal nor in_filter should be used as an output target, but don't know how to express that...
//    			right_edge_PE(in_signal[systolic_row][systolic_col], in_filter[systolic_row][systolic_col], psums[systolic_row + 1][systolic_col],
//						psums[systolic_row][systolic_col]);
//    		} else if (row_end) {
//    			// Up and to the right doesn't exist, but just to the right does
//    			// The in_signal shoudln't be used
//    			top_edge_PE(in_signal[systolic_row][systolic_col], in_filter[systolic_row][systolic_col], psums[systolic_row + 1][systolic_col],
//						in_filter[systolic_row][systolic_col + 1], psums[systolic_row][systolic_col]);
//    		} else {
//    			PE(in_signal[systolic_row][systolic_col], in_filter[systolic_row][systolic_col], psums[systolic_row + 1][systolic_col], // in
//    				in_signal[systolic_row - 1][systolic_col + 1], in_filter[systolic_row][systolic_col + 1], psums[systolic_row][systolic_col]); // out
//    		}
//    	}
//    }

//    drain(psums, out);
	return;
}

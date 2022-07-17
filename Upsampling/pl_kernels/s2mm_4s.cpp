/**********
© Copyright 2020 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**********/


#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>


extern "C" {

void s2mm(ap_int<16>* mem, hls::stream<qdma_axis<16, 0, 0, 0>  >& s1, hls::stream<qdma_axis<16, 0, 0, 0>  >& s2,  hls::stream<qdma_axis<16, 0, 0, 0>  >& s3,  hls::stream<qdma_axis<16, 0, 0, 0>  >& s4, int size) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem

#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=s4

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

	for(int i = 0; i < size; i++) {
#pragma HLS PIPELINE II=1
		qdma_axis<16, 0, 0, 0> x1 = s1.read();
		mem[i] = x1.data;
	}

	for(int j = size; j < 2*size; j++) {
#pragma HLS PIPELINE II=1
                qdma_axis<16, 0, 0, 0> x2 = s2.read();
                mem[j] = x2.data;
        }

	for(int m = 2*size; m < 3*size; m++) {
#pragma HLS PIPELINE II=1
                qdma_axis<16, 0, 0, 0> x3 = s3.read();
                mem[m] = x3.data;
        }

	for(int n = 3*size; n < 4*size; n++) {
#pragma HLS PIPELINE II=1
                qdma_axis<16, 0, 0, 0> x4 = s4.read();
                mem[n] = x4.data;
        }


}

}

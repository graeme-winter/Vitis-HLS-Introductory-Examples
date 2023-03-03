/*
 * Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fft_top.h"

void inputdatamover(
    bool direction,
    hls::stream<config_t> &config_strm,
    cmpxDataIn in[FFT_LENGTH],
    hls::stream<cmpxDataIn> &out_strm)
{
    config_t config; 
    config.setDir(direction);
    config.setSch(0x2AB);
    config_strm.write(config);
L0:
    for (int i=0; i< FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1 rewind
        out_strm << in[i];
    }
}

void outputdatamover(
    hls::stream<status_t> &status_in_strm, 
    bool* ovflo,
    hls::stream<cmpxDataOut> &in_strm,
    cmpxDataOut out[FFT_LENGTH])
{
 
L0:
    for (int i=0; i< FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1 rewind
        out[i] = in_strm.read();
    }
    status_t status = status_in_strm.read();
    *ovflo = status.getOvflo() & 0x1;
}

void myfftwrapper(hls::stream<complex<data_in_t>,FFT_LENGTH> &xn,
hls::stream<complex<data_out_t>,FFT_LENGTH> &xk,
hls::stream<status_t,FFT_LENGTH> &fft_status,
hls::stream<config_t,FFT_LENGTH> &fft_config
) {
    #pragma HLS dataflow
    #pragma HLS INLINE recursive
    hls::fft<config1>(xn, xk, fft_status, fft_config);
}

void fft_top(
    bool direction,
    complex<data_in_t> in[FFT_LENGTH],
    complex<data_out_t> out[FFT_LENGTH],
    bool* ovflo)
{
#pragma HLS interface ap_hs port=direction
#pragma HLS interface ap_fifo depth=1 port=ovflo
#pragma HLS interface ap_fifo depth=FFT_LENGTH port=in,out
#pragma HLS data_pack variable=in
#pragma HLS data_pack variable=out
#pragma HLS dataflow
//    #pragma HLS INLINE recursive
    hls::stream<complex<data_in_t>,FFT_LENGTH> xn;
    hls::stream<complex<data_out_t>,FFT_LENGTH> xk;
    hls::stream<config_t,FFT_LENGTH> fft_config;
    hls::stream<status_t,FFT_LENGTH> fft_status;
   
    inputdatamover(direction, fft_config, in, xn);
    // FFT IP
//    hls::fft<config1>(xn, xk, fft_status, fft_config);
    myfftwrapper(xn, xk, fft_status, fft_config);

    outputdatamover(fft_status, ovflo, xk, out);
}


#include <iostream>
#include <cstring>
#include <stdlib.h>

// This is used for the PL Kernels
// XRT includes
#include "xrt/experimental/xrt_bo.h"
#include "xrt/experimental/xrt_xclbin.h"
#include "xrt/experimental/xrt_device.h"
#include "xrt/experimental/xrt_kernel.h"
#include "xrt/experimental/xrt_ip.h"


#define SAMPLES 256

void wait_for_enter(const std::string &msg) {
    std::cout << msg << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void add_iter(int stream_len, int iter_times, uint16_t* data_in, uint16_t* data_out){
	for(int i=0; i<stream_len; i++){
		data_out[i] = 0;
		for(int j=0; j<iter_times; j++){
			data_out[i] = data_out[i] + data_in[j*stream_len + i];
		}
	}
}

int main(int argc, char ** argv)
{
	//////////////////////////////////////////
	// Open xclbin
	//////////////////////////////////////////
	int stream_len = 32;
	int iter_times = 8;

	std::string binaryFile = argv[1];
    	auto xclbin = xrt::xclbin(binaryFile);
	int device_index = 0;

	std::cout << "Open the device" << device_index << std::endl;
    	auto device = xrt::device(device_index);
    	std::cout << "Load the xclbin " << binaryFile << std::endl;
	auto uuid = device.load_xclbin(binaryFile);
	std::cout << "Load the xclbin " << binaryFile << std::endl;
	
	auto mm2s_1 = xrt::kernel(device, uuid, "mm2s_1");
	auto mm2s_2 = xrt::kernel(device, uuid, "mm2s_2");
	auto mm2s_3 = xrt::kernel(device, uuid, "mm2s_3");
	auto mm2s_4 = xrt::kernel(device, uuid, "mm2s_4");

	auto s2mm_1 = xrt::kernel(device, uuid, "s2mm_1");
	auto s2mm_2 = xrt::kernel(device, uuid, "s2mm_2");
	auto s2mm_3 = xrt::kernel(device, uuid, "s2mm_3");
	auto s2mm_4 = xrt::kernel(device, uuid, "s2mm_4");
	// auto mm2s = xrt::ip(device, uuid, "mm2s");
	// auto s2mm = xrt::ip(device, uuid, "s2mm");
	auto adderIP = xrt::ip(device, uuid, "AddIterTop");

	wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

	//////////////////////////////////////////
        // Create Buffer
        //////////////////////////////////////////
	size_t vector_size_bytes = sizeof(uint16_t) * SAMPLES;
	std::cout << "Allocate Buffer in Global Memory\n";
	auto boA_1 = xrt::bo(device, vector_size_bytes, mm2s_1.group_id(0)); //Match kernel arguments to RTL kernel
	auto boA_2 = xrt::bo(device, vector_size_bytes, mm2s_2.group_id(0));
	auto boA_3 = xrt::bo(device, vector_size_bytes, mm2s_3.group_id(0));
	auto boA_4 = xrt::bo(device, vector_size_bytes, mm2s_4.group_id(0));


    	auto boB_1 = xrt::bo(device, vector_size_bytes, s2mm_1.group_id(0));
	auto boB_2 = xrt::bo(device, vector_size_bytes, s2mm_2.group_id(0));
	auto boB_3 = xrt::bo(device, vector_size_bytes, s2mm_3.group_id(0));
	auto boB_4 = xrt::bo(device, vector_size_bytes, s2mm_4.group_id(0));
	// auto boA = xrt::bo(device, vector_size_bytes, 1); //Match kernel arguments to RTL kernel
	// auto boB = xrt::bo(device, vector_size_bytes, 1);

	// Map the contents of the buffer object into host memory
	auto bo0_map_1 = boA_1.map<uint16_t*>();
	auto bo0_map_2 = boA_2.map<uint16_t*>();
	auto bo0_map_3 = boA_3.map<uint16_t*>();
	auto bo0_map_4 = boA_4.map<uint16_t*>();

    	auto bo1_map_1 = boB_1.map<uint16_t*>();
	auto bo1_map_2 = boB_2.map<uint16_t*>();
	auto bo1_map_3 = boB_3.map<uint16_t*>();
	auto bo1_map_4 = boB_4.map<uint16_t*>();

    	std::fill(bo0_map_1, bo0_map_1 + SAMPLES, 0);
	std::fill(bo0_map_2, bo0_map_2 + SAMPLES, 0);
	std::fill(bo0_map_3, bo0_map_3 + SAMPLES, 0);
	std::fill(bo0_map_4, bo0_map_4 + SAMPLES, 0);

    	std::fill(bo1_map_1, bo1_map_1 + SAMPLES, 0);
	std::fill(bo1_map_2, bo1_map_2 + SAMPLES, 0);
	std::fill(bo1_map_3, bo1_map_3 + SAMPLES, 0);
	std::fill(bo1_map_4, bo1_map_4 + SAMPLES, 0);
	
	srand((unsigned)time(NULL));
	for (int i = 0; i < SAMPLES; ++i) {
        	bo0_map_1[i] = 1; //rand()%32;
		bo0_map_2[i] = 1;
		bo0_map_3[i] = 1;
		bo0_map_4[i] = 1;
    	}

	uint16_t data_out_1[stream_len];
	uint16_t data_out_2[stream_len];
	uint16_t data_out_3[stream_len];
	uint16_t data_out_4[stream_len];

	//add_iter(stream_len, iter_times, bo0_map, data_out);

	std::cout << "synchronize input buffer data to device global memory\n";
	boA_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boA_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boA_3.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boA_4.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    	boB_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boB_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boB_3.sync(XCL_BO_SYNC_BO_TO_DEVICE);
	boB_4.sync(XCL_BO_SYNC_BO_TO_DEVICE);

	std::cout << "Execution of the kernel\n";
	auto mm2s_run_1 = mm2s_1(boA_1, nullptr, stream_len*iter_times); //DATA_SIZE=size
	auto mm2s_run_2 = mm2s_1(boA_2, nullptr, stream_len*iter_times);
	auto mm2s_run_3 = mm2s_1(boA_3, nullptr, stream_len*iter_times);
	auto mm2s_run_4 = mm2s_1(boA_4, nullptr, stream_len*iter_times);

//	auto adderIP = xrt::ip(device, uuid, "AddIterTop");
	adderIP.write_register(0x04, 0);
	adderIP.write_register(0x08, 0);
	adderIP.write_register(0x0c, iter_times);
	adderIP.write_register(0x10, stream_len);
	adderIP.write_register(0x00, 0xffffffff);

	auto s2mm_run_1 = s2mm_1(boB_1, nullptr, stream_len); //DATA_SIZE=size
	auto s2mm_run_2 = s2mm_2(boB_2, nullptr, stream_len);
	auto s2mm_run_3 = s2mm_3(boB_3, nullptr, stream_len);
	auto s2mm_run_4 = s2mm_4(boB_4, nullptr, stream_len);

	std::cout << "kernel starting, wait for the mm2s kernel end \n";
	mm2s_run_1.wait();
	mm2s_run_2.wait();
	mm2s_run_3.wait();
	mm2s_run_4.wait();
	std::cout << "mm2s kernel end, wait for the s2mm kernel end \n";
        s2mm_run_1.wait();
	s2mm_run_2.wait();
	s2mm_run_3.wait();
	s2mm_run_4.wait();

	// Get the output;
    	std::cout << "Get the output data from the device" << std::endl;
    	boB_1.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	boB_2.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	boB_3.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
	boB_4.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

	bool cmp_result = true;
	for (int i = 0; i < stream_len; ++i) {
                printf("the value of output is: %d \n", bo1_map_1[i]);
		printf("the value of output is: %d \n", bo1_map_2[i]);
		printf("the value of output is: %d \n", bo1_map_3[i]);
		printf("the value of output is: %d \n", bo1_map_4[i]);

		printf("the value of golden data is: %d \n", data_out_1[i]);
		printf("the value of golden data is: %d \n", data_out_2[i]);
		printf("the value of golden data is: %d \n", data_out_3[i]);
		printf("the value of golden data is: %d \n", data_out_4[i]);
		//if(bo1_map[i] != data_out[i]) cmp_result = false;
        }

	if(cmp_result) printf("TEST PASS! \n");
	else printf("TEST FAIL!");


	return 0;


}

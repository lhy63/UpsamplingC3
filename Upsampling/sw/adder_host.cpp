#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <fstream>

// This is used for the PL Kernels
// XRT includes
#include "xrt/experimental/xrt_bo.h"
#include "xrt/experimental/xrt_xclbin.h"
#include "xrt/experimental/xrt_device.h"
#include "xrt/experimental/xrt_kernel.h"
#include "xrt/experimental/xrt_ip.h"
#include<iostream>
#include"bmpFile.h"

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
	//int stream_len = 32;
	//int iter_times = 8;

	std::string binaryFile = argv[1];
  std::string imageNum = argv[2];
  std::string imageDir = argv[3];
  std::string outFileName = argv[4];

  auto xclbin = xrt::xclbin(binaryFile);
	int device_index = 0;

	std::cout << "Open the device" << device_index << std::endl;
  auto device = xrt::device(device_index);
  std::cout << "Load the xclbin " << binaryFile << std::endl;
	auto uuid = device.load_xclbin(binaryFile);
	std::cout << "Finsh Load the xclbin " << binaryFile << std::endl;
	
	auto mm2s_1 = xrt::kernel(device, uuid, "mm2s_1");
	auto s2mm_1 = xrt::kernel(device, uuid, "s2mm_1");
 
  auto mm2s_2 = xrt::kernel(device, uuid, "mm2s_2");
	auto s2mm_2 = xrt::kernel(device, uuid, "s2mm_2");
 
  auto mm2s_3 = xrt::kernel(device, uuid, "mm2s_3");
	auto s2mm_3 = xrt::kernel(device, uuid, "s2mm_3");
	//auto adderIP = xrt::ip(device, uuid, "AddIterTop");

	wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

  bmp image;
  image.readPic(imageDir);
  int inputImgH = image.getBmpFileInFoHeader().biHeight;
  int inputImgW = image.getBmpFileInFoHeader().biWidth;
  auto inputImgDataSizeC1 = inputImgH * inputImgW;
  
  //output image constrution
  int outputImgW = inputImgW, outputImgH = inputImgH;  //*4
  auto outputImgDataSizeC1 = inputImgDataSizeC1;//* 16
  unsigned int BfSize = sizeof(BmpFileHeader) + sizeof(BmpFileInFoHeader) + outputImgDataSizeC1*3;
  BmpFileHeader outFileHeader = {BMPTYPE, BfSize, 0, 0, 138};
  BmpFileInFoHeader outFileInFoHeader = {40, outputImgW, outputImgH, 1, 24, 0, outputImgDataSizeC1*3, 0, 0, 0, 0};
  auto outputSurface = new RGBTriple[outputImgDataSizeC1];
  int outputoffset = (outputImgW * 3) % 4;
  
  
  
  
	//////////////////////////////////////////
  // Create Buffer
  //////////////////////////////////////////
	//size_t vector_size_bytes = sizeof(uint16_t) * SAMPLES;
	std::cout << "Allocate Buffer in Global Memory\n";
	auto boA_1= xrt::bo(device,  inputImgDataSizeC1, mm2s_1.group_id(0)); //Match kernel arguments to RTL kernel
  auto boB_1 = xrt::bo(device, outputImgDataSizeC1, s2mm_1.group_id(0));
  
  auto boA_2 = xrt::bo(device, inputImgDataSizeC1, mm2s_2.group_id(0)); //Match kernel arguments to RTL kernel
  auto boB_2 = xrt::bo(device, outputImgDataSizeC1, s2mm_2.group_id(0));
  
  auto boA_3 = xrt::bo(device, inputImgDataSizeC1, mm2s_3.group_id(0)); //Match kernel arguments to RTL kernel
  auto boB_3 = xrt::bo(device, outputImgDataSizeC1, s2mm_3.group_id(0));

	// Map the contents of the buffer object into host memory
  auto bo0_map_3 = boA_3.map<uint8_t*>();
  auto bo1_map_3 = boB_3.map<uint8_t*>();
  std::fill(bo0_map_3, bo0_map_3 + inputImgDataSizeC1, 0);
  std::fill(bo1_map_3, bo1_map_3 + outputImgDataSizeC1, 0);
 
	auto bo0_map_2 = boA_2.map<uint8_t*>();
  auto bo1_map_2 = boB_2.map<uint8_t*>();
  std::fill(bo0_map_2, bo0_map_2 + inputImgDataSizeC1, 0);
  std::fill(bo1_map_2, bo1_map_2 + outputImgDataSizeC1, 0);
	
  auto bo0_map_1 = boA_1.map<uint8_t*>();
  auto bo1_map_1 = boB_1.map<uint8_t*>();
  std::fill(bo0_map_1, bo0_map_1 + inputImgDataSizeC1, 0);
  std::fill(bo1_map_1, bo1_map_1 + outputImgDataSizeC1, 0);
 
	for (int i = 0; i < inputImgDataSizeC1; ++i) {
	       bo0_map_1[i] = image.getRGBTriple()[i].blue;
         bo0_map_2[i] = image.getRGBTriple()[i].green;
         bo0_map_3[i] = image.getRGBTriple()[i].red;
  }

	std::cout << "synchronize input buffer data to device global memory\n";
	boA_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  boB_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);
 	boA_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  boB_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);
 	boA_3.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  boB_3.sync(XCL_BO_SYNC_BO_TO_DEVICE);

	std::cout << "Execution of the kernel\n";
	auto mm2s_run_1 = mm2s_1(boA_1, nullptr, inputImgDataSizeC1); //DATA_SIZE=size
  auto mm2s_run_2 = mm2s_2(boA_2, nullptr, inputImgDataSizeC1); //DATA_SIZE=size
  auto mm2s_run_3 = mm2s_3(boA_3, nullptr, inputImgDataSizeC1); //DATA_SIZE=size

	auto s2mm_run_1 = s2mm_1(boB_1, nullptr, outputImgDataSizeC1); //DATA_SIZE=size
  auto s2mm_run_2 = s2mm_2(boB_2, nullptr, outputImgDataSizeC1); //DATA_SIZE=size
  auto s2mm_run_3 = s2mm_3(boB_3, nullptr, outputImgDataSizeC1); //DATA_SIZE=size
 

	std::cout << "3-kernel starting, wait for the mm2s kernel end \n";
	mm2s_run_1.wait();
  mm2s_run_2.wait();
  mm2s_run_3.wait();
	std::cout << "mm2s kernel end, wait for the s2mm kernel end \n";
  s2mm_run_1.wait();
  s2mm_run_2.wait();
  s2mm_run_3.wait();


	// Get the output;
  std::cout << "Get the output data from the device" << std::endl;
  boB_1.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  boB_2.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  boB_3.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  
  for (int i = 0;i <outputImgH;i++) {
     for (int j = 0;j < outputImgW;j++) {
           outputSurface[outputImgW * i + j].blue = bo1_map_1[outputImgW * i + j];   
           outputSurface[outputImgW * i + j].green = bo1_map_2[outputImgW * i + j]; 
           outputSurface[outputImgW * i + j].red = bo1_map_3[outputImgW * i + j]; 
        }
  }
  
  bmp outputBmp(outputoffset, outputSurface, outFileHeader, outFileInFoHeader);
  outputBmp.writePic(outFileName);

	bool cmp_result = true;
	for (size_t i = 0; i < 100 ; ++i) {
    printf("the value of bule:%ld-input is: %d \n", i,bo0_map_1[i]);
		printf("the value of bule:%ld-out is: %d \n", i,bo1_map_1[i]);
    printf("the value of green:%ld-input is: %d \n", i,bo0_map_2[i]);
		printf("the value of green:%ld-out is: %d \n", i,bo1_map_2[i]);
    printf("the value of red:%ld-input is: %d \n", i,bo0_map_3[i]);
		printf("the value of red:%ld-out is: %d \n", i,bo1_map_3[i]);
		if(bo0_map_1[i] != bo1_map_1[i]) cmp_result = false;
    if(bo0_map_2[i] != bo1_map_2[i]) cmp_result = false;
    if(bo0_map_3[i] != bo1_map_3[i]) cmp_result = false;
  }

	if(cmp_result) printf("TEST PASS! \n");
	else printf("TEST FAIL!");


	return 0;


}
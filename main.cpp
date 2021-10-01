#include <CL/cl.hpp>
#include <vector>
#include <iostream>
#include <string>

//-------------------------------------------
//第四步 撰寫 OpenCL 的 kernel
//這段代碼代表 A[:] 和 B[:] 的 vector 依序相加到 C[:]，其中 get_global_id 是獲取執行緒編號，
// 假設有 100 個執行緒，那執行緒編號就會從 0 數到 99。
//-------------------------------------------
static std::string opencl_kernel =
		R"(
            __kernel void vecadd
            (
                __global int *A,
                 __global int *B,
                __global int *C
            )
            {
                int id = get_global_id(0);
                C[id] = A[id] + B[id];
            }
         )";

int main(int argc, char** argv) {

	//-------------------------------------------
	//第二步 獲取平台 (platform) 和裝置 (device)
	//OpenCL 能夠兼容不同平台和裝置，必須要先收集所有可用的硬體。一般情況下，平台就是你目前使用的電腦，
	// 裝置就是你電腦的 CPU 和 GPU，設備這邊設定 CL_DEVICE_TYPE_GPU 限定使用 GPU 。
	//	CL_DEVICE_TYPE_CPU：使用 CPU 裝置
	//	CL_DEVICE_TYPE_GPU：使用顯示晶片裝置
	//	CL_DEVICE_TYPE_ACCELERATOR：特定的 OpenCL 加速裝置，例如 CELL
	//	CL_DEVICE_TYPE_DEFAULT：系統預設的 OpenCL 裝置
	//	CL_DEVICE_TYPE_ALL：所有系統中的 OpenCL 裝置
	//-------------------------------------------
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);  // 檢查平台數目

	if (platforms.empty()) {
		printf("No platforms!\n");
		return 1;
	}

	cl::Platform platform = platforms[0];
	std::vector<cl::Device> Devices;    // 檢查裝置數目

	platform.getDevices(CL_DEVICE_TYPE_GPU, &Devices);
	if (Devices.empty()) {
		printf("No Devices!\n");
		return 1;
	}

	cl::Device device = Devices[0];
	std::cout << "Device : " << device.getInfo<CL_DEVICE_NAME>() << "\n";

	//-------------------------------------------
	//第三步 產生 context
	//在 OpenCL 裡的架構裡，context 是整個執行的虛擬裝置，device 是實際執行的裝置，
	// 基本上所有的執行指令都是對 context 操作的，而 context 會幫你轉到對應的 device 上執行。
	//-------------------------------------------
	cl::Context context({ device });

	//-------------------------------------------
	//第五步 編譯 Kernel
	//OpenCL 屬於執行期間編譯， cladd 是 kernel 的函式名稱，裡面的程式碼尚未經過編譯，
	// cl::Program 會先將文字讀入，program.build 會對整段文字編譯，最終 cl::Kernel 加載編譯好的 kernel。
	//-------------------------------------------
	cl::Program program(context, opencl_kernel);

	if (program.build({ device }) != CL_SUCCESS) {  // 編譯程式
		printf("Fail to build\n");
		return 1;
    }

    cl::Kernel vec_add(program, "vecadd");

    //-------------------------------------------
    //第六步 設定 buffer 並連接
    //由於 GPU 和 CPU 的記憶體不共通，GPU 只能使用自己的記憶體，cl::Buffer 代表 GPU 的記憶體。
    // setArg 連接 kernel 和 buffer 讓資料能進入 function。
    //-------------------------------------------
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int) * 100);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int) * 100);
    cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(int) * 100);

	vec_add.setArg(0, buffer_A);    // 連接 buffer A 到第一個參數位置
	vec_add.setArg(1, buffer_B);    // 連接 buffer B 到第二個參數位置
	vec_add.setArg(2, buffer_C);    // 連接 buffer C 到第三個參數位置

	//-------------------------------------------
	//第七步 產生 CommandQueue
	//剛剛提到所有操作都是針對 context ，但還是指定一的實際的裝置執行指令，
	// cl::CommandQueue 的一個主要目的就是指定執行裝置。
	//-------------------------------------------
	cl::CommandQueue queue(context, device);

	//-------------------------------------------
	//第八步 運行
	//通過 cl::CommandQueue 執行程式。
	//-------------------------------------------
	std::vector<int> A(100, 1);
	std::vector<int> B(100, 2);
	std::vector<int> C(100, 0);

	queue.enqueueWriteBuffer(buffer_A, CL_FALSE, 0, sizeof(int) * 100, A.data());   //寫入資料
	queue.enqueueWriteBuffer(buffer_B, CL_FALSE, 0, sizeof(int) * 100, B.data());   //寫入資料
	queue.finish(); // 同步

	queue.enqueueNDRangeKernel(vec_add, cl::NullRange,
	                           cl::NDRange(100),     // 設定執行緒數目
	                           cl::NullRange);

	queue.enqueueReadBuffer(buffer_C, CL_FALSE, 0, sizeof(int) * 100, C.data());    // 讀出資料
	queue.finish(); // 同步

	for (const auto& v : A) {
		std::cout << v << " ";
	}
	std::cout << std::endl;

	for (const auto& v : B) {
		std::cout << v << " ";
	}
	std::cout << std::endl;

	for (const auto& v : C) {
		std::cout << v << " ";
	}
	std::cout << std::endl;

	return 0;

	//-------------------------------------------
	//第九步 編譯(CMD)
	//編譯的參數如下
	// $ g++ main.cpp -o first_opencl -std=c++11 -lOpenCL
	//
	//運行
	//$ ./first_opencl
	//-------------------------------------------
}
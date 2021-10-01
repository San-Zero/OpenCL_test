#include <CL/cl.hpp>
#include <vector>
#include <iostream>
#include <string>

//-------------------------------------------
//�ĥ|�B ���g OpenCL �� kernel
//�o�q�N�X�N�� A[:] �M B[:] �� vector �̧Ǭۥ[�� C[:]�A�䤤 get_global_id �O���������s���A
// ���]�� 100 �Ӱ�����A��������s���N�|�q 0 �ƨ� 99�C
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
	//�ĤG�B ������x (platform) �M�˸m (device)
	//OpenCL ����ݮe���P���x�M�˸m�A�����n�������Ҧ��i�Ϊ��w��C�@�뱡�p�U�A���x�N�O�A�ثe�ϥΪ��q���A
	// �˸m�N�O�A�q���� CPU �M GPU�A�]�Ƴo��]�w CL_DEVICE_TYPE_GPU ���w�ϥ� GPU �C
	//	CL_DEVICE_TYPE_CPU�G�ϥ� CPU �˸m
	//	CL_DEVICE_TYPE_GPU�G�ϥ���ܴ����˸m
	//	CL_DEVICE_TYPE_ACCELERATOR�G�S�w�� OpenCL �[�t�˸m�A�Ҧp CELL
	//	CL_DEVICE_TYPE_DEFAULT�G�t�ιw�]�� OpenCL �˸m
	//	CL_DEVICE_TYPE_ALL�G�Ҧ��t�Τ��� OpenCL �˸m
	//-------------------------------------------
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);  // �ˬd���x�ƥ�

	if (platforms.empty()) {
		printf("No platforms!\n");
		return 1;
	}

	cl::Platform platform = platforms[0];
	std::vector<cl::Device> Devices;    // �ˬd�˸m�ƥ�

	platform.getDevices(CL_DEVICE_TYPE_GPU, &Devices);
	if (Devices.empty()) {
		printf("No Devices!\n");
		return 1;
	}

	cl::Device device = Devices[0];
	std::cout << "Device : " << device.getInfo<CL_DEVICE_NAME>() << "\n";

	//-------------------------------------------
	//�ĤT�B ���� context
	//�b OpenCL �̪��[�c�̡Acontext �O��Ӱ��檺�����˸m�Adevice �O��ڰ��檺�˸m�A
	// �򥻤W�Ҧ���������O���O�� context �ާ@���A�� context �|���A�������� device �W����C
	//-------------------------------------------
	cl::Context context({ device });

	//-------------------------------------------
	//�Ĥ��B �sĶ Kernel
	//OpenCL �ݩ��������sĶ�A cladd �O kernel ���禡�W�١A�̭����{���X�|���g�L�sĶ�A
	// cl::Program �|���N��rŪ�J�Aprogram.build �|���q��r�sĶ�A�̲� cl::Kernel �[���sĶ�n�� kernel�C
	//-------------------------------------------
	cl::Program program(context, opencl_kernel);

	if (program.build({ device }) != CL_SUCCESS) {  // �sĶ�{��
		printf("Fail to build\n");
		return 1;
    }

    cl::Kernel vec_add(program, "vecadd");

    //-------------------------------------------
    //�Ĥ��B �]�w buffer �ós��
    //�ѩ� GPU �M CPU ���O���餣�@�q�AGPU �u��ϥΦۤv���O����Acl::Buffer �N�� GPU ���O����C
    // setArg �s�� kernel �M buffer ����Ư�i�J function�C
    //-------------------------------------------
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int) * 100);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int) * 100);
    cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(int) * 100);

	vec_add.setArg(0, buffer_A);    // �s�� buffer A ��Ĥ@�ӰѼƦ�m
	vec_add.setArg(1, buffer_B);    // �s�� buffer B ��ĤG�ӰѼƦ�m
	vec_add.setArg(2, buffer_C);    // �s�� buffer C ��ĤT�ӰѼƦ�m

	//-------------------------------------------
	//�ĤC�B ���� CommandQueue
	//��责��Ҧ��ާ@���O�w�� context �A���٬O���w�@����ڪ��˸m������O�A
	// cl::CommandQueue ���@�ӥD�n�ت��N�O���w����˸m�C
	//-------------------------------------------
	cl::CommandQueue queue(context, device);

	//-------------------------------------------
	//�ĤK�B �B��
	//�q�L cl::CommandQueue ����{���C
	//-------------------------------------------
	std::vector<int> A(100, 1);
	std::vector<int> B(100, 2);
	std::vector<int> C(100, 0);

	queue.enqueueWriteBuffer(buffer_A, CL_FALSE, 0, sizeof(int) * 100, A.data());   //�g�J���
	queue.enqueueWriteBuffer(buffer_B, CL_FALSE, 0, sizeof(int) * 100, B.data());   //�g�J���
	queue.finish(); // �P�B

	queue.enqueueNDRangeKernel(vec_add, cl::NullRange,
	                           cl::NDRange(100),     // �]�w������ƥ�
	                           cl::NullRange);

	queue.enqueueReadBuffer(buffer_C, CL_FALSE, 0, sizeof(int) * 100, C.data());    // Ū�X���
	queue.finish(); // �P�B

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
	//�ĤE�B �sĶ(CMD)
	//�sĶ���ѼƦp�U
	// $ g++ main.cpp -o first_opencl -std=c++11 -lOpenCL
	//
	//�B��
	//$ ./first_opencl
	//-------------------------------------------
}
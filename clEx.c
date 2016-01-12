#include <stdio.h>
#include <CL/cl.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int checkErr(cl_int clErr, char* filename, int line);

int main(int argc, char** argv)
{
// determine number of platforms
cl_int clErr;
cl_uint num_platforms;
clErr = clGetPlatformIDs(0,NULL,&num_platforms);
checkErr(clErr,__FILE__,__LINE__);
printf("OpenCL Platforms found: %i\n",num_platforms);
if(num_platforms<1) { return 0; }

// get platform IDs
cl_platform_id platforms[num_platforms];
clErr = clGetPlatformIDs(num_platforms,platforms,NULL);
checkErr(clErr,__FILE__,__LINE__);

cl_int num_devices;
clErr = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
checkErr(clErr, __FILE__, __LINE__);
printf("Devices found: %i\n", num_devices);

cl_device_id device;
clErr = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, 1, &device, NULL);
checkErr(clErr, __FILE__, __LINE__);
printf("Device id: %p\n", device);

cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &clErr);
checkErr(clErr, __FILE__, __LINE__);
cl_command_queue queue = clCreateCommandQueue(context, device, 0, &clErr);
checkErr(clErr,__FILE__,__LINE__);

cl_int vecSize = 10;
cl_mem memA = clCreateBuffer(context, CL_MEM_READ_ONLY, vecSize * sizeof(int), NULL, &clErr);
checkErr(clErr,__FILE__,__LINE__);
cl_mem memB = clCreateBuffer(context, CL_MEM_READ_ONLY, vecSize * sizeof(int), NULL, &clErr);
checkErr(clErr,__FILE__,__LINE__);
cl_mem memC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, vecSize * sizeof(int), NULL, &clErr);
checkErr(clErr,__FILE__,__LINE__);

const char* src = "__kernel void vecAdd(__global int* A, __global int* B, __global int* C)\n"
"{\n"
"int id = get_global_id(0); \n"
"C[id] = A[id] + B[id];"
"}\n\0";
cl_program prog = clCreateProgramWithSource(context, 1, &src, NULL, &clErr);
checkErr(clErr,__FILE__,__LINE__);

clErr = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
checkErr(clErr,__FILE__,__LINE__);

cl_kernel kernel = clCreateKernel(prog, "vecAdd", &clErr);
checkErr(clErr,__FILE__,__LINE__);

clErr = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memA);
checkErr(clErr,__FILE__,__LINE__);
clErr = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memB);
checkErr(clErr,__FILE__,__LINE__);
clErr = clSetKernelArg(kernel, 2, sizeof(cl_mem), &memC);
checkErr(clErr,__FILE__,__LINE__);

const int A[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const int B[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
clErr = clEnqueueWriteBuffer(queue, memA, CL_TRUE, 0, vecSize * sizeof(int), A, 0, NULL, NULL);
checkErr(clErr,__FILE__,__LINE__);
clErr = clEnqueueWriteBuffer(queue, memB, CL_TRUE, 0, vecSize*sizeof(int), B, 0, NULL, NULL);
checkErr(clErr,__FILE__,__LINE__);

size_t gSize = vecSize;
size_t lSize = 1;
clErr = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &gSize, &lSize, 0, NULL, NULL);
checkErr(clErr,__FILE__,__LINE__);

int C[10];
clErr = clEnqueueReadBuffer(queue, memC, CL_TRUE, 0, vecSize * sizeof(int), C, 0, NULL, NULL);
checkErr(clErr,__FILE__,__LINE__);

int i;
for (i=0; i<vecSize; ++i)
{
	printf("%i + %i = %i \n", A[i], B[i], C[i]); 
}

return 0;
}

int checkErr(cl_int clErr, char* filename, int line)
{
   if (clErr!=CL_SUCCESS)
     {
       printf("OpenCL Error %i at line %i of%s\n",clErr,line,filename);
       return 1;
     }
}

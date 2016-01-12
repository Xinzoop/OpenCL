#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "./randomInit.c"
#include "./getErrorString.c"

static inline void loadBar(int x, int n, int r, int w);

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

int main(void) {
char buf[80];
system("clear");
printf("\n\n\n ### GPU ENABLED CODE\n");
printf("\n ### Host creating two input vectors and one output vector\n");
    unsigned i,  LIST_SIZE;
    FILE *fp0;

    fp0 = fopen("data.txt", "r");
    if (!fp0) {
        fprintf(stderr, "Failed to load data.txt.\n");
        exit(1);
    }
    fgets(buf, sizeof(buf),  fp0);
    sscanf(buf,"%d", &LIST_SIZE);
    printf(" ### Vector_size:%d elements, Elem_size:%lu byte\n", LIST_SIZE, sizeof(int));
    fclose(fp0);

    int *A = (int*)malloc(sizeof(int)*LIST_SIZE);
    if (A == NULL) {
    fprintf(stderr, "failed to allocate memory.\n");
    return -1;
    }

    int *B = (int*)malloc(sizeof(int)*LIST_SIZE);
    if (B == NULL) {
    fprintf(stderr, "failed to allocate memory.\n");
    return -1;
    }

    int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
    if (C == NULL) {
    fprintf(stderr, "failed to allocate memory.\n");
    return -1;
    }

printf("\n Host initialising input vector values\n");
    for(i = 0; i < LIST_SIZE; i++) {
        A[i] = /*randomInit()*/ i; 
        B[i] = /*randomInit()*/ LIST_SIZE-i;
        loadBar(i, LIST_SIZE, 100, 2);
    }

printf("\n Setting up GPU\n");    
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("vec.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    printf("Check platform id %s\n", getErrorString(ret));
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_CPU, 1, 
            &device_id, &ret_num_devices);
printf("device_id:%u, device_count:%u\n", (int) device_id, ret_num_devices);
     printf("Check device id %s\n", getErrorString(ret));

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
     printf("Check context %s\n", getErrorString(ret));

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
     printf("Check command queue %s\n", getErrorString(ret));

    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            LIST_SIZE * sizeof(int), NULL, &ret);
     printf("Check a_mem_obj %s\n", getErrorString(ret));
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            LIST_SIZE * sizeof(int), NULL, &ret);
     printf("Check b_mem_obj %s\n", getErrorString(ret));
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
            LIST_SIZE * sizeof(int), NULL, &ret);
     printf("Check c_mem_obj %s\n", getErrorString(ret));

// <-
    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);
     printf("Check program %s\n", getErrorString(ret));

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
//    ret = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
     printf("Check build program %s, device_id:%d \n", getErrorString(ret),(int)device_id);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
     printf("Check kernel create %s\n", getErrorString(ret));

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
     printf("Check kernel arg 0 %s\n", getErrorString(ret));
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
     printf("Check kernel arg 1 %s\n", getErrorString(ret));
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    ret |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &LIST_SIZE);
     printf("Check kernel args 0-3 %s\n", getErrorString(ret));

    // COPY EXECUTE BLOCK //////////////////////////////////////////////
    // ->Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            LIST_SIZE * sizeof(int), A, 0, NULL, NULL);
     printf("Check Read B %s\n", getErrorString(ret));
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), B, 0, NULL, NULL);
     printf("Check Read B %s\n", getErrorString(ret));

    // Execute the OpenCL kernel on the list
    printf("\n GPU adding the vectors \n");
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = 1; // Process one item at a time
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
    printf("Check kernel EnqueueNDRange %s\n", getErrorString(ret));

    // Read the memory buffer c_mem_obj on the device to the local C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), C, 0, NULL, NULL);
     printf("Check Read Buffer C %s\n", getErrorString(ret));
    ////////////////////////////////////////////////////////////////////

    // Display the first 10 result to the screen
    for(i = 0; i < 10; i++)
        printf("%d + %d = %d\n", A[i], B[i], C[i]);
    printf("\n");
    // Display the last 10 result to the screen
    for(i = LIST_SIZE-10; i < LIST_SIZE; i++)
        printf("%d + %d = %d\n", A[i], B[i], C[i]);

//// SECOND TIME ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
printf("\n Host initialising input vectors randomly \n");
    for(i = 0; i < LIST_SIZE; i++) {
        A[i] = randomInit(); 
        B[i] = randomInit();
        loadBar(i, LIST_SIZE, 100, 2);
    }

    // COPY EXECUTE BLOCK RE-RUN ////////////////////////////////////////
    // ->Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            LIST_SIZE * sizeof(int), A, 0, NULL, NULL);
     printf("Check Read B %s\n", getErrorString(ret));
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), B, 0, NULL, NULL);
     printf("Check Read B %s\n", getErrorString(ret));

    // Execute the OpenCL kernel on the list
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
    printf("Check kernel EnqueueNDRange %s\n", getErrorString(ret));

    // Read the memory buffer c_mem_obj on the device to the local C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), C, 0, NULL, NULL);
     printf("Check Read Buffer C %s\n", getErrorString(ret));
    ////////////////////////////////////////////////////////////////////

    // Display the first 10 result to the screen
    for(i = 0; i < 10; i++)
        printf("%d + %d = %d\n", A[i], B[i], C[i]);
    printf("\n");
    // Display the last 10 result to the screen
    for(i = LIST_SIZE-10; i < LIST_SIZE; i++)
        printf("%d + %d = %d\n", A[i], B[i], C[i]);


////// SECOND TIME ENDS/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
printf("\n Cleaning up GPU queues\n");
    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

printf("\n Host freeing up the vectors\n");
    free(A);
    free(B);
    free(C);
    return 0;
}

// Process has done i out of n rounds,
// and we want a bar of width w and resolution r.
static inline void loadBar(int x, int n, int r, int w)
{
    int k;
    // Only update r times.
    if ( x % (n/r +1) != 0 ) return;
 
    // Calculuate the ratio of complete-to-incomplete.
    float ratio = x/(float)n;
    int   c     = ratio * w;
 
    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );
 
    // Show the load bar.
    for (k=0; k<c; k++)
       printf("=");
 
    for (k=c; k<w; k++)
       printf(" ");

    // ANSI Control codes to go back to the
    // previous line and clear it.
    printf("]\n\033[F\033[J");
}


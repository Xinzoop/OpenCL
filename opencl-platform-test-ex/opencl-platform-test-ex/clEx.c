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
for (cl_uint i=0; i<num_platforms; ++i)
{
	printf("%");
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

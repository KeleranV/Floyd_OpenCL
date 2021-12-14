#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#include <CL/cl.hpp>
#include <vector>



int * matr_adj;
int n = 10;
int i;
int j;
int offset;

/*
void init(int n){
    matr_adj = (int *)calloc(n*n, sizeof(int*));
    //on va utiliser un offset : i*n + j pour naviguer sur m(i, j)
    
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < n; j++)
        {
            offset = i * n + j;
            if (j == i)
            { 
                matr_adj[offset] = 0;
                continue;
            }
            if (i < n-1)
            {
                matr_adj[offset] = 1;
                continue;
            }
            matr_adj[offset] = n+1;
        }
    }
    matr_adj[n - 1] = 1;
    matr_adj[n * (n - 1)] = 1;
}
*/

void main(){

    
    //Pas besoin de parraléliser cette attribution
    matr_adj = (int*)calloc(n * n, sizeof(int*));
    matr_adj[n - 1] = 1;
    matr_adj[n * (n - 1)] = 1;

    //Test initial pour vérifier que le PC est compatible avec OpenCL
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Device default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    cl::Context context({ default_device });

    cl::Program::Sources sources;

    std::string kernel_code =
        "void kernel generate(global int* M, global const int* i, global const int* j, global const int* off, global const int* n){"
        "   if (j[get_global_id(0)] == i[get_global_id(0)])"
        "   {"
        "       M[off[get_global_id(0)]] = 0;"
        "   }"
        "   if (i[get_global_id(0)] < *n - 1)"
        "   {"
        "       M[off[get_global_id(0)]] = 1;"
        "   }"
        "   M[off[get_global_id(0)]] = *n + 1; "
        "}";

    sources.push_back({ kernel_code.c_str(), kernel_code.length() });

    cl::Program program(context, sources);
    if (program.build({ default_device }) != CL_SUCCESS) {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }


    //int debug = 10;
    //init(debug);
    //int offset;


    //Création des buffers
    cl::Buffer buff_M(context, CL_MEM_READ_WRITE, n * n * sizeof(int*));
    cl::Buffer buff_i(context, CL_MEM_READ_WRITE, sizeof(int) * n);
    cl::Buffer buff_j(context, CL_MEM_READ_WRITE, sizeof(int) * n);
    cl::Buffer buff_off(context, CL_MEM_READ_WRITE, sizeof(int) * n);
    cl::Buffer buff_n(context, CL_MEM_READ_WRITE, sizeof(int));
    //Checker si on pourrait pas utiliser moins de buffer ?

    int* Itt = (int*)calloc(n, sizeof(int));
    int* Off = (int*)calloc(n * n, sizeof(int));
    for (int k = 0; k < n; k++) {
        Itt[k] = k;
        Off[k] = k * n + k;
    }

    cl::CommandQueue queue(context, default_device);

    queue.enqueueWriteBuffer(buff_M, CL_TRUE,0, n * n * sizeof(int*), matr_adj);
    queue.enqueueWriteBuffer(buff_i, CL_TRUE, 0, n * sizeof(int*), Itt);
    queue.enqueueWriteBuffer(buff_j, CL_TRUE, 0, n * sizeof(int*), Itt);
    queue.enqueueWriteBuffer(buff_off, CL_TRUE, 0, n * sizeof(int*), Off);
    queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);

    cl::Kernel generate(program, "generate");
    generate.setArg(0, buff_M);
    generate.setArg(1, buff_i);
    generate.setArg(2, buff_j);
    generate.setArg(3, buff_off);
    generate.setArg(3, buff_n);

    queue.enqueueNDRangeKernel(generate, cl::NullRange, cl::NDRange(n * n), cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);


    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            offset = i * n + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
    
}
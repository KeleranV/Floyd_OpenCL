#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#include <CL/cl.hpp>
#include <vector>



int * matr_adj;


void init(int n){
    matr_adj = (int *)calloc(n*n, sizeof(int*));
    //on va utiliser un offset : i*n + j pour naviguer sur m(i, j)
    int i;
    int j;
    int offset;
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


void main(){


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


    int debug = 10;
    init(debug);
    int offset;


    for(int i = 0; i < debug; i++){
        for(int j = 0; j < debug; j++){
            offset = i * debug + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
    
}
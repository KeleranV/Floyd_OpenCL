#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>


#include <CL/cl.hpp>

#include <vector>



int * matr_adj;
int n = 30;
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
    int* matr_k = (int*)calloc(n * n, sizeof(int*));
    matr_adj[n - 1] = 1;
    matr_adj[n * (n - 1)] = 1;

    //Test initial pour v�rifier que le PC est compatible avec OpenCL
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
        "void kernel generate(global int* M, global const int* n){"
        "   __private int o;"
        "   __private int i = get_global_id(0);"
        "   __private int j;"
        "   for(j = 0; j < *n; j++)"
        "       {"
        "       o = i * (*n) + j;"
        "       if (j == i)"
        "       {"
        "           M[o] = 0;"
        "           continue;"
        "       }"
        "       if (i < *n - 1)"
        "       {"
        "           M[o] = 1;"
        "           continue;"
        "       }"
        "       M[o] = *n + 1; "
        "   }"
        "}";

    kernel_code +=
        "void kernel floyd_1_itt(global int* M, global int* n, global int * Mk)"
        "{"
        "   __private int k;"
        "   __private int o = get_global_id(0);"
        "   __private int op1;"
        "   __private int op2;"
        "   __private int i = o / *n;"
        "   __private int j = o % *n;"
        "   for(k = 0; k < *n; k++)"
        "   {"
        "       op1 = i * (*n) + k;"
        "       op2 = k * (*n) + j;"
        "       if(M[o] < M[op1] + M[op2])"  
        "       {"
        "           Mk[o] = M[o];"
        "       }"
        "       else"
        "       {"
        "           Mk[o] = M[op1] + M[op2];"
        "       }"
        "   }"
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


    //Cr�ation des buffers
    cl::Buffer buff_M(context, CL_MEM_READ_WRITE, n * n * sizeof(int*));
    cl::Buffer buff_n(context, CL_MEM_READ_WRITE, sizeof(int));
    cl::Buffer buff_Mk(context, CL_MEM_READ_WRITE, n * n * sizeof(int*));
    //Checker si on pourrait pas utiliser moins de buffer ?



    cl::CommandQueue queue(context, default_device);

    queue.enqueueWriteBuffer(buff_M, CL_TRUE,0, n * n * sizeof(int*), matr_adj);
    queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);

    cl::Kernel generate(program, "generate");
    generate.setArg(0, buff_M);
    generate.setArg(1, buff_n);

    queue.enqueueNDRangeKernel(generate, cl::NullRange, cl::NDRange(n), cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
    queue.finish();


    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            offset = i * n + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;


    //Utilisation de la version classique de l'algo de Floyd
    //On devra le faire n fois
    

    //proto : floyd_1_itt(global int* M, global int* n, global int * Mk)

    cl::Kernel floyd_1_itt(program, "floyd_1_itt");

    for (int k = 0; k < n; k++) {
        queue.enqueueWriteBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
        queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);
        queue.enqueueWriteBuffer(buff_Mk, CL_TRUE, 0, n * n * sizeof(int*), matr_k);
        floyd_1_itt.setArg(0, buff_M);
        floyd_1_itt.setArg(1, buff_n);
        floyd_1_itt.setArg(2, buff_Mk);
        queue.enqueueNDRangeKernel(floyd_1_itt, cl::NullRange, cl::NDRange(n * n), cl::NullRange);
        queue.finish();

        queue.enqueueReadBuffer(buff_Mk, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
        queue.finish();

        
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            offset = i * n + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <chrono>

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

void main() {

    std::cin >> n;

    //Pas besoin de parraléliser cette attribution
    matr_adj = (int*)calloc(n * n, sizeof(int*));
    int* matr_k = (int*)calloc(n * n, sizeof(int*));
    //matr_adj[n - 1] = 1;
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


    //Creation des buffers
    cl::Buffer buff_M(context, CL_MEM_READ_WRITE, n * n * sizeof(int*));
    cl::Buffer buff_n(context, CL_MEM_READ_WRITE, sizeof(int));
    cl::Buffer buff_Mk(context, CL_MEM_READ_WRITE, n * n * sizeof(int*));
    cl::Buffer buff_k(context, CL_MEM_READ_WRITE, sizeof(int));


    cl::Program::Sources init_sources;


    std::string init_kernel =
        "void kernel init(global int* M, global int* n){"
        "   __private int o;"
        "   __private int i = get_global_id(0);"
        "   __private int j = get_global_id(1);"
        "   o = i * (*n) + j;"
        "   M[o] = *n + 1; "
        "}";

    init_sources.push_back({init_kernel.c_str(), init_kernel.length()});

    cl::Program program_init(context, init_sources);
    if (program_init.build({ default_device }) != CL_SUCCESS) {
        std::cout << " Error building: " << program_init.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }

    cl::CommandQueue queue(context, default_device);

    queue.enqueueWriteBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
    queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);

    cl::Kernel init(program_init, "init");
    init.setArg(0, buff_M);
    init.setArg(1, buff_n);

    queue.enqueueNDRangeKernel(init, cl::NullRange, cl::NDRange(n, n), cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
    queue.finish();





    cl::Program::Sources sources;

    /*std::string kernel_code =
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
        "}";*/
    std::string kernel_code =
        "void kernel generate(global int* M, global const int* n){"
        "   __private int o;"
        "   __private int i = get_global_id(0);"
        "   __private int j = get_global_id(1);"
        "   o = i * (*n) + j;"
        "   if(i == j)"
        "   {"
        "       M[o] = 0;"
        "       return;"      
        "   }"
        "   if(i < (*n) - 1)"
        "   {"
        "       o = i * (*n) + i + 1;"
        "       M[o] = 1;"
        "       return;"
        "   }"
        "   else"
        "   {"
        "       M[o] = *n + 1; "
        "   }"
        "}";


    sources.push_back({ kernel_code.c_str(), kernel_code.length() });

    cl::Program program(context, sources);
    if (program.build({ default_device }) != CL_SUCCESS) {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }




    queue.enqueueWriteBuffer(buff_M, CL_TRUE,0, n * n * sizeof(int*), matr_adj);
    queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);

    cl::Kernel generate(program, "generate");
    generate.setArg(0, buff_M);
    generate.setArg(1, buff_n);

    queue.enqueueNDRangeKernel(generate, cl::NullRange, cl::NDRange(n, n), cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
    queue.finish();

    /*
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            offset = i * n + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    */

    //Utilisation de la version classique de l'algo de Floyd
    //On devra le faire n fois
    std::string kernel_code_floyd =
        "void kernel floyd_1_itt(global read_only int* M, global read_only int* n, global write_only int * Mk, global read_only int* k)"
        "{"
        "   __private int op1;"
        "   __private int op2;"
        "   __private int i = get_global_id(0);"
        "   __private int j = get_global_id(1);"
        "   __private int o = i * (*n) + j;"
        "   op1 = i * (*n) + (*k);"
        "   op2 = (*k) * (*n) + j;"
        "   if(M[o] < M[op1] + M[op2])"
        "   {"
        "       Mk[o] = M[o];"
        "   }"
        "   else"
        "   {"
        "       Mk[o] = M[op1] + M[op2];"
        "   }"
        "}";
    cl::Program::Sources sources_floyd;

    sources_floyd.push_back({ kernel_code_floyd.c_str(), kernel_code_floyd.length() });

    cl::Program program_floyd(context, sources_floyd);
    if (program_floyd.build({ default_device }) != CL_SUCCESS) {
        std::cout << " Error building: " << program_floyd.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }


    cl::Kernel floyd_1_itt(program_floyd, "floyd_1_itt");

    //proto : floyd_1_itt(global read_only int* M, global read_only int* n, global write_only int * Mk, global read_only int* k)


    
    int k;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (k = 0; k < n; k++) {
        queue.enqueueWriteBuffer(buff_M, CL_TRUE, 0, n * n * sizeof(int*), matr_adj);
        queue.enqueueWriteBuffer(buff_n, CL_TRUE, 0, sizeof(int), &n);
        queue.enqueueWriteBuffer(buff_Mk, CL_TRUE, 0, n * n * sizeof(int*), matr_k);
        queue.enqueueWriteBuffer(buff_k, CL_TRUE, 0, sizeof(int), &k);
        floyd_1_itt.setArg(0, buff_M);
        floyd_1_itt.setArg(1, buff_n);
        floyd_1_itt.setArg(2, buff_Mk);
        floyd_1_itt.setArg(3, buff_k);
        queue.enqueueNDRangeKernel(floyd_1_itt, cl::NullRange, cl::NDRange(n, n), cl::NullRange);
        queue.finish();

        queue.enqueueReadBuffer(buff_Mk, CL_TRUE, 0, n * n * sizeof(int*), matr_k);
        queue.finish();
        memcpy(matr_adj, matr_k, n * n * sizeof(int*));

        
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << duration.count() << std::endl;
    /*
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            offset = i * n + j;
            std::cout << matr_adj[offset] << " ";
        }
        std::cout << std::endl;
    }
    */
    free(matr_adj);
    free(matr_k);
}
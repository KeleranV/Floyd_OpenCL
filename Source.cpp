#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#include <CL/cl.h>



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
#include "task1.h"


#define DEBUG 1
#define DEEP_DEBUG 0

/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

static long globalN;

//static double** matrixC;

// TODO: Check if this works.
double inline randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

void inline fillNOverNmatrixWith0(double** matrix, int n){
    for(int y = 0; y < n; y++){
        for(int x = 0; x < n; x++){
            matrix[y][x] = 0;
        }
    }
}

void inline fillNOverPMatrixWithRandomValue(double** matrix, int n, int p){
    for(int y = 0; y < n; y++){
        for(int x = 0; x < n; x++){
            matrix[y][x] = randfrom(0,1000); // TODO: Use ~MAXDDOUBLE
        }
    }
}





/**
 * This method will run on every machine.
 */
void bspEntrance(){

    bsp_begin(numberOfProcessors);

    // Distribution between processors.
    long p= bsp_nprocs();
    long s= bsp_pid();
    long n = globalN;
    bsp_push_reg(&n,sizeof(long));
    bsp_sync();

    bsp_get(0,&n,0,&n,sizeof(long));
    bsp_sync();
    bsp_pop_reg(&n);

    int start = n/numberOfProcessors * s;
    int end = n/p * (s+1);
    int k = start;
    int nrows = end - start;

    // Matrix init
    double* pointerA = (double*) malloc(sizeof(double)*n*nrows);
    double* pointerB = (double*) malloc(sizeof(double)*n*nrows);
    double* pointerC = (double*) malloc(sizeof(double)*n*nrows);
    double** matrixA = (double**) malloc(sizeof(double*) * nrows);
    double** matrixB = (double**) malloc(sizeof(double*) * nrows);
    double** localMatrixC = (double**) malloc(sizeof(double*) * nrows);

    for(int i = 0; i < nrows; i++){
        matrixA[i] = pointerA + i*n;
        matrixB[i] = pointerB + i*n;
        localMatrixC[i] = pointerC + i*n;
    }

    // TODO: Random fill.

    bsp_push_reg(pointerB,n*nrows*sizeof(double));
    bsp_sync();

    if(DEBUG) printf("...Matrix init done for s=%ld\n",s);

    double timeStart= bsp_time();
    do {
        for(int i = 0; i < nrows;i++){
            for(int j = 0; j < n; j++){
                for(int h = k; h < k + n/p;h++){ // h or k
                    if(DEEP_DEBUG){
                        printf("i=%d,j=%d,h=%d,k=%d for s=%ld\n",i,j,h,k,s);
                    }
                    localMatrixC[i][j] += matrixA[i][h] * matrixB[h-k][j];
                }
            }
        }
        k = (k + n / numberOfProcessors) % n;
        if(DEBUG) printf("Start distribution k=%d for s=%ld...\n",k,s);
        bsp_get((s+1)%p,pointerB,0,pointerB,n*nrows*sizeof(double));
        bsp_sync();
        if(DEBUG) printf("...distribution k=%d for s=%ld done...\n",k,s);
    } while(k != start);

    
    double timeEnd= bsp_time();
    if(DEBUG) printf("...calculations done for s=%ld\n",s);

    if(s==0){
        printf("Calculations done in %.6lf seconds\n",timeEnd-timeStart);
    }
    // TODO: Make it possible to access the (i,j) cell and matching row
    // and colum.

    /*free(matrixA);
    free(matrixB);
    free(localMatrixC);
    free(pointerA);
    free(pointerB);
    free(pointerC);*/
    bsp_end();
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 1;
    globalN = 10;
    
    //double** matrixC = NULL;
    //initMatrix(matrixC, globalN);
    // TODO: Check how to distribute the result matrix.
    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }
    if(DEBUG) printf("Start program with n=%ld,p=%d\n",globalN,numberOfProcessors);
    bspEntrance();

    exit(EXIT_SUCCESS);
}
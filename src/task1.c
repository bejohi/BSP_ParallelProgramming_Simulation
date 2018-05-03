#include "task1.h"


#define DEBUG 1
#define DEEP_DEBUG 1

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

void inline initMatrix(double** matrix, int n){
    matrix = (double**) malloc(sizeof(double*) * n);
    if(!matrix){
        bsp_abort("Not enough memory for allocating the matrix available\n");
    }
    for(long i = 0; i < n; i++){
        matrix[i] = (double*) malloc(sizeof(double) * n);
        if(!matrix[i]){
            bsp_abort("Not enough memory for allocating the matrix available\n");
        }
    }
}

void inline freeMatrix(double** matrix, int n){
    for(long i = 0; i < n; i++){
        free(matrix[i]);
    }
    free(matrix);
}

void inline initNOverPMatrix(double** matrix, int n, int p){
    matrix = (double**) malloc(sizeof(double*) * n);
    long s= bsp_pid();
    
    if(!matrix){
        bsp_abort("Not enough memory for allocating the matrix available\n");
    }
    if(DEEP_DEBUG)printf("little malloc done for s=%d\n",s);
    for(int i = 0; i < p; i++){
        matrix[i] = (double*) malloc(sizeof(double) * n);

        if(!matrix[i]){
            bsp_abort("Not enough memory for allocating the matrix available\n");
        }
        if(DEEP_DEBUG)printf("big malloc [%d] done for s=%d\n",i,s);
    }
}

void inline freeNOverPMatrix(double** matrix, int p){
    for(long i = 0; i < p; i++){
        free(matrix[i]);
    }
    free(matrix);
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

    long p= bsp_nprocs();
    long s= bsp_pid();
    long n = globalN;
    bsp_push_reg(&n,sizeof(long));
    bsp_sync();

    if(DEBUG) printf("p,s,n calc done for s=%d\n",s);

    bsp_get(0,&n,0,&n,sizeof(long));
    bsp_sync();
    bsp_pop_reg(&n);
    //bsp_sync();

    if(DEBUG) printf("n distribution done for s=%d\n",s);

    // TODO: nloc?
    // TODO: Distribute globalN as local n over the processors.

    int start = n/numberOfProcessors * s;
    int end = n/p * (s+1);
    int k = start;
    int nrows = end - start;

    // TODO: Don't init sqare matrix but n*nrows!
    double** matrixA = NULL;
    double** matrixB = NULL;
    double** localMatrixC = NULL;

    if(DEBUG) printf("n=%d, p=%d, nrows=%d for s=%d...\n",n,p,nrows,s);

    if(DEBUG) printf("Init matrix A for s=%d...\n",s);
    initNOverPMatrix(matrixA,n,nrows);

    if(DEBUG) printf("Init matrix B for s=%d...\n",s);
    initNOverPMatrix(matrixB,n,nrows);

    if(DEBUG) printf("Init matrix C for s=%d...\n",s);
    initMatrix(localMatrixC,n);

    if(DEBUG) printf("Create pointerB for s=%d...\n",s);
    if(DEBUG){
        if(!matrixB){
            printf("Problem 1 with b\n");
        } else if(!matrixB[0]){
            printf("Problem 2 with b\n");
        }
        
    }
    double* pointerB = matrixB[0];

    if(DEBUG) printf("Random fill matrix C for s=%d, n=%d...\n",s);
    fillNOverNmatrixWith0(localMatrixC,n);

    if(DEBUG) printf("Random fill matrix A for s=%d...\n",s);
    fillNOverPMatrixWithRandomValue(matrixA,n,nrows);

    if(DEBUG) printf("Random fill matrix B for s=%d...\n",s);
    fillNOverPMatrixWithRandomValue(matrixB,n,nrows);

    if(DEBUG) printf("...Matrix init done for s=%d\n",s);

    do {
        for(int i = nrows; i < n;i++){
            for(int j = 0; j < n; j++){
                for(int h = k; h < k + n/p;h++){ // h or k
                    localMatrixC[i][j] += matrixA[i][h] * matrixB[h][j];
                }
            }
        }
        k = (k + n / numberOfProcessors) % n;
        bsp_get((s+1)%p,pointerB,0,pointerB,n*nrows*sizeof(double));
        bsp_sync();
        // TODO: Shift the matrices.
    } while(k != start);
    if(DEBUG) printf("...calculations done for s=%d\n",s);

    // TODO: Make it possible to access the (i,j) cell and matching row
    // and colum.

    freeNOverPMatrix(matrixA,nrows);
    freeNOverPMatrix(matrixB,nrows);
    freeMatrix(localMatrixC, n);
    bsp_end();
    bsp_sync(); // Should we sync here?
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 2;
    globalN = 10;
    
    //double** matrixC = NULL;
    //initMatrix(matrixC, globalN);
    // TODO: Check how to distribute the result matrix.
    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }
    if(DEBUG) printf("Start program with n=%d,p=%d\n",globalN,numberOfProcessors);
    bspEntrance();

    exit(EXIT_SUCCESS);
}
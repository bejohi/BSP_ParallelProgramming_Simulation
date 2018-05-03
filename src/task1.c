#include "task1.h"


/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

static long globalN;

static double** matrixC;

// TODO: Check if this works.
double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

void initMatrix(double** matrix, int n){
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

void freeMatrix(double** matrix, int n){
    for(long i = 0; i < n; i++){
        free(matrix[i]);
    }
    free(matrix);
}

void initNOverPMatrix(double** matrix, int n, int p){
    matrix = (double**) malloc(sizeof(double*) * globalN);
    if(!matrix){
        bsp_abort("Not enough memory for allocating the matrix available\n");
    }
    for(int i = 0; i < p; i++){
        matrix[i] = (double*) malloc(sizeof(double) * globalN);
        if(!matrix[i]){
            bsp_abort("Not enough memory for allocating the matrix available\n");
        }
    }
}

void freeNOverPMatrix(double** matrix, int p){
    for(long i = 0; i < p; i++){
        free(matrix[i]);
    }
    free(matrix);
}

void fillNOverNmatrixWith0(double** matrix, int n){
    for(int y = 0; y < n; y++){
        for(int x = 0; x < n; x++){
            matrix[y][x] = 0;
        }
    }
}

void fillNOverPMatrixWithRandomValue(double** matrix, int n, int p){
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

    bsp_push_reg(&globalN,sizeof(long));
    bsp_sync();

    bsp_get(0,&globalN,0,&globalN,sizeof(long));
    bsp_sync();
    bsp_pop_reg(&globalN);
    bsp_sync();

    // TODO: nloc?
    // TODO: Distribute globalN as local n over the processors.

    int start = globalN/numberOfProcessors * s;
    int end = globalN/p * (s+1);
    int k = start;
    int nrows = end - start;

    // TODO: Don't init sqare matrix but n*nrows!
    double** matrixA = NULL;
    double** matrixB = NULL;
    double** localMatrixC = NULL;
    initNOverPMatrix(matrixA,globalN,p);
    
    initNOverPMatrix(matrixB,globalN,p);
    initMatrix(localMatrixC, globalN);
    fillNOverNmatrixWith0(localMatrixC,globalN);
    fillNOverPMatrixWithRandomValue(matrixA,globalN,p);
    fillNOverPMatrixWithRandomValue(matrixB,globalN,p);

    do {
        for(int i = nrows; i < globalN;i++){
            for(int j = 0; j < globalN; j++){
                for(int h = k; h < k + globalN/p;h++){ // h or k
                    localMatrixC[i][j] += matrixA[i][h] * matrixB[h][j];
                }
            }
        }
        k = (k + globalN / numberOfProcessors) % globalN;
        // TODO: Shift the matrices.
    } while(k != start);


    freeNOverPMatrix(matrixA,p);
    freeNOverPMatrix(matrixB,p);
    freeMatrix(localMatrixC, globalN);
    bsp_end();
    bsp_sync(); // Should we sync here?
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 36;
    globalN = 10;
    double** matrixC = NULL;
    initMatrix(matrixC, globalN);
    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }

    bspEntrance();

    exit(EXIT_SUCCESS);
}
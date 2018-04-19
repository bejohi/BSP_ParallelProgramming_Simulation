#include "task1.h"


/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

static long globalN;

static double** matrixC;

void initMatrix(double** matrix){
    matrix = (double**) malloc(sizeof(double*) * globalN);
    if(!matrix){
        bsp_abort("Not enough memory for allocating the matrix available\n");
    }
    for(long i = 0; i < globalN; i++){
        matrix[i] = (double*) malloc(sizeof(double) * globalN);
        if(!matrix[i]){
            bsp_abort("Not enough memory for allocating the matrix available\n");
        }
    }
}

void freeMatrix(double** matrix){
    for(long i = 0; i < globalN; i++){
        free(matrix[i]);
    }
    free(matrix);
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

    int start = globalN/numberOfProcessors * s;
    int k = start;
    int nrows = 0; // ???

    double** matrixA = NULL;
    double** matrixB = NULL;
    double** localMatrixC = NULL;
    initMatrix(matrixA);
    initMatrix(matrixB);
    initMatrix(localMatrixC);

    do {
        for(int i = nrows; i < globalN;i++){
            for(int j = 0; j < globalN; j++){
                for(int h = k; k < 0;k++){ // ???
                    localMatrixC[i][j] += matrixA[i][h] * matrixB[h][j];
                }
            }
        }
        //k = ; ???
    } while(k != start);


    freeMatrix(matrixA);
    freeMatrix(matrixB);
    freeMatrix(localMatrixC);
    bsp_end();
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 36;
    double** matrixC = NULL;
    initMatrix(matrixC);
    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }

    bspEntrance();

    exit(EXIT_SUCCESS);
}
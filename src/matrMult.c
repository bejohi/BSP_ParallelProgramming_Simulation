#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{

    srand(time(NULL));
    time_t start,stop;
    double cur_time;
    double **A,**B,**C;
    int i,j,k;

    int n=1800;

    double *p1 = (double *) malloc(n*n*sizeof(double));
    double *p2 = (double *) malloc(n*n*sizeof(double));
    double *p3 = (double *) malloc(n*n*sizeof(double));

    A = (double **) malloc(n*sizeof(double*));
    B = (double **) malloc(n*sizeof(double*));
    C = (double **) malloc(n*sizeof(double*));

    for (i=0;i<n;i++) {
      A[i] = &(p1[n*i]);
      B[i] = &(p2[n*i]);
      C[i] = &(p3[n*i]);
    }

    for (i=0;i<n;i++) {
      for (j=0;j<n;j++) {
        A[i][j] = (double)rand()/(double)(RAND_MAX);
        B[i][j] = (double)rand()/(double)(RAND_MAX);
      }
    }

    start = clock();
    for (i=0;i<n;i++) 
      for (j=0;j<n;j++) 
        C[i][j] = 0.0;

    for (i=0;i<n;i++) {
      for (k=0;k<n;k++) {
        for (j=0;j<n;j++) {
          C[i][j] += A[i][k]*B[k][j];
        }
      }
    }

    stop = clock();
    cur_time = ((double) stop-start) / CLOCKS_PER_SEC;

    printf("Sequential code took %lf seconds.\n", cur_time);
    printf("C element %lf .\n", C[0][0]);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Matrices dimensions
int a,b,c,d;
// Matrices
int **mat1, **mat2, **matR1, **matR2;

// Structure to use it for sending multiple arguments to the thread function
struct threadData
{
    int row;    // Number of row to be produced in MatrixResult
    int col;    // Number of column to be produced in MatrixResult (neglected in case of 2nd Algorithm)
};

// First Algorithm
void* multiplyElement(void* x)
{
    struct threadData *data;
    data = (struct threadData*)x;
    matR1[data->row][data->col] = 0;
    // Compute elemnt at [row,column]
    for(int k = 0; k < c; k ++)
    {
        matR1[data->row][data->col] += mat1[data->row][k] * mat2[k][data->col];
    }
    return NULL;
}

//Second Algorithm
void* multiplyRow(void* x)
{
    struct threadData *data;
    data = (struct threadData*)x;
    // Compute elements of row [row]
    for(int i = 0; i < d; i ++)
    {
        matR2[data->row][i] = 0;
        // Compute element at [row,column]
        for(int j = 0; j < c; j ++)
        {
            matR2[data->row][i] += mat1[data->row][j] * mat2[j][i];
        }
    }
    return NULL;
}

void readMatrixFile(char *filename)
{
    FILE *f;
    f = fopen(filename, "r");
    if(f == NULL)
    {
        // Don't procceed if error occurred while opening the file
        printf("Can't open file: %s\n", filename);
        exit(-1);
    }
    else
    {
        // Reading both matrices size and content from the file whose name is entered by the user
        fscanf(f,"%d %d",&a,&b);
	// Reserve sufficient stroage for Matrix1 (of size axb) and store its elements in it
	mat1 = (int**)malloc(a*sizeof(int*));
	for(int i = 0; i < a; i++)
	{
	    mat1[i] = (int*)malloc(b*sizeof(int));
	    for(int j = 0; j < b; j ++)
	    {
	        fscanf(f,"%d",&mat1[i][j]);
	    }
	}
	fscanf(f,"%d %d",&c,&d);
	// Reserve sufficient stroage for Matrix2 (of size cxd) and store its elements in it
	mat2 = (int**)malloc(c*sizeof(int*));
	for(int i = 0; i < c; i++)
	{
	    mat2[i] = (int*)malloc(d*sizeof(int));
	    for(int j = 0; j < d; j ++)
	    {
	        fscanf(f,"%d",&mat2[i][j]);
	    }
	}
        fclose(f);
        printf("Finished reading.\n");
    }
}

void printInMatrix()
{
    // Print content of both matrices after reading them from the file
    printf("Size of matrix 1 = %d x %d\n",a,b);
    for(int i = 0; i < a; i++)
    {
        for(int j = 0; j < b; j ++)
        {
            printf("%d ", mat1[i][j]);
        }
        printf("\n");
    }
    printf("Size of matrix 2 = %d x %d\n",c,d);
    for(int i = 0; i < c; i++)
    {
        for(int j = 0; j < d; j ++)
        {
            printf("%d ",mat2[i][j]);
        }
        printf("\n");
    }
}

void printOutMatrix()
{
    // Print content of result matrix for both algorithms
    printf("Size of matrix out 1 = %d x %d\n",a,d);
    for(int i = 0; i < a; i++)
    {
        for(int j = 0; j < d; j ++)
        {
            printf("%d ", matR1[i][j]);
        }
        printf("\n");
    }
    printf("Size of matrix out 2 = %d x %d\n",a,d);
    for(int i = 0; i < a; i++)
    {
        for(int j = 0; j < d; j ++)
        {
            printf("%d ",matR2[i][j]);
        }
        printf("\n");
    }
}

void reserveOut()
{
    // Reserve sufficient stroage for MatrixResult1 (of size axd)
    matR1 = (int**)malloc(a*sizeof(int*));
    for(int i = 0; i < a; i ++)
    {
        matR1[i] = (int*)malloc(d*sizeof(int));
    }
    // Reserve sufficient stroage for MatrixResult2 (of size axd)
    matR2 = (int**)malloc(a*sizeof(int*));
    for(int i = 0; i < a; i ++)
    {
        matR2[i] = (int*)malloc(d*sizeof(int));
    }
}

void freeStorage()
{
    for(int i = 0; i < a; i ++)
    {
        free(mat1[i]);
        free(matR1[i]);
        free(matR2[i]);
    }
    free(mat1);
    free(matR1);
    free(matR2);
    for(int i = 0; i < c; i ++)
    {
        free(mat2[i]);
    }
    free(mat2);
}
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
    	printf("Inproper number of arguments.\n");
    	exit(-1);
    }
    readMatrixFile(argv[1]);
    if(b != c)
    {
    	printf("Can't multiply these 2 matrices.\n");
    	exit(-1);
    }
    printInMatrix();
    reserveOut();
    // Threads
    pthread_t id1[a][d];
    pthread_t id2[a];
    int start1, end1, start2, end2, time1, time2;
    // Applying first algorithm
    start1 = clock();
    for(int i = 0; i < a; i ++)
    {
        for(int j = 0; j < d; j ++)
        {
	    struct threadData *x1 = (struct threadData*)malloc(sizeof(struct threadData));
            x1->row = i;
            x1->col = j;
            // Create thread for every column with every row (axd threads)
            pthread_create(&id1[i][j], NULL, multiplyElement, (void*)x1);
        }
    }
    for(int i = 0; i < a; i ++)
    {
        for(int j = 0; j < d; j ++)
        {
            pthread_join(id1[i][j],NULL);
        }
    }
    end1 = clock();
    // Applying second algorithm
    start2 = clock();
    for(int i = 0; i < a; i ++)
    {
        struct threadData *x2 = (struct threadData*)malloc(sizeof(struct threadData));
        x2->row = i;
        // Create thread for every row (a threads)
        pthread_create(&id2[i], NULL, multiplyRow, (void*)x2);
    }
    for(int i = 0; i < a; i ++)
    {
        pthread_join(id2[i],NULL);
    }
    end2 = clock();
    time1 = end1 - start1;
    time2 = end2 - start2;
    // Reporting time taken by each method as well as results of both algorithms
    printf("Time of first algorithm [element calculated in thread] is %f sec\n", (float)(time1)/CLOCKS_PER_SEC);
    printf("Time of second algorithm [row calculated in thread] is %f sec\n", (float)(time2)/CLOCKS_PER_SEC);
    printOutMatrix();
    freeStorage();
    // Conclusion
    printf("Time taken by second method is smaller than that taken by the first one; because the number of threads created in second one is the number of rows of resulted matrix (n/c for n elements and c columns) while in first one is the total number of elements (n for n elements).\n");
    return 0;
}

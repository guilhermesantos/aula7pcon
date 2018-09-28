#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"mpi.h"
#define N 3

/*
INTEGRANTES DO GRUPO:
GUILHERME MILAN SANTOS 9012966
LEONARDO DE ALMEIDA LIMA ZANGUETIN 8531866
LUCAS CARDOSO LEMOS 9064342

OBS: Compilar o slave com o nome aula7_slave

*/


void ordena_colunas(int *, int, int);
void calcula_mediana(int *, float *, int, int);
void quicksort(int *, int, int, int);
int partition (int *, int, int, int);

//O slave recebe por broadcast as dimensoes da matriz
//Em seguida, por meio de um scatter, recebe as colunas das quais serao calculadas as medianas
//Ele ordena as colunas e calcula as medianas, e em seguida transmite os resultados
//de volta ao root por meio de um gather
int main(int argc, char **argv) {
	//Inicia o MPI
	MPI_Init(&argc, &argv);
	printf("slave\n");
    int  tag = 1, my_rank, num_proc;
    char msg_0[50], msg_1[]="hi master", master_data[50], slaves_data[50];
	int  i, j, vet[N], buf_rcv, src, dst, root;
    MPI_Status  status;
    MPI_Comm    inter_comm; 
	
	//Coleta informacoes do comunicador
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
   
   	//Acha o intercomm
    int rec_buff[2]; 
    MPI_Comm_get_parent(&inter_comm);

    //O slave recebe as dimensoes totais da matriz p/ saber quantos
    //elementos serao lidos das operacoes subsequentes
    MPI_Bcast(rec_buff, 2, MPI_INT, 0, inter_comm);
    printf("slave linhas E colunas %d e %d\n", rec_buff[0], rec_buff[1]);
    int L = rec_buff[0];
    int C = rec_buff[1];
    int partition_size = L;
    printf("slave partition size: %d\n", partition_size);


    int *matriz_rec_buf = malloc(sizeof(int)*partition_size);
    int M[1];
    //Recebe chunk do master
 	MPI_Scatter(NULL, partition_size, MPI_INT, matriz_rec_buf, partition_size, MPI_INT, 0, inter_comm);

 	for(i=0; i < partition_size; i++) { 		
		printf("slave %d [%d]=%d\n", my_rank, i, matriz_rec_buf[i]);
 		printf("\n");
 	}

 	//Nas linhas a seguir, o slave ordenaria as suas colunas e entao calcularia a mediana
 	//Infelizmente, nao deu tempo de fazer essa parte funcionar, entao foi deixado comentado
 	//a fim de explicar a logica pretendida pelo grupo

 	/*
 	ordena_colunas(matriz_rec_buf, 4, 3);
 	
 	float *medianas = malloc(sizeof(float));//1 mediana so, para o caso de teste informado
 											//para aula
 	calcula_mediana(M, medianas,4, 1);

 	printf("slave %d mediana %f\n", my_rank, medianas[0]);
 	//Manda mediana de volta pro root
 	float med_rec_buf[1]; 

 	//Apos calcular as medianas, o root faria um gather para coletar os resultados
 	MPI_Gather(medianas, 1, MPI_FLOAT, med_rec_buf, 1, MPI_FLOAT, 0, inter_comm);
	*/

 	free(matriz_rec_buf);

 	MPI_Finalize();
	exit(0);
}
/* low  --> Starting index,  high  --> Ending index */
//https://www.geeksforgeeks.org/quick-sort/
void quicksort(int *arr, int low, int high, int C)
{
    int pi;
    
    if (low < high)  {
        /* pi is partitioning index, arr[pi] is now
           at right place */
        pi = partition(arr, low, high, C);

        quicksort(arr, low, pi - 1, C);  // Before pi
        quicksort(arr, pi + 1, high, C); // After pi
    }
    
} // fim quicksort

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot 
   https://www.geeksforgeeks.org/quick-sort/
*/
int partition (int *arr, int low, int high, int C)
{
    int i, j, swap, pivot;
    
    // pivot (Element to be placed at right position)
    pivot = arr[high*C];  
 
    i = (low - 1);  // Index of smaller element

    for (j = low; j <= high-1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j*C] <= pivot)
        {
            i++;    // increment index of smaller element
            
            // swap arr[i] and arr[j]
            swap = arr[i*C];
	    arr[i*C] = arr[j*C];
	    arr[j*C] = swap;
        }
    }
    
    //swap arr[i + 1] and arr[high]
    swap = arr[(i + 1)*C];
    arr[(i + 1)*C] = arr[high*C];
    arr[high*C] = swap;
    
    return (i + 1);
  
} // fim partition



void ordena_colunas(int *M, int L, int C) 
{
  int j;
  
  for (j = 0; j < C; j++) {
      //manda o endereco do primeiro elemento da coluna, limites inf e sup e a largura da matriz
      quicksort(&M[j], 0, L-1, C);
  }
} // fim ordena_colunas


void calcula_mediana(int *M, float *vet, int L, int C) 
{  
  int j;
  
//  printf("calcula_mediana: L=%d e C=%d \n", L, C);
  
  for (j = 0; j < C; j++) {
    vet[j] = M[((L/2)*C)+j];
//    printf("vet[%d] = %f, pos = %d, M[]=%d, ", j, vet[j], (L/2)*C+j, M[((L/2)*C)+j]);
    // se o nr de elementos eh par, tem que fazer a media
    // das duas medianas. Tem que pegar a mediana anteior.
    if(!(L%2))  {
      vet[j]+=M[((((L-1)/2)*C)+j)];
//    printf("vet[%d] = %f, pos = %d, M[]=%d, \n", j, vet[j], (((L-1)/2)*C)+j, M[((((L-1)/2)*C)+j)]);   
      vet[j]*=0.5;
    } // fim do if 
  } // fim do for
  
  return;
} // fim calcula_mediana
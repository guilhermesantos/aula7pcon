

/*
INTEGRANTES DO GRUPO:
GUILHERME MILAN SANTOS 9012966
LEONARDO DE ALMEIDA LIMA ZANGUETIN 8531866
LUCAS CARDOSO LEMOS 9064342

OBS: Compilar o slave com o nome aula7_slave

O programa inicia com o processo master lendo os dados de entrada do arquivo entrada.txt. Em seguida, ele transmite
para cada slave as dimensoes da matriz, para que saibam quantos elementos devem ser recebidos nas operacoes
subsequentes.

O maste entao envia os elementos para cada slave por meio de um scatter. Os slaves computam as medianas
e enviam de volta para o master por meio de um gather.

Particionamento: A matriz lida do arquivo entrada.txt é particionada em suas células.
Comunicação: A tarefa inicial transmite para as demais tarefas as células necessárias em suas computações. Após o côḿputo das
medianas, as tarefas comunicam seus resultados para a tarefa inicial.
Aglomeração: Cada tarefa pode lidar com uma coluna inteira da matriz ao invés de receber células individuais.
Mapeamento: Cada processo recebe uma coluna inteira da matriz para computar a mediana.
*/


// Based on Author: Paulo S L Souza 
// quick sort code adapted from https://www.geeksforgeeks.org/quick-sort/


#include<stdio.h>
#include<stdlib.h>
#include <mpi.h>


void ordena_colunas(int *, int, int);
void calcula_mediana(int *, float *, int, int);
void quicksort(int *, int, int, int);
int partition (int *, int, int, int);
int master();
void slave(int my_rank);

#define N 3

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



int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  printf("master\n");
  //Inicializa MPI
   int i,j;
  int my_rank;
  int npes; 
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);

  int L, C;
  int *M;
  float *vet;

  if(my_rank == 0) {
    //MASTER
      //Le do arquivos
      FILE *arquivo_entrada,*arquivo_saida;

      if(!(arquivo_entrada=fopen("entrada.txt","r"))) 
      {
        printf("Erro ao abrir entrada.txt como leitura! Saindo! \n");
        return(-1);
      }

      if(!(arquivo_saida=fopen("saida.txt","w+")))  
      {
        printf("Erro ao abrir/criar saida.txt como escrita. Saindo! \n");
        return(-1);
      }

      // Leitura da primeira linha de entrada.txt contendo as dimensoes de M
      fscanf(arquivo_entrada, "%d %d", &L, &C);
      //printf("Ordem da Matriz M: L=%d C=%d\n", L, C);

      // criando M[LxC]
      M = (int *) malloc ( L * C * sizeof(int));

      // criando vet[C] do tipo float. Este vetor terá as medianas das colunas.
      vet = (float *) malloc (C * sizeof (float) );

      // carregando M do arquivo
      for(i=0; i<L; i++) {
        for(j=0; j<C; j++) {
          //A matriz ja eh lida transposta
         fscanf(arquivo_entrada, "%d", &M[j*L+i]);
        }
      }

      for(i=0; i<L*C; i++) {
        printf("%d\n", M[i]);
      }


      MPI_Request mpirequest_mr;
      MPI_Status  status;
      MPI_Comm    inter_comm;
      int root   = 0;
      char slave[20] = "aula7_slave";
      int  errcodes[10];


      //Cria os slaves
      MPI_Comm_spawn(slave, MPI_ARGV_NULL, N, MPI_INFO_NULL, root, MPI_COMM_WORLD, &inter_comm, errcodes);
      int send_buf[2];
      printf("master linhas %d colunas %d\n", L, C);
      send_buf[0] = L;
      send_buf[1] = C;
      printf("send buf 0 %d send buf 1 %d\n", send_buf[0], send_buf[1]);

      //Envia o numero de linhas e colunas para os slaves
      MPI_Bcast(send_buf, 2, MPI_INT, MPI_ROOT, inter_comm);

      int partition_size = L;
      int *rec_buff = malloc(sizeof(int)*partition_size);
      printf("master partition size: %d\n", partition_size);

      //Despacha os chunks para todos os slaves
      MPI_Scatter(M, partition_size, MPI_INT, rec_buff, partition_size, MPI_INT, MPI_ROOT, inter_comm);

      //Imprime a particao que ficou pro master
      for(i=0; i < partition_size; i++) {
        printf("master [%d]=%d\n", i, M[i]);
        printf("\n");
      }

      //Em seguida, o master faria um gather para coletar os resultados das medianas
      //calculadas pelos slaves. Infelizmente, nao deu tempo de terminar

/*      
      float *rec_buf_med = malloc(sizeof(float)*3);
      float send_buf_med[3];
      MPI_Gather(send_buf, 3, MPI_FLOAT, rec_buf_med, 3, MPI_FLOAT, MPI_ROOT, inter_comm);

      for(i=0; i < 3; i++) {
        printf("mediana %d: %f\n", i, rec_buf_med[i]);
      }
*/

      MPI_Finalize();


  }


  
  exit(0);    
} // fim da main

/*
int master() {  


  for(i=0; i<L*C; i++) {
    printf("matriz[%d]=%d\n", i, M3[i]);
  }



  int send_buf[2];


  send_buf[0] = L;
  send_buf[1] = C/4;
  //printf("master: num linhas %d num colunas %d\n", send_buf[0], send_buf[1]);

  for(i=1; i < 3; i++) {//manda linha e coluna pra todos os slaves-1
      MPI_Send((void*)send_buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
  }
  send_buf[1] += C%4;
  //printf("ultimo slave %d vai receber %d\n", i, send_buf[1]);
  MPI_Send((void*)send_buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);//manda pro ultimo slave todas as colunas que sobraram

  int primeiro = C/4;
  for(i=1; i < 4; i++) {
    MPI_Send((void*)&M[L*primeiro], L*C/4, MPI_INT, i, 0, MPI_COMM_WORLD);
    primeiro += C/4;
  }
  
  

  //As primeiras C/4 colunas sao feitas pelo processo master
  ordena_colunas(M, L, C/4);
  calcula_mediana(M, vet, L, C/4);

  //Recebe os resultados dos outros processos e junta no vetor de medianas
  for(i=1; i<3; i++) {
    //printf("processo main aguardando %d medianas do slave %d\n", C/4, i);
    MPI_Recv((void*)&vet[i], C/4, MPI_FLOAT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
    //printf("recebeu mediana %.1lf\n", vet[i]);
  }
  MPI_Recv((void*)&vet[i], (C/4)+(C%4), MPI_FLOAT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
  //Escreve no arquivo de saida
    for(j=0; j<C; j++) 
      fprintf(arquivo_saida,"%.1f, ",vet[j]);
  fprintf(arquivo_saida, "\n");

  fclose(arquivo_entrada);
  fclose(arquivo_saida);

  free(vet);
  free(M);
  return 0;
    
}*/

/*void slave(int my_rank) {
  //printf("executando slave %d\n", my_rank);
  int source, linhas, colunas;
  MPI_Status status;
  int *rec_buff;
  float *medianas;
  int *matriz;
  
  rec_buff = (int*)malloc(sizeof(int)*2);
  MPI_Recv((void*)rec_buff, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//recebe numero de linhas e colunas
  //printf("slave %d recebeu %d linhas e %d colunas\n", my_rank, rec_buff[0], rec_buff[1]);

  source = status.MPI_SOURCE;
  linhas = rec_buff[0];
  colunas = rec_buff[1];
  free(rec_buff);

  //printf("worker %d recebeu %d linhas e %d colunas do source %d\n", my_rank, linhas, colunas, source);

  //printf("slave %d vai receber sua parte da matriz\n", my_rank);
  rec_buff = malloc(sizeof(int)*linhas*colunas);
  MPI_Recv((void*)rec_buff, linhas*colunas, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  //printf("slave %d recebeu sua parte da matriz\n", my_rank);

  matriz = rec_buff;
  int i;


  ordena_colunas(rec_buff, linhas, colunas);
  medianas = (float*)malloc(sizeof(float)*colunas);
  calcula_mediana(rec_buff, medianas, linhas, colunas);
  
  for(i=0; i < colunas; i++) {
    printf("mediana %d do slave %d: %.1lf\n", i, my_rank, medianas[i]);
  }

  //printf("slave %d vai mandar de volta pro master\n", my_rank);
  //printf("worker %d mandando %d medianas de volta\n", my_rank, colunas);
  MPI_Send((void*)medianas, colunas, MPI_FLOAT, source, 0, MPI_COMM_WORLD);
  //printf("slave %d terminou de executar\n", my_rank);

  free(medianas);
  free(rec_buff);
}*/
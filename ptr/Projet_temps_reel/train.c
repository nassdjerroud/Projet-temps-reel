
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>


/* Les villes */
#define A 0
#define B 1
#define C 2
#define D 3


/* Code des threads */
void* train_de_A_vers_C(void* arg);
void* train_de_D_vers_A(void* arg);

pthread_mutex_t mut_AB;
pthread_mutex_t mut_BC;
pthread_mutex_t mut_DB;

/* Gestion du chemin de fer  */
void utiliser_le_segment(int ville_depart, int ville_arrivee)
{
	if  ( (ville_depart == A) && (ville_arrivee == B) )
		pthread_mutex_lock(&mut_AB);
	else
		if  ( (ville_depart == B) && (ville_arrivee == C) )
			pthread_mutex_lock(&mut_BC);
		else
			pthread_mutex_lock(&mut_DB);
}


void liberer_le_segment(int ville_depart, int ville_arrivee)
{
	if  ( (ville_depart == A) && (ville_arrivee == B) )
		pthread_mutex_unlock(&mut_AB);
	else
		if  ( (ville_depart == B) && (ville_arrivee == C) )
			pthread_mutex_unlock(&mut_BC);
		else
			pthread_mutex_unlock(&mut_DB);
}


int main(int argc, char* argv[])
{
	int i;
	pthread_t id;

	/* Creer autant de trains que 
		necessaire */

	pthread_mutex_init(&mut_AB,0);
	pthread_mutex_init(&mut_BC,0);
	pthread_mutex_init(&mut_DB,0);
	
	for(i =0; i< 3; i++)
		{
		pthread_create(&id, NULL, train_de_A_vers_C, NULL);
		pthread_create(&id, NULL, train_de_D_vers_A, NULL);
		}

	pthread_exit(NULL);
}


void* train_de_A_vers_C(void* arg)
{
	utiliser_le_segment(A, B);
	printf("Train %d : utilise segment AB \n",pthread_self());
	attendre(6);
	liberer_le_segment(A, B);

	utiliser_le_segment(B, C);
	printf("Train %d : utilise segment BC \n",pthread_self());
	attendre(6);
	liberer_le_segment(B, C);

	pthread_exit(NULL);
}

void* train_de_D_vers_A(void* arg)
{
	utiliser_le_segment(D, B);
	printf("Train %d : utilise segment DB \n",pthread_self());
	attendre(5);
	liberer_le_segment(D, B);

	utiliser_le_segment(A, B);
	printf("Train %d : utilise segment BA \n",pthread_self());
	attendre(5);
	liberer_le_segment(A, B);

	pthread_exit(NULL);
}





int tirage_aleatoire(double max)
{
        int j=(int) (max*rand()/(RAND_MAX+1.0));
        if(j<1)
                j=1;
        return j;
}



void attendre(double max)
{
        struct timespec delai;

        delai.tv_sec=tirage_aleatoire(max);
        delai.tv_nsec=0;
        nanosleep(&delai,NULL);
}



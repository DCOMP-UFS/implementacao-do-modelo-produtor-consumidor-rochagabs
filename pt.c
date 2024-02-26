#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h> 

//Compile:  gcc -g -Wall -o pt pt.c -lpthread -lrt

#define BUFFER_SIZE 3

typedef struct Clock {
    int id;
    int p[BUFFER_SIZE];
    struct Clock *next;
} Clock;

typedef struct {
    Clock *head;
    Clock *tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond_cons;
    pthread_cond_t cond_produ;
} FilaClock;

FilaClock filaClock;
int nextClockID = 0;

void executeTask(Clock *clock, const char *func) {
    printf("%s thread %d: (%d, %d, %d)\n", func, clock->id, clock->p[0], clock->p[1], clock->p[2]);
}

void ini_fila(FilaClock *fila) {
    fila->head = NULL;
    fila->tail = NULL;
    fila->count = 0;
    pthread_mutex_init(&fila->mutex, NULL);
    pthread_cond_init(&fila->cond_cons, NULL);
    pthread_cond_init(&fila->cond_produ, NULL);
}

void enfileirar(FilaClock *fila, Clock *relogio) {
    pthread_mutex_lock(&fila->mutex);

    while (fila->count >= BUFFER_SIZE) {
        printf("Fila cheia. Aguardando espaço para produção...\n");
        pthread_cond_wait(&fila->cond_cons, &fila->mutex);
    }

    if (fila->head == NULL) {
        fila->head = relogio;
        fila->tail = relogio;
    } else {
        fila->tail->next = relogio;
        fila->tail = relogio;
    }

    fila->count++;
    pthread_cond_signal(&fila->cond_produ);
    pthread_mutex_unlock(&fila->mutex);
}

Clock *desenfileirar(FilaClock *fila) {
    Clock *relogio = NULL;
    pthread_mutex_lock(&fila->mutex);

    while (fila->head == NULL) {
        printf("Fila vazia. Aguardando relógios para consumir...\n");
        pthread_cond_wait(&fila->cond_produ, &fila->mutex);
    }

    relogio = fila->head;
    fila->head = fila->head->next;

    if (fila->head == NULL) {
        fila->tail = NULL;
    }

    fila->count--;
    pthread_cond_signal(&fila->cond_cons);
    pthread_mutex_unlock(&fila->mutex);

    return relogio;
}

void *produtor(void *valor) {
    srand(time(NULL));
    while (1) {
        Clock *relogio = malloc(sizeof(Clock));
        relogio->id = nextClockID++;
        for (int j = 0; j < BUFFER_SIZE; j++) {
            relogio->p[j] = rand() % 11;
        }
        relogio->next = NULL;

        enfileirar(&filaClock, relogio);
        executeTask(relogio, "Produzido");

        sleep(1); //sleeps de controle
    }

    pthread_exit(NULL);
}

void *consumidor(void *valor) {
    while (1) {
        Clock *relogio = desenfileirar(&filaClock);
        executeTask(relogio, "Consumido");
        free(relogio);

        sleep(2); //sleeps de controle
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t produtor_thread, consumidor_thread;

    ini_fila(&filaClock);

// adicionei 3  threads consumidoras e produtoras 

    for (int i = 0; i < 3; i++) {
        pthread_create(&produtor_threads[i], NULL, produtor, NULL);
        pthread_create(&consumidor_threads[i], NULL, consumidor, NULL);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(produtor_threads[i], NULL);
        pthread_join(consumidor_threads[i], NULL);
    }

    pthread_mutex_destroy(&filaClock.mutex);
    pthread_cond_destroy(&filaClock.cond_cons);
    pthread_cond_destroy(&filaClock.cond_produ);

    return 0;
}

    return 0;
}

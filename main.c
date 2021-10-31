#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 22

static const char FILLED = 'X';
static const char EMPTY = ' ';

static const char VERTICAL_DASH = '|';

struct {
	int produced_items;
	int consumed_items;
	sem_t mutex;

	char buffer[BUFFER_SIZE];
} pc;

void init_table();
void display_table();
int number_of(const char);
void *producer_handler(void *arg);
void *consumer_handler(void *arg);
void *escape_handler(void *arg);
void print_consumer(const char *);
void print_producer(const char *);

int main() {
	time_t t;
	// Inicializar el generador de números
	srand((unsigned) time(&t));

	// Inicializar el semaforo con un solo valor
	sem_init(&pc.mutex, 0, 1);
	pthread_t producer, consumer, escape;

	init_table();
	printf("[Main]: Estado inicial del buffer\n");
	display_table();
	printf("\n\n");

	pthread_create(&escape, NULL, escape_handler, NULL);

	while (true) {
		pthread_create(&producer, NULL, producer_handler, NULL);
		sleep(1);
		pthread_create(&consumer, NULL, consumer_handler, NULL);
		sleep(1);

		pthread_join(producer, NULL);
		pthread_join(consumer, NULL);
		printf("\n======\n\n");
	}

	pthread_join(escape, NULL);

	return 0;
}

void init_table() {
	for (int i = 0; i < BUFFER_SIZE; ++i) pc.buffer[i] = EMPTY;
}

int number_of(const char ch) {
	int counter = 0;
	for (int i = 0; i < BUFFER_SIZE; ++i) if (pc.buffer[i] == ch) ++counter;
	return counter;
}

void display_table() {
	printf("[Buffer]: Distribución\n");
	for (int i = 0; i < BUFFER_SIZE / 2; ++i) {
		if (i == 0) printf("%c", VERTICAL_DASH);
		printf("%c%c", pc.buffer[i], VERTICAL_DASH);
	}
	printf("\n");
	for (int i = 0; i < BUFFER_SIZE / 2; ++i) {
		if (i == 0) printf("%c", VERTICAL_DASH);
		printf("%c%c", pc.buffer[i + 11], VERTICAL_DASH);
	}
	printf("\n");
}

void *escape_handler(void *arg) {
	int c;
	while(true) {
		c = getchar();
		break;
	}
	printf("[Escape]: Terminando el programa en la siguiente ronda: %c", c);
}

void *producer_handler(void *arg) {
	char output[80];
	print_producer("Intentando entrar");
	sem_wait(&pc.mutex);

	bool should_do_something = rand() % 2 != 0;
	if (!should_do_something) {
		print_producer("Saltando ronda");
		goto producer_exit;
	}

	int empty_spaces = number_of(EMPTY);

	if (empty_spaces == 0) {
		print_producer("Buffer esta lleno, saltando");
		goto producer_exit;
	}

	int elements_to_produce = 0;
	if (empty_spaces > 7) elements_to_produce = (rand() % 7) + 1;
	else elements_to_produce = (rand() % empty_spaces) + 1;
	sprintf(output, "Insertando %d elementos", elements_to_produce);
	print_producer(output);

	for (int i = 0; i < elements_to_produce; ++i) {
		sprintf(output, "Insertando elemento en la posición %d", pc.produced_items);
		print_producer(output);
		pc.buffer[pc.produced_items] = FILLED;
		pc.produced_items = (pc.produced_items + 1) % BUFFER_SIZE;
	}

producer_exit:
	display_table();
	sem_post(&pc.mutex);
	print_producer("Saliendo");
}

void *consumer_handler(void *arg) {
	char output[80];
	print_consumer("Intentando entrar");
	sem_wait(&pc.mutex);

	bool should_do_something = rand() % 2 != 0;
	if (!should_do_something) {
		print_consumer("Saltando ronda");
		goto consumer_exit;
	}

	int filled_spaces = number_of(FILLED);
	if (filled_spaces == 0) {
		print_consumer("Buffer esta vacio, saltando");
		goto consumer_exit;
	}

	int elements_to_remove = 0;
	if (filled_spaces > 7) elements_to_remove = (rand() % 7) + 1;
	else elements_to_remove = (rand() % filled_spaces) + 1;
	sprintf(output, "Removiendo %d elementos", elements_to_remove);
	print_consumer(output);

	for (int i = 0; i < elements_to_remove; ++i) {
		sprintf(output, "Removiendo elemento en la posición %d", pc.consumed_items);
		print_consumer(output);
		pc.buffer[pc.consumed_items] = EMPTY;
		pc.consumed_items = (pc.consumed_items + 1) % BUFFER_SIZE;
	}

consumer_exit:
	display_table();
	sem_post(&pc.mutex);
	print_consumer("Saliendo");
}

void print_consumer(const char *message) {
	printf("[Consumidor]: %s\n", message);
}

void print_producer(const char *message) {
	printf("[Productor]: %s\n", message);
}

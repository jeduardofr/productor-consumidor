#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 22 // Tamaño del buffer

static const char FILLED = 'X';		   // Carácter para cuando el productor inserta un elemento
static const char EMPTY = ' ';		   // Carácter para cuando un espacio del buffer esta vacío
static const char VERTICAL_DASH = '|'; // Carácter para separar los elementos del buffer

struct
{
	int produced_items;		  // Contador del productor dentro del buffer
	int consumed_items;		  // Contador del consumidor dentro del buffer
	sem_t mutex;			  // Semaforo
	char buffer[BUFFER_SIZE]; // Buffer de datos
} pc;

// Declaración de las funciones usadas en el programa
void init_table();
void display_table();
int number_of(const char);
void *producer_handler(void *arg);
void *consumer_handler(void *arg);
void print_consumer(const char *);
void print_producer(const char *);

int main()
{
	time_t t;
	srand((unsigned)time(&t));	  // Inicializar la semilla para generar números aleatorios
	sem_init(&pc.mutex, 0, 1);	  // Inicializar el semáforo
	pthread_t producer, consumer; // Hilos respectivos del productor y del consumidor

	printf("[Main]: Estado inicial del buffer\n");
	init_table();	 // Inicializamos la tabla con espacios vacíos
	display_table(); // Mostrar los datos que contiene el buffer
	printf("\n\n");

	while (true) // El programa se mantendrá corriendo hasta que el usuario presione Ctrl+C
	{
		pthread_create(&producer, NULL, producer_handler, NULL); // Crear el hilo del productor y ejecutar su funcionalidad
		sleep(1);												 // Dormir por un segundo
		pthread_create(&consumer, NULL, consumer_handler, NULL); // Crear el hilo del consumidor y ejecutar su funcionalidad
		sleep(1);												 // Dormir por un segundo

		pthread_join(producer, NULL); // Esperar a que termine el hilo del productor
		pthread_join(consumer, NULL); // Esperar a que termine el hilo del consumidor
		printf("\n======\n\n");
	}

	return 0;
}

void init_table()
{
	for (int i = 0; i < BUFFER_SIZE; ++i) // Llenamos el buffer con espacios vacíos
		pc.buffer[i] = EMPTY;
}

// Función que cuenta la cantidad de espacios vacíos o de elementos insertados en el buffer
int number_of(const char ch)
{
	int counter = 0;
	for (int i = 0; i < BUFFER_SIZE; ++i)
		if (pc.buffer[i] == ch)
			++counter;
	return counter;
}

// Función que muestra los valores dentro del buffer como una tabla
void display_table()
{
	printf("[Buffer]: Distribución\n");
	for (int i = 0; i < BUFFER_SIZE / 2; ++i)
	{
		if (i == 0)
			printf("%c", VERTICAL_DASH);
		printf("%c%c", pc.buffer[i], VERTICAL_DASH);
	}
	printf("\n");
	for (int i = 0; i < BUFFER_SIZE / 2; ++i)
	{
		if (i == 0)
			printf("%c", VERTICAL_DASH);
		printf("%c%c", pc.buffer[i + 11], VERTICAL_DASH);
	}
	printf("\n");
}

// Función ejecutada por el productor
void *producer_handler(void *arg)
{
	char output[80];
	print_producer("Intentando entrar");
	sem_wait(&pc.mutex); // Esperando a obtener el mutex

	bool should_do_something = rand() % 2 != 0; // Determinar si el productor va hacer algo esta ronda
	if (!should_do_something)
	{
		print_producer("Saltando ronda");
		goto producer_exit; // Saltar al final de la función
	}

	int empty_spaces = number_of(EMPTY); // Calcular la cantidad de espacios vacios para ser llenados por el productor
	if (empty_spaces == 0)				 // Si no hay espacio quiere decir que el buffer esta completamente lleno
	{
		print_producer("Buffer esta lleno, saltando");
		goto producer_exit;
	}

	int elements_to_produce = 0; // Calcular los elementos a ser insertados por el productor. Como minimo 1 y maximo 7
	if (empty_spaces > 7)
		elements_to_produce = (rand() % 7) + 1;
	else
		elements_to_produce = (rand() % empty_spaces) + 1;
	sprintf(output, "Insertando %d elementos", elements_to_produce);
	print_producer(output);

	for (int i = 0; i < elements_to_produce; ++i)
	{
		sprintf(output, "Insertando elemento en la posición %d", pc.produced_items);
		print_producer(output);
		pc.buffer[pc.produced_items] = FILLED;					   // Modificar dicho elemento en el buffer
		pc.produced_items = (pc.produced_items + 1) % BUFFER_SIZE; // Incrementar el contador del productor
	}

producer_exit:
	display_table();
	sem_post(&pc.mutex); // Liberar el semaforo
	print_producer("Saliendo");
}

void *consumer_handler(void *arg)
{
	char output[80];
	print_consumer("Intentando entrar");
	sem_wait(&pc.mutex); // Esperando a obtener el mutex

	bool should_do_something = rand() % 2 != 0; // Determinar si el productor va hacer algo esta ronda
	if (!should_do_something)
	{
		print_consumer("Saltando ronda");
		goto consumer_exit;
	}

	int filled_spaces = number_of(FILLED); // Calcular los elementos a ser eliminados por el productor. Como minimo 1 y maximo 7
	if (filled_spaces == 0)				   // Si no hay elementos llenados, quiere decir que el buffer esta vacío
	{
		print_consumer("Buffer esta vacio, saltando");
		goto consumer_exit;
	}

	int elements_to_remove = 0;
	if (filled_spaces > 7)
		elements_to_remove = (rand() % 7) + 1;
	else
		elements_to_remove = (rand() % filled_spaces) + 1;
	sprintf(output, "Removiendo %d elementos", elements_to_remove);
	print_consumer(output);

	for (int i = 0; i < elements_to_remove; ++i)
	{
		sprintf(output, "Removiendo elemento en la posición %d", pc.consumed_items);
		print_consumer(output);
		pc.buffer[pc.consumed_items] = EMPTY;					   // Modificar dicho elemento en el buffer
		pc.consumed_items = (pc.consumed_items + 1) % BUFFER_SIZE; // Incrementar el contador del productor
	}

consumer_exit:
	display_table();
	sem_post(&pc.mutex); // Liberar el semaforo
	print_consumer("Saliendo");
}

void print_consumer(const char *message)
{
	printf("[Consumidor]: %s\n", message);
}

void print_producer(const char *message)
{
	printf("[Productor]: %s\n", message);
}

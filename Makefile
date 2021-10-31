main:
	@gcc -lpthread -lrt main.c -o consumer_producer
	@./consumer_producer

#include "multithreading.h"

/**
* tprintf - print a message using printf
* @format: format string
* Autor: Frank Onyema Orji
* Return: 0
*/
int tprintf(char const *format, ...)
{
	pthread_t tid = pthread_self(); /* get the thread id */

	printf("[%lu] %s", tid, format);
	return (0);
}

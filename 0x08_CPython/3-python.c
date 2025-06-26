#include <Python.h>
#include <stdio.h>
#include <string.h>

/**
 * print_python_bytes - program that prints the size
 * and first few bytes of a Python bytes object
 * this function prints the first 10 bytes or the total bytes
 * if less than 10;
 * if 'p' is not a valid PyBytesObject, an error message is printed
 * @p: PyObject pointer representing the Python bytes object
 * Return: nothing (void)
 */

void print_python_bytes(PyObject *p)
{
	int i;
	Py_ssize_t size, printed_bytes;
	char *array_as_string = NULL;

	printf("[.] bytes object info\n");

	if (!PyBytes_Check(p))
	{
		printf("  [ERROR] Invalid Bytes Object\n");
		fflush(stdout);
		return;
	}

	size = ((PyVarObject *)(p))->ob_size;

	printf("  size: %li\n", size);

	array_as_string = ((PyBytesObject *)(p))->ob_sval;

	printf("  trying string: %s\n", array_as_string);

	printed_bytes = (size + 1 >= 10) ? 10 : size + 1;

	printf("  first %li bytes:", printed_bytes);

	for (i = 0; i < printed_bytes; i++)
		printf(" %02x", (unsigned char)(array_as_string[i]));
	putchar('\n');

	fflush(stdout);
}

/**
 * print_python_float - program that prints information
 * about a Python float object
 * If 'p' is not a valid PyFloatObject, an error message is printed
 * @p: PyObject pointer representing the Python float object
 * Return: nothing (void)
 */

void print_python_float(PyObject *p)
{
	char float_str[40];

	printf("[.] float object info\n");

	if (!PyFloat_Check(p))
	{
		printf("  [ERROR] Invalid Float Object\n");
		fflush(stdout);
		return;
	}

	sprintf(float_str, "%.16g", ((PyFloatObject *)p)->ob_fval);

	if (strchr(float_str, '.') != NULL)
		printf("  value: %s\n", float_str);
	else
		printf("  value: %.1f\n", ((PyFloatObject *)p)->ob_fval);

	fflush(stdout);
}

/**
 * print_python_list - program that prints information
 * about a Python list object
 * If 'p' is not a valid PyListObject, an error message is printed
 * @p: PyObject pointer representing the Python list object
 * Return: nothing (void)
 */

void print_python_list(PyObject *p)
{
	int i;
	Py_ssize_t size;
	PyObject *list_member;

	printf("[*] Python list info\n");

	if (!PyList_Check(p))
	{
		printf("  [ERROR] Invalid List Object\n");
		fflush(stdout);
		return;
	}

	size = ((PyVarObject *)(p))->ob_size;

	printf("[*] Size of the Python List = %li\n", size);

	printf("[*] Allocated = %lu\n", ((PyListObject *)p)->allocated);

	for (i = 0; i < size; i++)
	{
		list_member = ((PyListObject *)p)->ob_item[i];
		printf("Element %d: %s\n", i,
		       list_member->ob_type->tp_name);

		if (PyBytes_Check(list_member))
			print_python_bytes(list_member);

		else if (PyFloat_Check(list_member))
			print_python_float(list_member);

	}

	fflush(stdout);
}

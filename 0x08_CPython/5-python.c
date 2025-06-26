#include <stdio.h>
#include <Python.h>
#include <stdbool.h>
#define ABS_VAL(x) ((x) < 0 ? -(x) : (x))

/**
 * print_python_int - Displays information about Python integer objects
 *
 * @p: Pointer to a PyObject struct
 * Author: Frank Onyema Orji
 */
void print_python_int(PyObject *p)
{
	Py_ssize_t i, size;
	bool negative;
	unsigned long base10 = 0, pow_base_pylong;
	unsigned int base_pylong = 1 << PyLong_SHIFT;

	if (!PyLong_Check(p))
	{
		puts("Invalid Int Object");
		return;
	}

	size = ABS_VAL(((PyVarObject *)p)->ob_size);
	negative = ((PyVarObject *)p)->ob_size < 0;

	if (size < 3 || (size == 3 && ((PyLongObject *)p)->ob_digit[2] < 16))
	{
		pow_base_pylong = 1;
		for (i = 0; i < size; i++)
		{
			base10 += pow_base_pylong * ((PyLongObject *)p)->ob_digit[i];
			pow_base_pylong *= base_pylong;
		}

		if (negative)
			putchar('-');
		printf("%lu\n", base10);
	}
	else
	{
		puts("C unsigned long int overflow");
	}
}

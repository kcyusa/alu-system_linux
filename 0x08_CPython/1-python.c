#include <python3.4/Python.h>
#include <python3.4/listobject.h>
#include <python3.4/object.h>

/**
 * print_python_list - provides basic information about Python lists
 * @p: Python Object
 * Author: Frank Onyema Orji
 * Return: void
 */
void print_python_list(PyObject *p)
{
    printf("[*] Python list info\n");
    printf("[*] Size of the Python List = %ld\n", PyList_Size(p));
    printf("[*] Allocated = %ld\n", ((PyListObject *)p)->allocated);

    for (int i = 0; i < Py_SIZE(p); i++)
    {
        printf("Element %d: %s\n", i, Py_TYPE(PyList_GetItem(p, i))->tp_name);
    }
}

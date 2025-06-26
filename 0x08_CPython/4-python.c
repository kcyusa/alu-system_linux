#include <python3.4/Python.h>
#include <python3.4/object.h>
#include <python3.4/unicodeobject.h>

/**
 * print_python_string - provides basic information about Python string objects
 * @p: Python Object
 * Author: Frank Onyema Orji
 * Return: void
 */
void print_python_string(PyObject *p)
{
    const char *type = NULL;
    Py_ssize_t len = 0;
    wchar_t *str = NULL;

    printf("[.] string object info\n");
    if (!PyUnicode_Check(p))
    {
        printf("  [ERROR] Invalid String Object\n");
        return;
    }

    if (PyUnicode_IS_COMPACT_ASCII(p))
    {
        type = "compact ascii";
    }
    else
    {
        type = "compact unicode object";
    }

    str = PyUnicode_AsWideCharString(p, &len);

    printf("  type: %s\n", type);
    printf("  length: %ld\n", len);
    printf("  value: %ls\n", str);
}

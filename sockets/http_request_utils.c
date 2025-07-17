#include "sockets.h"
#include <stdlib.h>

/**
 * get_header - get value of a header request field if it exists
 *
 * @headers: headers in a request
 * @field: desired field value
 * Return: value of field if it exists, NULL if it does not
 */
char *get_header(http_header_t *headers, char *field)
{
	for (; headers; headers = headers->next)
		if (strcmp(headers->field, field) == 0)
			return (headers->value);

	return (NULL);
}

/**
 * get_param - returns a parameter value from a list of parameters
 * @params: parameters
 * @key: key to search for
 * Return: value of key
 */
char *get_param(http_param_t *params, char *key)
{
	for (; params; params = params->next)
		if (strcmp(params->key, key) == 0)
			return (params->value);

	return (NULL);
}

/**
 * get_method - returns http_method_t (enum type) for the request method
 *
 * @method_str: the method (string) from the request
 * Return: http_method_t (enum type describing request type (see sockets.h))
 */
http_method_t get_method(char *method_str)
{
	char *method_strs[] = {
		"GET",
		"HEAD",
		"POST",
		"PUT",
		"DELETE",
		"CONNECT",
		"OPTIONS",
		"TRACE"
	};
	size_t i, size = sizeof(method_strs) / sizeof(*method_strs);

	if (!method_str || !(*method_str))
		return (UNKNOWN);

	for (i = 0; i < size; i++)
		if (!strcmp(method_strs[i], method_str))
			return (i);

	return (UNKNOWN);
}

/**
 * http_headers_init - creates linked list of http_header_t structs. each node
 *                     in the list is equal to one request header field/value
 *                     pair
 *
 * @line: string of headers
 * Return: head of linked list of http_header_t structs
 **/
http_header_t *http_headers_init(char *line)
{
	http_header_t *tmp, *head = NULL, *prev = NULL;
	char *next_line;

	while (*line != '\r')
	{
		tmp = malloc(sizeof(http_header_t));
		if (!(*line) || !tmp)
		{
			free(tmp);
			while (head)
				tmp = head->next, free(head), head = tmp;
			return (NULL);
		}
		next_line  = strstr(line, "\r\n") + 2;
		tmp->field = strtok(line, ":");
		tmp->value = strtok(NULL, "\r\n") + 1;
		tmp->next  = NULL;
		if (prev)
			prev->next = tmp;
		else
			head = tmp;
		line = next_line;
		prev = tmp;
	}

	return (head);
}

/**
 * http_params_init - creates linked list of http_param_t structs. each node
 *                     in the list is equal to one key/value parameter pair
 *
 * @str: string of key value pairs, delimited by '&'s
 * Return: head of linked list of http_param_t structs
 **/
http_param_t *http_params_init(char *str)
{
	char *key, *value;
	http_param_t *head = NULL, *prev = NULL, *tmp;

	if (!str || !(*str))
		return (NULL);

	for (
		key = strtok(str, "="), value = strtok(NULL, "&");
		key && value;
		key = strtok(NULL, "="), value = strtok(NULL, "&")
	)
	{
		tmp = malloc(sizeof(http_header_t));
		if (!tmp)
		{
			while (head)
				tmp = head->next, free(head), head = tmp;
			return (NULL);
		}
		tmp->key   = key;
		tmp->value = value;
		tmp->next  = NULL;
		if (prev)
			prev->next = tmp;
		else
			head = tmp;
		prev = tmp;
	}

	return (head);
}

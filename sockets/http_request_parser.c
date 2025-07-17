#include "sockets.h"
#include <stdlib.h>

http_method_t get_method(char *method_str);
http_header_t *http_headers_init(char *line);
http_param_t *http_params_init(char *str);

/**
 * http_request_init - constructs an instance of a http_request_t struct, which
 *                     describes an http request received by the server, by
 *                     parsing the raw HTTP request sent by the client.
 *
 *                     EXAMPLE:
 *                     * An HTTP Request looks like this:
 *                         <request-method> <request-uri> <http->version>\r\n
 *                         <request-header-field>: <header-value>\r\n
 *                             . . . (as many request headers as needed)
 *                         \r\n
 *                         <request-body>\0
 *
 *                     * Here's an example of an HTTP Request:
 *                         POST /todos HTTP/1.1\r\n
 *                         Content-Length: 29\r\n
 *                         \r\n
 *                         title=Hello&description=World
 *
 *                     * This function parses the request to create an
 *                     * http_request_t struct. Defined in `./sockets.h`
 *
 * @request_string: HTTP request received (as string)
 * Return: pointer to a struct descrbing the request
 */
http_request_t *http_request_init(char *request_string)
{
	http_request_t *request = malloc(sizeof(http_request_t));
	char *headers, *body, *query;

	if (!request)
		return (NULL);

	headers = strstr(request_string, "\r\n");
	body = strstr(request_string, "\r\n\r\n");

	if (!headers || !body)
	{
		free(request);
		return (NULL);
	}
	headers += strlen("\r\n");
	body += strlen("\r\n\r\n");

	request->raw_request = strdup(request_string);
	request->method_str  = strtok(request_string, " ");
	request->method      = get_method(request->method_str);
	request->uri         = strtok(NULL, " ");
	request->version     = strtok(NULL, "\r\n");
	request->headers     = http_headers_init(headers);
	request->body        = *body ? strdup(body) : NULL;
	request->body_params = http_params_init(body);
	query = strchr(request->uri, '?'); /* ex: /todos?id=1 */
	if (query)	/* query -> ?id=1 */
		*(query++) = '\0'; /* nullify '?' and move query to next char */
	request->query_params = http_params_init(query); /* query -> id=1 */
	return (request);
}

/**
 * free_http_request - deallocates memory from a heap-alloacted http_request_t
 *                     struct
 *
 * @request: pointer to request
 */
void free_http_request(http_request_t *request)
{
	http_param_t *param_tmp;
	http_header_t *header_tmp;

	if (request)
	{
		free(request->raw_request);
		while (request->body_params)
		{
			param_tmp = request->body_params->next;
			free(request->body_params);
			request->body_params = param_tmp;
		}
		while (request->headers)
		{
			header_tmp = request->headers->next;
			free(request->headers);
			request->headers = header_tmp;
		}
		free(request);
	}
}

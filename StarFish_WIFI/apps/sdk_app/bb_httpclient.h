
#ifndef BB_HTTPCLIENT_H_
#define BB_HTTPCLIENT_H_

#include "bb_url.h"

typedef struct tag_http_response
{
	BB_PARSED_URL *request_uri;
	char *body;
	char *status_code;
	int status_code_int;
	char *status_text;
	char *request_headers;
	char *response_headers;
}BB_HTTP_RESPONSE;

BB_HTTP_RESPONSE* http_req(char *http_headers, BB_PARSED_URL *purl);
BB_HTTP_RESPONSE* http_get(char *url, char *custom_headers);
BB_HTTP_RESPONSE* http_head(char *url, char *custom_headers);
BB_HTTP_RESPONSE* http_post(char *url, char *custom_headers, char *post_data);
void http_response_free(BB_HTTP_RESPONSE *hresp);

#endif /* BB_HTTPCLIENT_H_ */

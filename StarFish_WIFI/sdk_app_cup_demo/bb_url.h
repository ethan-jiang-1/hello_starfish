#ifndef BB_URL_H_
#define BB_URL_H_

typedef struct tag_parsed_url{
	char *uri;					/* mandatory */
	char *scheme;               /* mandatory */
	char *host;                 /* mandatory */
	char *ip; 					/* mandatory */
	char *port;                 /* optional */
	char *path;                 /* optional */
	char *query;                /* optional */
	char *fragment;             /* optional */
	char *username;             /* optional */
	char *password;             /* optional */
} BB_PARSED_URL;


void bb_parsed_url_free(BB_PARSED_URL *purl);
BB_PARSED_URL *bb_parse_url(const char *url);


char *bb_urlencode(char *str);

#endif /* BB_URL_H_ */

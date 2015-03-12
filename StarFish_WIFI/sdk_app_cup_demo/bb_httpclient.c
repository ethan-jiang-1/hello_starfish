#include <qcom/qcom_common.h>
#include <qcom/socket_api.h>
#include <qcom/select_api.h>
#include "bb_url.h"
#include "utils.h"
#include "bb_base64.h"
#include "bb_httpclient.h"

#define BUFSIZ 1024
BB_HTTP_RESPONSE hresp_buf;
struct sockaddr_in remote_sockaddr;
char response[BUFSIZ];
char http_headers_buf[1024];
char upwd_buf[512];
char auth_buf[512];
extern A_UINT32 _inet_addr(A_CHAR *str);

/*
    Handles redirect if needed for get requests
*/
BB_HTTP_RESPONSE* handle_redirect_get(BB_HTTP_RESPONSE *hresp, char *custom_headers) 
{
    if (hresp->status_code_int > 300 && hresp->status_code_int < 399) {

        char *token = str_strtok(hresp->response_headers, "\r\n");

        while (token != NULL) {

            if (str_contains(token, "Location:")) {
                /* Extract url */
                char *location = str_replace("Location: ", "", token);
                return http_get(location, custom_headers);
            }

            token = str_strtok(NULL, "\r\n");
        }
    } else {
        /* We're not dealing with a redirect, just return the same structure */
        return hresp;
    }

    return NULL;
}

/*
    Handles redirect if needed for head requests
*/
BB_HTTP_RESPONSE* handle_redirect_head(BB_HTTP_RESPONSE *hresp, char *custom_headers) 
{
    if (hresp->status_code_int > 300 && hresp->status_code_int < 399) {
        char *token = str_strtok(hresp->response_headers, "\r\n");
        while (token != NULL) {
            if (str_contains(token, "Location:")) {
                /* Extract url */
                char *location = str_replace("Location: ", "", token);
                return http_head(location, custom_headers);
            }
            token = str_strtok(NULL, "\r\n");
        }
    } else {
        /* We're not dealing with a redirect, just return the same structure */
        return hresp;
    }
    return NULL;
}

/*
    Handles redirect if needed for post requests
*/
BB_HTTP_RESPONSE* handle_redirect_post(BB_HTTP_RESPONSE *hresp, char *custom_headers, char *post_data) 
{
    if (hresp->status_code_int > 300 && hresp->status_code_int < 399) {

        char *token = str_strtok(hresp->response_headers, "\r\n");

        while (token != NULL) {

            if (str_contains(token, "Location:")) {
                /* Extract url */
                char *location = str_replace("Location: ", "", token);
                return http_post(location, custom_headers, post_data);
            }

            token = str_strtok(NULL, "\r\n");
        }
    } else {
        /* We're not dealing with a redirect, just return the same structure */
        return hresp;
    }
    return NULL;
}

#ifndef AF_INET
#define AF_INET 2
#endif

/*
    Makes a HTTP request and returns the response
*/
BB_HTTP_RESPONSE* http_req(char *http_headers, BB_PARSED_URL *purl) 
{
    /* Parse url */
    if (purl == NULL) {
        A_PRINTF("Unable to parse url");
        return NULL;
    }

    /* Declare variable */
    int sock;
    int tmpres;
    //char buf[BUFSIZ+1];
    struct sockaddr_in *remote;

    /* Allocate memeory for htmlcontent */
  //  BB_HTTP_RESPONSE *hresp = (BB_HTTP_RESPONSE *)qcom_mem_alloc(sizeof(BB_HTTP_RESPONSE));
    memset(&hresp_buf,0,sizeof(BB_HTTP_RESPONSE));
    BB_HTTP_RESPONSE *hresp = &hresp_buf;
    hresp->body = NULL;
    hresp->request_headers = NULL;
    hresp->response_headers = NULL;
    hresp->status_code = NULL;
    hresp->status_text = NULL;

    /* Create TCP socket */
    if ((sock = qcom_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        A_PRINTF("Can't create TCP socket");
        return NULL;
    }

    /* Set remote->sin_addr.s_addr */
   // remote = (struct sockaddr_in *)qcom_mem_alloc(sizeof(struct sockaddr_in));
    memset(&remote_sockaddr,0,sizeof(remote_sockaddr));
    remote =  &remote_sockaddr;
    remote->sin_family      = AF_INET;
    remote->sin_addr.s_addr = htonl(_inet_addr(purl->ip));

    if (remote->sin_addr.s_addr == 0) {
        A_PRINTF("Not a valid IP");
        return NULL;
    }

    remote->sin_port = htons(atoi(purl->port));

    /* Connect */
    if (qcom_connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0) {
        A_PRINTF("Could not connect, remote IP = %s(%08X), port = %d\r\n", 
                 purl->ip, remote->sin_addr.s_addr, remote->sin_port);
		qcom_close(sock);
        return NULL;
    }

    A_PRINTF("Connected to remote IP = %s, port = %d\r\n", purl->ip, remote->sin_port);

    /* Send headers to server */
    int sent = 0;
    while (sent < strlen(http_headers)) {
        tmpres = qcom_send(sock, http_headers + sent, strlen(http_headers) - sent, 0);
        if (tmpres == -1) {
            A_PRINTF("Can't send headers");
            return NULL;
        }
        sent += tmpres;
    }

    A_PRINTF("zg Send http header = %s\r\n", http_headers);

    /* Recieve into response*/
    //char *response = (char *)qcom_mem_alloc(0);
  //  char *response     = (char *)qcom_mem_alloc(BUFSIZ);//static char response_buf[BUFSIZ];
   // memset(response_buf,0,BUFSIZ);
    //memset(pBuf,0,BUFSIZ);
    //char *response     = response_buf;
    int recived_len = 0;
    int  response_pos=0;
    q_fd_set fdRead;

    // Define event wait time
    struct timeval aTime;
    aTime.tv_sec  = 5;
    aTime.tv_usec = 0;

    // Zero fdRead event
    FD_ZERO(&fdRead);

    // Set client socket read event
    FD_SET(sock, &fdRead);

    // Select, check whether there is any event happen
    int ret = qcom_select(sock + 1, &fdRead, NULL, NULL, &aTime); 

    if (ret > 0) {
        
        if (FD_ISSET(sock, &fdRead)) {

            while ((recived_len = qcom_recv(sock, &response[response_pos], BUFSIZ - 1, 0)) > 0) {
		response_pos+=recived_len;
		A_PRINTF("recived_len1:%d,response_pos:%d\r\n",recived_len,response_pos);
                
            }
        }
    }
    else {
        A_PRINTF("Timeout in qcom_select\r\n");
        goto ERRRO_HANDLE;
    }
     A_PRINTF("recived_len2:%d\r\n",recived_len);
    if(response_pos>0)
	{
	 response[response_pos]='\0';	
	}
	
    if (recived_len < 0 || strlen(response) == 0) {
        A_PRINTF("No data receive\r\n");
        goto ERRRO_HANDLE;
    }

    A_PRINTF("Received resp = %s\r\n", response);

   // qcom_mem_free(pBuf);
    A_PRINTF("response add1 = 0x%x\r\n", response);


    /* Close socket */
    qcom_close(sock);
     A_PRINTF("response add2 = 0x%x\r\n", response);
/*test parser&memory leak: it seems ok*/
	if(1)
	{
		char *body = strstr(response, "\r\n\r\n");
    	A_PRINTF("body  = %s\r\n", body );
   	 body = str_replace("\r\n\r\n", "", body);
	qcom_mem_free(body);
	}	
if(0)
{
     /* Parse status code and text */
    char *status_line = get_until(response, "\r\n");
     A_PRINTF("status_line add1 = %s\r\n", status_line);
    status_line = str_replace("HTTP/1.1 ", "", status_line);
    A_PRINTF("status_line add2 = %s\r\n", status_line);
    char *status_code = str_ndup(status_line, 4);
    A_PRINTF("status_code  add1= %s\r\n", status_code );
    status_code = str_replace(" ", "", status_code);
    A_PRINTF("status_code  add2= %s\r\n", status_code );
    char *status_text = str_replace(status_code, "", status_line);
    A_PRINTF("status_text add1 = %s\r\n", status_text );
    status_text = str_replace(" ", "", status_text);
    A_PRINTF("status_text add2 = 0x%x\r\n", status_text );
    hresp->status_code = status_code;
    hresp->status_code_int = atoi(status_code);
    hresp->status_text = status_text;

    /* Parse response headers */
    char *headers = get_until(response, "\r\n\r\n");
      A_PRINTF("headers  = %s\r\n", headers );
    hresp->response_headers = headers;

    /* Assign request headers */
    hresp->request_headers = http_headers;

    /* Assign request url */
    hresp->request_uri = purl;

    /* Parse body */
    char *body = strstr(response, "\r\n\r\n");
    A_PRINTF("body  = %s\r\n", body );
    body = str_replace("\r\n\r\n", "", body);
    hresp->body = body;
}
    /* Return response */
    return hresp;

ERRRO_HANDLE:
   // qcom_mem_free(http_headers);
    //qcom_mem_free(pBuf);

    qcom_close(sock);
    A_PRINTF("Unabel to recieve\r\n");

    return NULL; 
}

/*
    Makes a HTTP GET request to the given url
*/
BB_HTTP_RESPONSE* http_get(char *url, char *custom_headers) 
{
    /* Parse url */
    BB_PARSED_URL *purl = bb_parse_url(url);
    if (purl == NULL) {
        A_PRINTF("Unable to parse url");
        return NULL;
    }

    /* Declare variable */
 //   char *http_headers = (char *)qcom_mem_alloc(1024);//static char http_headers_buf[1024];
	memset(http_headers_buf,0,1024);
      char *http_headers = http_headers_buf;
	
    /* Build query/headers */
    if (purl->path != NULL) {
        if (purl->query != NULL) {
            sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
        } else {
            sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
        }
    } else {
        if (purl->query != NULL) {
            sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
        } else {
            sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
        }
    }

    /* Handle authorisation if needed */
    if (purl->username != NULL) {
        /* Format username:password pair */
	memset(upwd_buf,0,512);
        char *upwd = upwd_buf;//(char *)qcom_mem_alloc(1024);//char upwd_buf[512]
        sprintf(upwd, "%s:%s", purl->username, purl->password);
       // upwd = (char *)qcom_mem_realloc(upwd, strlen(upwd) + 1);

        /* Base64 encode */
        char *base64 = bb_base64_encode(upwd);

        /* Form header */
	memset(auth_buf,0,512);
        char *auth_header = auth_buf;//(char *)qcom_mem_alloc(1024);// char auth_buf[512]
        sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
    //    auth_header = (char *)qcom_mem_realloc(auth_header, strlen(auth_header) + 1);

        /* Add to header */
      //  http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
        sprintf(http_headers, "%s%s", http_headers, auth_header);
    }

    /* Add custom headers, and close */
    if (custom_headers != NULL) {
        sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
    } else {
        sprintf(http_headers, "%s\r\n", http_headers);
    }

    //http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + 1);

    /* Make request and return response */
    BB_HTTP_RESPONSE *hresp = http_req(http_headers, purl);

    if (hresp == NULL) {
        return NULL;
    }

    /* Handle redirect */
    return handle_redirect_get(hresp, custom_headers);
}

/*
    Makes a HTTP POST request to the given url
*/
BB_HTTP_RESPONSE* http_post(char *url, char *custom_headers, char *post_data) 
{
    /* Parse url */
    BB_PARSED_URL *purl = bb_parse_url(url);
    if (purl == NULL) {
        A_PRINTF("Unable to parse url");
        return NULL;
    }
	 A_PRINTF("post parse url 0\r\n");
    /* Declare variable */
		memset(http_headers_buf,0,1024);
      char *http_headers = http_headers_buf;

    /* Build query/headers */
    if (purl->path != NULL) {
        if (purl->query != NULL) {
A_PRINTF("post parse url 0.1\r\n");
            sprintf(http_headers, "POST /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->query, purl->host, strlen(post_data));
        } else {
A_PRINTF("post parse url 0.2\r\n");
            sprintf(http_headers, "POST /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->host, strlen(post_data));
        }
    } else {
        if (purl->query != NULL) {
A_PRINTF("post parse url 0.3\r\n");
            sprintf(http_headers, "POST /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->query, purl->host, strlen(post_data));
        } else {
A_PRINTF("post parse url 0.4\r\n");
            sprintf(http_headers, "POST / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->host, strlen(post_data));
        }
    }
 A_PRINTF("post parse url1\r\n");
    /* Handle authorisation if needed */
    if (purl->username != NULL) {
        /* Format username:password pair */
       // char *upwd = (char *)qcom_mem_alloc(1024);
		memset(upwd_buf,0,512);
        char *upwd = upwd_buf;
        sprintf(upwd, "%s:%s", purl->username, purl->password);
        //upwd = (char *)qcom_mem_realloc(upwd, strlen(upwd) + 1);

        /* Base64 encode */
        char *base64 = bb_base64_encode(upwd);

        /* Form header */
       // char *auth_header = (char *)qcom_mem_alloc(1024);
			memset(auth_buf,0,512);
        char *auth_header = auth_buf;//(char *)qcom_mem_allo
        sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
      //  auth_header = (char *)qcom_mem_realloc(auth_header, strlen(auth_header) + 1);

        /* Add to header */
        http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
        sprintf(http_headers, "%s%s", http_headers, auth_header);
    }
A_PRINTF("post parse url 2\r\n");
    if (custom_headers != NULL) {
        sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
        sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
    } else {
        sprintf(http_headers, "%s\r\n%s", http_headers, post_data);
    }

  //  http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + 1);
A_PRINTF("post parse url 3\r\n");
    /* Make request and return response */
    BB_HTTP_RESPONSE *hresp = http_req(http_headers, purl);
	    if (hresp == NULL) {
        return NULL;
    }
A_PRINTF("post parse url4\r\n");
    /* Handle redirect */
    return handle_redirect_post(hresp, custom_headers, post_data);
}

/*
    Makes a HTTP HEAD request to the given url
*/
BB_HTTP_RESPONSE* http_head(char *url, char *custom_headers) 
{
    /* Parse url */
    BB_PARSED_URL *purl = bb_parse_url(url);
    if (purl == NULL) {
        A_PRINTF("Unable to parse url");
        return NULL;
    }

    /* Declare variable */
    char *http_headers = (char *)qcom_mem_alloc(1024);

    /* Build query/headers */
    if (purl->path != NULL) {
        if (purl->query != NULL) {
            sprintf(http_headers, "HEAD /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
        } else {
            sprintf(http_headers, "HEAD /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
        }
    } else {
        if (purl->query != NULL) {
            sprintf(http_headers, "HEAD/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
        } else {
            sprintf(http_headers, "HEAD / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
        }
    }

    /* Handle authorisation if needed */
    if (purl->username != NULL) {
        /* Format username:password pair */
        char *upwd = (char *)qcom_mem_alloc(1024);
        sprintf(upwd, "%s:%s", purl->username, purl->password);
        upwd = (char *)qcom_mem_realloc(upwd, strlen(upwd) + 1);

        /* Base64 encode */
        char *base64 = bb_base64_encode(upwd);

        /* Form header */
        char *auth_header = (char *)qcom_mem_alloc(1024);
        sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
        auth_header = (char *)qcom_mem_realloc(auth_header, strlen(auth_header) + 1);

        /* Add to header */
        http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
        sprintf(http_headers, "%s%s", http_headers, auth_header);
    }

    if (custom_headers != NULL) {
        sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
    } else {
        sprintf(http_headers, "%s\r\n", http_headers);
    }

    http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + 1);

    /* Make request and return response */
    BB_HTTP_RESPONSE *hresp = http_req(http_headers, purl);

    /* Handle redirect */
    return handle_redirect_head(hresp, custom_headers);
}

/*
    Do HTTP OPTIONs requests
*/
BB_HTTP_RESPONSE* http_options(char *url) 
{
    /* Parse url */
    BB_PARSED_URL *purl = bb_parse_url(url);
    if (purl == NULL) {
        A_PRINTF("Unable to parse url");
        return NULL;
    }

    /* Declare variable */
    char *http_headers = (char *)qcom_mem_alloc(1024);

    /* Build query/headers */
    if (purl->path != NULL) {
        if (purl->query != NULL) {
            sprintf(http_headers, "OPTIONS /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
        } else {
            sprintf(http_headers, "OPTIONS /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
        }
    } else {
        if (purl->query != NULL) {
            sprintf(http_headers, "OPTIONS/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
        } else {
            sprintf(http_headers, "OPTIONS / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
        }
    }

    /* Handle authorisation if needed */
    if (purl->username != NULL) {
        /* Format username:password pair */
        char *upwd = (char *)qcom_mem_alloc(1024);
        sprintf(upwd, "%s:%s", purl->username, purl->password);
        upwd = (char *)qcom_mem_realloc(upwd, strlen(upwd) + 1);

        /* Base64 encode */
        char *base64 = bb_base64_encode(upwd);

        /* Form header */
        char *auth_header = (char *)qcom_mem_alloc(1024);
        sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
        auth_header = (char *)qcom_mem_realloc(auth_header, strlen(auth_header) + 1);

        /* Add to header */
        http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
        sprintf(http_headers, "%s%s", http_headers, auth_header);
    }

    /* Build headers */
    sprintf(http_headers, "%s\r\n", http_headers);
    http_headers = (char *)qcom_mem_realloc(http_headers, strlen(http_headers) + 1);

    /* Make request and return response */
    BB_HTTP_RESPONSE *hresp = http_req(http_headers, purl);

    /* Handle redirect */
    return hresp;
}

/*
    Free memory of http_response
*/
void http_response_free(BB_HTTP_RESPONSE *hresp) 
{
    if (hresp != NULL) {

        if (hresp->request_uri != NULL) 
            bb_parsed_url_free(hresp->request_uri);

        if (hresp->body != NULL) 
            qcom_mem_free(hresp->body);

        if (hresp->status_code != NULL) 
            qcom_mem_free(hresp->status_code);

        if (hresp->status_text != NULL) 
            qcom_mem_free(hresp->status_text);

        if (hresp->request_headers != NULL) 
            qcom_mem_free(hresp->request_headers);

        if (hresp->response_headers != NULL) 
            qcom_mem_free(hresp->response_headers);

        qcom_mem_free(hresp);
    }
}


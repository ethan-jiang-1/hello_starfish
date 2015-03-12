#include <qcom/qcom_common.h>
#include <qcom/socket_api.h>
#include <qcom/select_api.h>
#include "bb_url.h"
#include "ctype.h"
#include "utils.h"
BB_PARSED_URL BB_URL;
#define URL_PARA 32
char schema_buf[256];
char url_password[URL_PARA];
char url_host[URL_PARA];
char url_port[URL_PARA];
char url_path[URL_PARA];
char url_fragment[URL_PARA];
char url_query[128];
char url_username[URL_PARA];
//char encode_buf[256];
/*
    Free memory of parsed url
*/
void bb_parsed_url_free(BB_PARSED_URL *purl) 
{/*
    if (NULL != purl) {

        if (NULL != purl->scheme) 
            qcom_mem_free(purl->scheme);

        if (NULL != purl->host) 
            qcom_mem_free(purl->host);

        if (NULL != purl->port) 
            qcom_mem_free(purl->port);

        if (NULL != purl->path)  
            qcom_mem_free(purl->path);

        if (NULL != purl->query) 
            qcom_mem_free(purl->query);

        if (NULL != purl->fragment) 
            qcom_mem_free(purl->fragment);

        if (NULL != purl->username) 
            qcom_mem_free(purl->username);

        if (NULL != purl->password) 
            qcom_mem_free(purl->password);

        qcom_mem_free(purl);
    }
*/
}

/*
    Retrieves the IP adress of a hostname
*/
char* hostname_to_ip(char *hostname) 
{
    A_UINT32 ip = 0;
    
    qcom_dnsc_get_host_by_name(hostname, &ip);

    return _inet_ntoa(ip);
}

/*
    Check whether the character is permitted in scheme string
*/
static int is_scheme_char(int c) 
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/*
    Parses a specified URL and returns the structure named 'parsed_url'
    Implented according to:
    RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
    RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
*/
BB_PARSED_URL* bb_parse_url(const char *url) 
{
    /* Define variable */
    BB_PARSED_URL *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;
    memset(&BB_URL,0,sizeof(BB_PARSED_URL));
    memset(schema_buf,0,256);
    memset(url_password,0,URL_PARA);
	memset(url_host,0,URL_PARA);
	memset(url_port,0,URL_PARA);
	memset(url_path,0,URL_PARA);
	memset(url_query,0,128);
	memset(url_fragment,0,URL_PARA);
	memset(url_username,0,URL_PARA);
	memset(url_query,0,128);

    /* Allocate the parsed url storage */
    purl = &BB_URL;//(BB_PARSED_URL *)qcom_mem_alloc(sizeof(BB_PARSED_URL));
    if (NULL == purl) {
        return NULL;
    }

    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = NULL;
    purl->path = NULL;
    purl->query = NULL;
    purl->fragment = NULL;
    purl->username = NULL;
    purl->password = NULL;
    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if (NULL == tmpstr) {
        bb_parsed_url_free(purl); 
        A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);

        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;

    /* Check restrictions */
    for (i = 0; i < len; i++) {
        if (is_scheme_char(curstr[i]) == 0) {
            /* Invalid format */
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
    }

    /* Copy the scheme to the storage */
    purl->scheme = schema_buf;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("schema len:%d\r\n",sizeof(char) * (len + 1));
    if (NULL == purl->scheme) {
        bb_parsed_url_free(purl); 
        A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }

    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';

    /* Make the character to lower if it is upper case. */
    for (i = 0; i < len; i++) {
        purl->scheme[i] = tolower(purl->scheme[i]);
    }

    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for (i = 0; i < 2; i++) {
        if ('/' != *curstr) {
            bb_parsed_url_free(purl);  
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ('\0' != *tmpstr) {
        if ('@' == *tmpstr) {
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        } else if ('/' == *tmpstr) {
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if (userpass_flag) {

        /* Read username */
        while ('\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr) {
            tmpstr++;
        }

        len = tmpstr - curstr;
        purl->username = url_username;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("username len:%d\r\n",sizeof(char) * (len + 1));
        if (NULL == purl->username) {
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }

        strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';

        /* Proceed current pointer */
        curstr = tmpstr;
        if (':' == *curstr) {
            /* Skip ':' */
            curstr++;

            /* Read password */
            tmpstr = curstr;
            while ('\0' != *tmpstr && '@' != *tmpstr) {
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = url_password;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("passwd len:%d\r\n",sizeof(char) * (len + 1));
            if (NULL == purl->password) {
                bb_parsed_url_free(purl); 
                A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }

        /* Skip '@' */
        if ('@' != *curstr) {
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }

    if ('[' == *curstr) {
        bracket_flag = 1;
    } else {
        bracket_flag = 0;
    }

    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ('\0' != *tmpstr) {
        if (bracket_flag && ']' == *tmpstr) {
            /* End of IPv6 address. */
            tmpstr++;
            break;
        } else if (!bracket_flag && (':' == *tmpstr || '/' == *tmpstr)) {
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }

    len = tmpstr - curstr;
    purl->host = url_host;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("host len:%d\r\n",sizeof(char) * (len + 1));
    if (NULL == purl->host || len <= 0) {
        bb_parsed_url_free(purl); 
        A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }

    strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if (':' == *curstr) {
        curstr++;
        /* Read port number */
        tmpstr = curstr;

        while ('\0' != *tmpstr && '/' != *tmpstr) {
            tmpstr++;
        }

        len = tmpstr - curstr;
        purl->port = url_port;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("port len:%d\r\n",sizeof(char) * (len + 1));
        if (NULL == purl->port) {
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }

        (void)strncpy(purl->port, curstr, len);
        purl->port[len] = '\0';
        curstr = tmpstr;
    } else {
        purl->port = "80";
    }

    /* Get ip */
    char *ip = purl->host;//hostname_to_ip(purl->host);
    purl->ip = ip;

    /* Set uri */
    purl->uri = (char *)url;

    /* End of the string */
    if ('\0' == *curstr) {
        return purl;
    }
	A_PRINTF("url parse 1\r\n");
    /* Skip '/' */
    if ('/' != *curstr) {
        bb_parsed_url_free(purl); 
        A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ('\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr) {
        tmpstr++;
    }

    len = tmpstr - curstr;
    purl->path = url_path;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("path len:%d\r\n",sizeof(char) * (len + 1));
    if (NULL == purl->path) {
        bb_parsed_url_free(purl); 
        A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    	A_PRINTF("url parse 2\r\n");
    strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    /* Is query specified? */
    if ('?' == *curstr) {
        /* Skip '?' */
        curstr++;
        /* Read query */
        tmpstr = curstr;

        while ('\0' != *tmpstr && '#' != *tmpstr) {
            tmpstr++;
        }

        len = tmpstr - curstr;
        purl->query = url_query;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("query len:%d\r\n",sizeof(char) * (len + 1));

        if (NULL == purl->query) {
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }

        strncpy(purl->query, curstr, len);
        purl->query[len] = '\0';
        curstr = tmpstr;
    }
		A_PRINTF("url parse 3\r\n");
    /* Is fragment specified? */
    if ('#' == *curstr) {

        /* Skip '#' */
        curstr++;
        /* Read fragment */
        tmpstr = curstr;

        while ('\0' != *tmpstr) {
            tmpstr++;
        }

        len = tmpstr - curstr;
        purl->fragment = url_fragment;//(char *)qcom_mem_alloc(sizeof(char) * (len + 1));
  A_PRINTF("frag len:%d\r\n",sizeof(char) * (len + 1));
        if (NULL == purl->fragment) {
            bb_parsed_url_free(purl); 
            A_PRINTF("Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
	A_PRINTF("url parse 4\r\n");
        strncpy(purl->fragment, curstr, len);

        purl->fragment[len] = '\0';
        curstr = tmpstr;
    }
	A_PRINTF("url parse 5\r\n");
    return purl;
}


static char to_hex(char code) 
{
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/*
    URL encodes a string
*/
char* bb_urlencode(char *str) 
{
    char *pstr = str, *buf = (char *)qcom_mem_alloc(strlen(str) * 3 + 1), *pbuf = buf;
   //memset(encode_buf,0,256);
  A_PRINTF("encode len:%d\r\n",(strlen(str) * 3 + 1));
    while (*pstr) {

        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
            *pbuf++ = *pstr;
        else if (*pstr == ' ') 
            *pbuf++ = '+';
        else 
            *pbuf++ = '%',*pbuf++ = to_hex(*pstr >> 4),*pbuf++ = to_hex(*pstr & 15);

        pstr++;
    }

    *pbuf = '\0';

    return buf;
}

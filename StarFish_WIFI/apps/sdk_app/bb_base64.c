#include <qcom/qcom_common.h>
#include "bb_base64.h"

static char st_b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
static void decodeblock(unsigned char in[], char *clrstr)
{
  	unsigned char out[4];
	out[0] = in[0] << 2 | in[1] >> 4;
	out[1] = in[1] << 4 | in[2] >> 2;
	out[2] = in[2] << 6 | in[3] >> 0;
	out[3] = '\0';
	strncat((char *)clrstr, (char *)out, sizeof(out));
}

/*
	Decodes a Base64 string
*/
char* bb_base64_decode(char *b64src)
{
	char *clrdst = (char*)qcom_mem_alloc( ((strlen(b64src) - 1) / 3 ) * 4 + 4 + 50);
	//char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int c, phase, i;
	unsigned char in[4];
	char *p;
	clrdst[0] = '\0';
	phase = 0; i=0;
	while(b64src[i])
	{
		c = (int) b64src[i];
		if(c == '=')
		{
			decodeblock(in, clrdst);
			break;
		}
		p = strchr(st_b64, c);
		if(p)
		{
			in[phase] = p - st_b64;
			phase = (phase + 1) % 4;
			if(phase == 0)
			{
				decodeblock(in, clrdst);
				in[0]=in[1]=in[2]=in[3]=0;
			}
		}
		i++;
	}
	clrdst = (char*)qcom_mem_realloc(clrdst, strlen(clrdst) + 1);
	return clrdst;
}

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
static void encodeblock( unsigned char in[], char b64str[], int len )
{
	//char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char out[5];
    out[0] = st_b64[ in[0] >> 2 ];
    out[1] = st_b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? st_b64[ ((in[1] & 0x0f) << 2) |
             ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? st_b64[ in[2] & 0x3f ] : '=');
    out[4] = '\0';
    strcat((char *)b64str, (char *)out);
}

/*
	Encodes a string with Base64
*/
char* bb_base64_encode(char *clrstr)
{
	char *b64dst = (char*)qcom_mem_alloc(strlen(clrstr) + 50);
	//char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char in[3];
	int i, len = 0;
	int j = 0;

	b64dst[0] = '\0';
	while(clrstr[j])
	{
		len = 0;
		for(i=0; i<3; i++)
		{
			in[i] = (unsigned char) clrstr[j];
			if(clrstr[j])
			{
				len++; j++;
			}
			else in[i] = 0;
		}
		if( len )
		{
			encodeblock( in, b64dst, len );
		}
	}
	b64dst = (char*)qcom_mem_realloc(b64dst, strlen(b64dst) + 1);
	return b64dst;
}

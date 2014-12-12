/*
 * bb_base64.h
 *
 *  Created on: Oct 31, 2014
 *      Author: ethan
 */

#ifndef BB_BASE64_H_
#define BB_BASE64_H_

/*decode base64 string,  the caller is responsible for free the return buffer if there is any*/
char* bb_base64_decode(char *b64src);


/*encode base64 string,  the caller is responsible for free the return buffer if there is any*/
char* bb_base64_encode(char *clrstr);

#endif /* BB_BASE64_H_ */

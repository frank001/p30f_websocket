/* 
 * File:   base64.h
 * Author: frans
 *
 * Created on 1 maart 2016, 17:27
 */

#ifndef BASE64_H
#define	BASE64_H

int Base64decode(char *bufplain, const char *bufcoded);
int Base64encode(char *encoded, const char *string, int len);
int Base64encode_len(int);


#endif	/* BASE64_H */


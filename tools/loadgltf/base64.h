#ifndef _BASE64_H_
#define _BASE64_H_

#ifdef __cplusplus
extern "C" {
#endif


// https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
int base64decode(const char* in, size_t inLen, unsigned char* out, size_t* outLen);

#ifdef __cplusplus
}
#endif


#endif

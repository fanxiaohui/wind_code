
#ifndef TEA_H_
#define TEA_H_


extern unsigned short TeaEncrypt(unsigned char *src, unsigned short size_src, int key);

extern unsigned short TeaDecrypt(unsigned char *src, unsigned short size_src,  int key);

#endif /* TEA_H_ */

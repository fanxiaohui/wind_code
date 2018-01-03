#include "tea.h"
#include "sdkDef.h"

static void tea_encrypt(signed int *v, signed int *k) {
	int y = NShtonlInt32(v[0]), z = NShtonlInt32(v[1]), sum = 0, i;
	int delta = 0x0e3779b9; //0x9e3779b9;
	int a = (k[0]), b = (k[1]), c = (k[2]), d = (k[3]);

	for (i = 0; i < 8; i++) {
		sum += delta;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
	}
	v[0] = NShtonlInt32(y);
	v[1] = NShtonlInt32(z);
}

static void tea_decrypt(signed int *v, signed int *k) {
	int y = NShtonlInt32(v[0]), z = NShtonlInt32(v[1]), sum = 0xC6EF3720, i;
	int delta = 0x0e3779b9;
	int a = (k[0]), b = (k[1]), c = (k[2]), d = (k[3]);

	sum = delta << 3;
	for (i = 0; i < 8; i++) {
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		sum -= delta;
	}
	v[0] = NShtonlInt32(y);
	v[1] = NShtonlInt32(z);
}

unsigned short TeaEncrypt(unsigned char *src, unsigned short size_src, int key) {
	//unsigned char a = 0;
	unsigned short i = 0;
	unsigned short num = 0;

	//将明文补足为8字节的倍数
//	a = size_src % 8;
//	if (a != 0) {
//		for (i = 0; i < 8 - a; i++) {
//			src[size_src++] = 0;
//		}
//	}
	//加密

	int key2[4];
	key2[0] = key;
	key2[1] = key;
	key2[2] = key;
	key2[3] = key;

	num = size_src / 8;
	for (i = 0; i < num; i++) {
		tea_encrypt((signed int *) (src + i * 8), key2);
	}

	return size_src;
}

unsigned short TeaDecrypt(unsigned char *src, unsigned short size_src, int key) {
	unsigned short i = 0;
	unsigned short num = 0;

	//判断长度是否为8的倍数
//	if (size_src % 8 != 0) {
//		return 0;
//	}
	//解密

	int key2[4];
	key2[0] = key;
	key2[1] = key;
	key2[2] = key;
	key2[3] = key;

	num = size_src / 8;
	for (i = 0; i < num; i++) {
		tea_decrypt((signed int *) (src + i * 8), key2);
	}

	return size_src;
}


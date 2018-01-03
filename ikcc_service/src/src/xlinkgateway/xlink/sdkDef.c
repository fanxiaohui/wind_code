#include "sdkDef.h"



// 短整型大小端互换

#define BigLittleSwap16(A)  (((( short)(A) & 0xff00) >> 8) | \
		((( short)(A) & 0x00ff) << 8))

// 长整型大小端互换

#define BigLittleSwap32(A)  (((( int)(A) & 0xff000000) >> 24) | \
               ((( int)(A) & 0x00ff0000) >> 8) | \
               ((( int)(A) & 0x0000ff00) << 8) | \
               ((( int)(A) & 0x000000ff) << 24))

signed int NShtonlInt32(signed int s) {
	int i = 0x12345678;
	unsigned char *l = (unsigned char*) &i;
	if (*l == 0x78) {
		s = BigLittleSwap32(s);
	}
	return s;
}

signed short NShtonlInt16(signed short s) {
	int i = 0x12345678;
	unsigned char *l = (unsigned char*) &i;
	if (*l == 0x78) {
		s = BigLittleSwap16(s);
	}
	return s;
}

int UtilHexToString(char *string, const unsigned char *hexBuffer, const unsigned int hexlength) {
	const static char HexStringBuf[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int i = 0;
	for (i = 0; i < hexlength; i++) {
		string[i * 2] = HexStringBuf[(hexBuffer[i] & 0xf0) >> 4];
		string[i * 2 + 1] = HexStringBuf[hexBuffer[i] & 0x0F];
	}
	return i * 2;
}

unsigned char UtilStringToHex(char *string) {

	unsigned char t = 0;
	unsigned char int_data = 0;

	if (string[0] <= '9') {
		t = string[0] - '0';
	} else {
		t = string[0] - 'A' + 10;
	}
	if (string[1] <= '9') {
		int_data = t * 16 + (string[1] - '0');
	} else {
		int_data = t * 16 + (string[1] - 'A' + 10);
	}
	return int_data;
}


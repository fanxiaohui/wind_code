/*
 * XGTime.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGTIME_H_
#define XGTIME_H_


typedef unsigned long long int XGTimeMs;

extern XGTimeMs XGGetLocalTime(void);
extern void XGSleepMs(int ms);


#endif /* XGTIME_H_ */

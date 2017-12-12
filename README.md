wind_code
=========

including C,Shell ,Linux Drivers,Network ,etc...

1. leoman_bed
	wifi AP mode for debug
	7688，uart0,1,2, SD，i2c-rtc [sensor--buffer--SD-file--UIcmd-show]

2. net4g
	7628,5350,7688,nowifi
	connect to remote server; json protocol;
	U8300/9300CW 4G dial; online 24Hours (comgt,crond,pppd)
	OTA,webUI,gpio-timing-ctrl,uart-tcp
	
3. pos-salemachine
	7688, widora BIT4(serial boot OK) or HLK-RM08A（serial boot ok??）
	Fushi salemachine; VTS card protocol
	POS machine; 
	VTS <---> 7688 <---> POS
	
4. tcptty
	linkit7688
	Wifi, AP mode, sta mode;
	uboot Udisk upgrade;
	uart <---> local network server
	simply webUI for setting

	

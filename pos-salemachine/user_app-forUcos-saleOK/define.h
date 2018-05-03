#ifndef DEFINE_H
#define DEFINE_H
#include  "ucos_ii.h"
#include "stm32f2xx.h"
#include "system_stm32f2xx.h"


#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

/*系统中用到的结构定义*/
/*C语言中面向对象设计，主要解决数据和函数的脱节问题*/
#ifndef C_Class
#define C_Class struct
#endif



/*系统状态机*/
enum SysState
{
    State_Idle = 0, //空闲状态
    State_PutInCoin, //投币状态
    State_CaseChooseGood,//现金选货
    State_FreeChooseGood,//免费选货
    State_BetChooseGood,//中奖选货
    state_CaseChooseFood,
    State_Change,//找零
    State_COMM_Change,//工控机找零
    State_Setting,//设置
    State_Bet,//抽奖
    State_Card,//刷卡
    State_FoodCard,
    State_DoDeduct,//指示扣费
    State_TakeoutCoin,//指示出币
    State_TakeoutDrinkGood_Channel,//指示出货
    State_TakeoutDrinkGood_Type,//指示出货
    State_TakeoutDrink_Channel,//按货道出货
    State_TakeoutDrink_Type,//按类型出货
    State_TakeoutFood_Type,
    State_COMM_CARD,//用于工控机卡销售
};

/************************************************************************************/
/*CAN总线发送数据定义
ID:CAN总线发送时的ID号,用于区分发送的目标板
CANType:CANID扩展，用于区分发送的内容
CANTxBuff：CAN总线发送的时的数据
fTx: 是否为空
*/
struct CANTxStruct
{
    u8 ID;	  //
    u8 CANType;
    u8 CANTxBuff[8];
    u8 fTx;
};

/*系统错误结构
 ErrType：错误类型
 ErrPara：错误参数
*/
struct SysErrStruct
{
    u8 ErrType;
    u16 ErrPara;
};


/*CAN总线接收结构数组
RxMsg CAN总线接收结构
next  执行下一个结构指针
*/
struct CANRxList
{
    CanRxMsg RxMsg;
    struct CANRxList *next;
} ;

/*系统调试结构数组*/
struct SytTeststr
{
    u8 StrLen;//字符长度
    u8 *pStr; //字符串指针
};

/*通知扣款的结构
*/
struct COMM_Mfun_str
{
    u8 fDeal;		/*处理完毕，0处理完毕，1正在处理*/
    u16 DSN;/*延迟执行的序号*/
    u8 MassageType;	 //信息类型	 1~5 1；指示出币，2：指示出货 3：指示扣费 4：按货道出货 5：按类型出货
    u16 Data; //信息数据
    u16 DealRes; //处理结果
    /*0执行完成，动作完成；
    1 正在执行
    2 执行完毕，动作失败，失败原因，未有足够的零钱
    3执行完毕，动作失败，失败原因，无货
    4 执行完毕，动作失败，失败原因，参数不对，可能是价格不对，无此货道等
    */
} ;

struct OpenChannelstr
{
    u8 Channel;
    u8 SN;
};
/*由MDB处理任务传输状态数据*/
struct MDBDSstr
{
    u8 DSType;	//信息传输类型,
    u8 DSDate[4]; //传输数据
};


/*系统日志等级等级*/
enum SysLogLevel
{
    SYSLOGLEVEL_GNL = 0, //正常运行时的产生的日志  ,如投币，退币等正常操作
    SYSLOGLEVEL_WAR = 1, //警告产生的日志  ,板级错误，如硬币发生错误等
    SYSLOGLEVEL_ERR = 2,   //错误产生的日志  ，系统统计错误，如CAN发生丢包
    SYSLOGLEVEL_MAX = 3 //
};
/*系统日志子系统*/
enum SubSystem
{
    SUBSYSTEM_MDB = 0, //钱币操作时产的日志
    SUBSYSTEM_SYSCTR = 1, //系统日常操作的日志，如压缩机管理
    SUBSYSTEM_VEND = 2, //售卖过程产生的日志，如按键，打开出货装置，LED灯等
    SUBSYSTEM_SYSSETTING = 3, /*系统设定检查错误*/
    SUBSYSTEM_MAX = 4 //
};
struct SystemLog
{

    enum SubSystem SubSys;//子系统类型
    enum SysLogLevel LogLevel;//日志等级
    u8 FirLogType;//第一履历类型
    u8 SecLogType;//第二履历类型
    u16 LogData;
    u8 fFree;//空值状态
};


/*上位机通讯*/
struct RemoteComm
{
    u8  CommType;//通讯标示
    u8  StrLen;//字符串长度
    u8  SN;
    u8  CMD;
    u8  *pCommStrng;//字符串指针
};
struct SaveSystemLogFlash
{
    u8 FirLogType;//第一履历类型
    u8 SecLogType;//第二履历类型
    u16 LogData;
    u8 TimeStamp[6];
};

//#define   LED1(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_2,State)
//#define   LED2(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_3,State)
//#define   LED3(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_4,State)

#define   LED1	GPIO_WriteBit(GPIOE,GPIO_Pin_2,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2)))
#define   LED2	GPIO_WriteBit(GPIOE,GPIO_Pin_3,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3)))
#define   LED3	GPIO_WriteBit(GPIOE,GPIO_Pin_4,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_4)))

#define SoftVer        200//软件版本V1.0.2



/*常量定义*/
#define CANTXQSize     40   //CAN发送数据队列长度
#define SysErrQSize    40	//系统错误队列长度
#define SysLogQSize    40	//系统测试队列长度
#define LEDDSQSize     20	//LED显示队列长度
#define TIMEOUTQSize   50	//LED显示队列长度
#define COMMQSize      20	//LED显示队列长度
#define COMM_StateQSize  10   /*向工控机报告最新状态*/


#define KeyMaxLen          32//定义最大的按键个数	  
#define	ChannelMaxLen      50//定义最大货道个数
#define ChannelGroupMaxLen 10 //定义最大小组数目，统计用
#define ChannelUnionMaxLen 30//定义交替销售小组数目
#define MaxRoom            4 //最大库数目

#define MAXDRINKTYPE      100
#define MAXFOODTYPE        100
#define MAXFOODCHANNEL     255

#define MAXHTEMP           180 /*库最高温度*/
#define MAXLTEMP           80  /*	库最低温度*/

#define STX1 0X5A
#define STX2 0XA5

#define Soft 4		   /*定义系统中软件的个数*/

/*定义常量CAN总线通讯ID*/
/*定义发送至键盘板的ID*/
#define KEYLEDCANID      0X06 //按键控制器LED灯控制
#define KEYLEDCANTYPE_LED 0X01
#define KEYLEDCANTYPE_FLASH 0X02
//#define KEYLEDFLASHCANID 22 //按键控制LED灯闪烁控制

/*定义发送至前显示器的ID*/
#define LEDDSCANID             0X05 //前显示器显示通讯ID
#define LEDDSCANTYPE_NUM       0X01 //前显示器正常显示
#define LEDDSCANTYPE_BUZZER    0X02 //前显示器显示通讯ID ，蜂鸣器控制
#define LEDDSCANTYPE_HAND      0X03 //前显示显示遥控上的内容
#define LEDDSCANTYPE_HAND_FLASH 	   0X04 //前显示器遥控模式的闪烁

/*定义发送至IC基板的ID*/
#define ICBASECANID      0X07
#define ICBASETYPE  0X01//所有类型


/*定义发送给遥控的的ID*/
#define   HANDCANID          0X08  //遥控器通讯ID
#define   HANDCANTYPE_NUM	 0X01  //遥控显示数字
#define   HANDCANTYPE_FLASH	 0X02  //遥控器闪烁控制
#define   HANDCANTYPE_LED	 0X03  //遥控上的LED状态控制


//#define BUZZERSCANID  16 //前显示器显示通讯ID ，蜂鸣器控制

//#define HANDLEDDS1	30 //前显示器遥控器模式数据控制ID
//
//#define HANDCANID1	10 //遥控器数据用通讯ID  数据显示
//#define HANDCANID2	11 //遥控器数据用通讯ID，闪烁控制
//#define HANDCANID3	14 //遥控器数据用通讯ID. LED灯控制

/*从遥控器发送至主板的*/
#define FORM_HAND_SendCANID         15  // 发给主板的通讯ID
#define FORM_HAND_CANTYPE_KEY       0x01//通讯类型 按键动作
#define FORM_HAND_CANTYPE_SWI       0x02//通讯类型 开关动作
#define FORM_HAND_CANTYPE_TIMEOUT   0x03//通讯超时
/*从LED显示器发送至主板的*/
#define FORM_LEDDS_CANTXID          16//有前显示发送至主板
#define FORM_LEDDS_CANTYPE_LIGHT    0X01//发送环境光亮度
/*从IC基板发送至主板的*/
#define FORM_ICBASE_CANID                  17//有IC基板发送至主板的ID
#define FORM_ICBASE_CANTYPE_TEMP_H         0x01 //发送温度高4位
#define FORM_ICBASE_CANTYPE_TEMP_L         0x02 //发送温度低4位
#define FORM_ICBASE_CANTYPE_SELLOUTSTATE   0x03 //发送温度
#define FORM_ICBASE_CANTYPE_FANERR         0x04 //发送风扇错误
/*从用户按键发送至主板*/
#define   FORM_KEY_CANID 18			  /*从用户按键发送至主板ID*/
#define   FORM_KEY_CANTYPE_KEY    0X01 /*按键按下*/
#define   FORM_KEY_CANTYPE_TIMEOUT 0X02	/*	超时联络*/

/*定义U盘升级的时的CAN总线ID*/
#define LEDDSUPDATA	 200//前显示器升级ID
#define ICUPDATA     201//ic基板升级ID
#define HANDUPDATA   202//遥控器升级ID




/*
错误代码常量定义区{错误类型，错误等级}
等级1:严重错误，停止销售
等级2：一般错误，仅发出警告
*/
#define COIN_COMM_ERR {10,1}//硬币机通讯异常
#define COIN_CHANGLE_SW {15,1}//退币开关异常
#define COIN_CHANGLE_ERR {161}//找零不良
#define COIN_SAFE_SW     {17,1}//安全开关异常
#define COIN_EXAMINE_ERR {19,1}//硬币选别装置异常
#define COIN_5NULL_ERR   {20,1}//5角空管开关异常
#define COIN_10NULL_ERR  {21,1}//5角空管开关异常
#define DALAY_ERR        {30,1}//选择继电器不动作
#define SELECT_KEY_ERR   {34,1}//选择按钮故障
#define TIME_ERR         {50,2}// 计时器异常
#define MEMORIZER_ERR    {51,1}//存储器异常
#define NO_SHIPMENT_MODE {52,2}//未设定出货装置模式
#define NO_SET_SECKEY_ERR {54,2}//未设定选择按钮
#define MORE_PRICE_ERR    {59,2}//价格设定过多
#define	BUS_COMM_ERR      {60,1}//通讯线短路
#define CTRL_BOARD_ERR    {62,1}//电控盒通讯异常
#define PRINTER_COMM_ERR  {64,1}//打印机通讯异常
#define HAND_COMM_ERR     {66,2}//遥控器通讯异常
#define BILL_COMM_ERR     {71,1}//纸币机通讯异常
#define COOLHOT_COMM_ERR  {77,1}//冷却加热单元通讯异常
#define BILL_JAM_ERR      {80,1}//纸币堵塞
#define BILL_CHANGLE_ERR  {81,1}//纸币支付机构异常
#define BILL_FULL_ERR     {82,1}//纸币金库已经装满
#define BILL_IDENFITY_ERR {84,1}//纸币识别错误
#define BILL_OPEN_ERR     {86,1}//纸币金库打开
#define BILL_PULLOUT_ERR  {87,2}//纸币拔出异常
#define PRINTER_ERR       {130,2}//打印机异常
#define PRINTER_JAM       {131,2}//打印机堵纸
#define IC_SEC_KEY_ERR    {150,2}//IC选择按钮未设定
#define COOLHOT_SET_ERR   {250,1}//冷却/加热设定异常
#define TEMP_SENSER1_ERR  {251,1}//温度热敏电阻1异常
#define TEMP_SENSER2_ERR  {252,1}//温度热敏电阻2异常
#define TEMP_SENSER3_ERR  {253,1}//温度热敏电阻3异常
#define TEMP_SENSER4_ERR  {254,1}//温度热敏电阻4异常
#define TEMP_SENSER5_ERR  {255,1}//温度热敏电阻5异常
#define TEMP_SENSER6_ERR  {256,1}//温度热敏电阻6异常
#define TEMP_SENSER7_ERR  {257,1}//温度热敏电阻7异常
#define TEMP_SENSER8_ERR  {258,1}//温度热敏电阻8异常
#define EXFAN_ERR         {300,2}//压缩机风扇错误 /*临时为2，硬件暂时没有实现*/
#define CONFIG_MODE_ERR   {2362,1}//构造模式异常

#define LEDDDS_COMM_ERR   {3000,1}//前显示器通讯异常
#define USEKEY_COMM_ERR   {3001,2}//按键控制通讯异常
#define SYSMEM_ERR        {3002,1}//系统存储器错误

#define CHANNEL_PRICE1_ERR  {3010,2}//有货道未设定现金价格
#define CHANNEL_PRICE2_ERR  {3011,2}//有货道未设定卡价格

#define CLEAR_ERR 0X8000//消除错误

#define CashVendMode 0
#define BetVendMode 1
#define FreeVendMode 2
#define BetTestVendMode 3
#define TestVendMode 4
#define CardVendMode 5


#define MaxPayoutTime  600/*最大自动退币时间60S*/
#define MaxClearAmount 1200/*最大强制清空用于余额时间120S*/


/*定义系统LOG信息*/
#define  PUTCOIN  0X11 /*硬币投币数据*/
#define  PAYCOIN 0X12/*硬币找零数据*/
#define  PAYLEFT  0X13/*找零剩余*/


#define  PUTBILL 0X21 /*纸币投币数据*/
#define  PAYBILL 0X22 /*纸币找零数据*/
#define  PUTBILLBOX 0X23/*纸币进入钞箱*/
/*工控机通讯*/
#define SYSSTATE		0X01 /*状态查询*/
#define CHECKRES        0X02/*查询延时执行的结果*/
#define CHECKACCOUNT    0X03 /*查询用户余额*/
#define SETRUNMODE      0X20 /*设置运行模式*/
#define SETDRINKTYPE    0X21 /*设置类型个数*/
#define SETTYPEPRICE    0X22/*设置商品类型价格*/
#define SETTYPECODE     0X23/*设置商品类型代码*/
#define SETCHANNELPARA  0X24/*设置商品类型代码*/
#define SETROOMTEMP     0X31/*设置库温度*/

#define REPORTRUNMODE      0X40 /*获取运行模式*/
#define REPORTDRINKTYPE	   0X41 /*获取类型个数*/
#define REPORTDRINKTYPEPRICE    0X42/*获取饮料类型价格*/
#define REPORTTYPECODE     0X43/*获取类型代码*/
#define REPORTCHANNELPARA  0X44 /*获取货道属性*/
#define REPORTMACHINECODE  0X45 /*获取机器构造代码*/
//#define REPORTPYEVALID     0X46 /*获取类型是否可售*/
#define REPORTFOOFPRICE     0X46 /*获取类食品机类型价格*/


#define REPORTROOMRUNMODE  0X50 /*获得库的工作方式*/
#define REPORTROOMTEMP	   0X51/*获取库的设置温度*/
#define REPORTROOMREALTEMP 0X52 /*	获取库的实际温度*/
#define REPORTSELLOUTSTATE 0X60 /*获取售空状态*/
#define REPORTRUNERR       0X61 /*获取故障*/
#define REPORTCHANGENUM    0X62 /*获取零钱数目*/
#define REPORTTYPESELLOUT   0X63 /*获取类型的售空信息*/

//extern  OS_EVENT  *CANTXQSem;
//extern	OS_EVENT  *SysErrQSem;
extern	OS_EVENT  *SysLogQSem;
extern	OS_EVENT  *LEDDSQSem;
extern  OS_EVENT  *TIMEOUTQSem;  //系统通讯超时用
extern  OS_EVENT  *COMMQSem;
extern OS_EVENT  *COMM_StateQSem;  //用户报告上位机查询的系统状态

extern OS_EVENT *KEYBOX;
extern OS_EVENT *COMM_Mfun_BOX;
extern OS_EVENT *fOpenChannelOver; //出货成功返回标示
extern OS_EVENT *BillSwithBOX;
extern OS_EVENT *WriteFlashBOX;
extern OS_EVENT *OpenchannelBOX;   //打开货道出货信箱
extern OS_EVENT *CardVendBOX;   //卡销售信箱
extern OS_EVENT *BetVendBOX;   //中奖销售信箱
extern OS_EVENT *FreeVendBOX;   //免费销售信箱
extern OS_EVENT *ENKeyBOX;   //使能按键信箱

extern OS_EVENT *LOGMutex;//系统日志互斥信号量
extern OS_EVENT *BuzzMutex;//蜂鸣器控制互斥信号量
extern OS_EVENT *CommMutex;//通讯互斥信号量
extern OS_EVENT *OpenChannelMutex;//系统打开货道互斥信号量


extern OS_EVENT *USART1Mutex;//系统串口1发送信号量
extern OS_EVENT *USART2Mutex;//系统串口2发送信号量
extern OS_EVENT *USART3Mutex;//系统串口3发送信号量
extern OS_EVENT *USART4Mutex;//系统串口4发送信号量
extern OS_EVENT *USART5Mutex;//系统串口5发送信号量
extern OS_EVENT *CANMutex;//系统串口5发送信号量

//extern	OS_MEM *CommRAMA_Ptr;
//extern	OS_MEM *CommRAMB_Ptr;


extern u32 SysTime;//系统时间节拍 0.1s一次

//extern u8 SysKeyLen;//用户按键数量
//extern u8 SysChannelLen;//货道长度
//extern u8 SysSuspendTime; //暂停销售时间
//extern u16 AllSysSetPra.MachineType;//构造模式
//extern u16 SysErr;

extern u8 NowTime[6];

//extern u8  UseKeyToChannel[KeyMaxLen+1];//用户按键对应的货道号

extern u16 SysErrCode[100][2];//故障代码
//extern u8  UseKeyToChannel[KeyMaxLen+1];//用户按键对应的货道号
//
//extern u8  AllSysSetPra.ChannelToGroup[ChannelGroupMaxLen+1][ChannelMaxLen+1];//货道小组
//extern const struct SystemParaStr SystemParaType[10];

struct SystemParaStr
{
    u8 SysKeyLen;//用户按键数量
    u8 SysChannelLen;//货道长度
    u8 Rooms;//有几个库，最大为4
    u8 RoomPara[4];//库属性，0，库无效，1，制热，2制冷，3，制热制冷

};
/*设置食品机的类型*/
struct FoodSystemParaStr
{
    u16 SysFoodType;//食品的种类
    u16 SysFoodChanel;//货道的个数
};
/*食品机货道属性*/
struct FoodChanelParaStr
{
    u8 FoodChanelState;//状态，0 已售 1可售
    u16 FoodType;//食品类型

};
//
struct FoodTypeStr
{
    u16 FoodTypeCode;//食品代码代码
    u16 FoodTypePrice;//食品价格  以角为单位
    u8  FoodTypeValid;//食品是否可售，用于内部条件检查，0 不可售 1可售
    u8  FoodTypeEn;//用于外部控制是否可售
};


/*系统固有属性*/
struct ChannelIMMPara
{
    u8  ChannelRoom;//属于哪个库
    u8 ChannelaType;//货道类型，0分为不分割合并， 1可分割合并
};
/*货道类型，分为主货道，附属货道，在货道设置中设置过对应按键的货道属于附属货道，否则为附属货道，*/

struct Channelpara
{
    u8 ChannelState;//货道状态，是否为可售状态，0，正常，1暂停出货，2售空，停止出货
    u8 ChannelNull;//货道标示是否为空 0：售空 1，未售空
    u8 ChannelNullSell;//货道标示为空后，已经销售的次数
    u8	ChannelGroup;//小组分组	  交替出货用
    u8  ChannelOpenClass;//交替出货类，属于同一交替销售销售，或者复数货道都可以归于此类中。
    u8  fChannelOpen;//出货标示，交替出货用
    u8  SuspendON;//暂停销售时间是否开启，0不开启 1，开启
    u32 SuspendStartTime;//货道暂停销售开始时间
    u8  VendTime[3];//销售时间，  VendTime[0]，时间带1，VendTime[1]，时间带2，VendTime[2]，时间带3
    u8  VendTimeMode;//一周销售模式
    u8  ChannelCut;//货道是否分割  1分割，0合并
    u8  MulMainCHannel;//复数货道注册的主按键值
    u8  ChannelKey;//复数货道分配的按键值
    u16 ChannelPrice1;//价格1
    u16 ChannelPrice2;//价格2
    u16 ChannelPrice3;//价格3
    u32 ChannelCode;//销售代码
    u16 LCDKey;//对应的LCD上的键值
    u16  ChannelDrinkTyp;/*货道对应的饮料类型，对应于LCD型*/
};

struct DrinkTypeStr
{
    u16 DrinkTypeCode;//饮料代码代码
    u16 DrinkTypePrice;//饮料价格  以角为单位
    u8  DrinkTypeValid;//饮料是否可售，用于内部条件检查	  0,不可售 1可售
    u8  DrinkTypeEn;//用于外部控制是否可售				  0 不可售 1可售
    u16 DrinkTypeLSChannel;/*该类型上次出货的货道*/
};

/*用户按键属性*/
struct UseKeyPara
{
    u8  UseKeyToChannel[KeyMaxLen + 1]; //按键注册的货道
    u8  UseKeyVal[KeyMaxLen + 1]; //按键是否有效  ,0此按键下无可销售货道,仅包括直接注册货道和复数货道
    u8  Roulette[KeyMaxLen + 1]; //是否轮盘赌按键，0，不是，1是
    u8  UseKeyLed[KeyMaxLen + 1]; //按键是否有效，包括本身所属的货道以及交替销售货道
};
//extern struct UseKeyPara AllSysSetPra.UseKeyState;//用户按键属性
/*各种运行中用到的模式*/
struct SysRunSettingPara
{
    u8 WeekVendMode[7];//一周内销售模式
    u8 WeekCoolMode[7];//一周内制冷模式
    u8 WeekHotMode[7];//一周内加热模式
    u8 SusPendTime;//系统暂停销售时间
    u8 SusPendMode;//系统暂停销售方式 ，0自动，1 不自动
    u8 LampRunTime[4];//日光灯运行时间 分别时间开始时间小时，分钟，结束时间小时，分钟
    u8 LampONOFF; //0，关闭，1，常开，2，自动
    u8 EnergyONOFF;//0，智能节能为关，1，打开
    u8 EnergySave;//0，日光灯，1风扇，3日光灯风扇，4关闭
    /*日光灯自动时运行模式，
    0，根据时间（条件1）设定，关闭灯
    1，根据光线传感器（条件2），环境光暗时，打开灯
    2，根据光线传感器（条件2），环境光亮时，打开灯
    3，条件1,条件满足一个，关闭灯
    4，条件1，条件2同时满足，关闭灯
    */
    u8 LampRunMode;
    u8 HotTemp[4][2];//加热温度	 [下限，加热开启温度][上限，加热停止温度]
    u8 CoolTemp[4][2];//制冷温度 [上限，制冷开启温度][下限，制冷停止温度]
    u8 MefrostTemp;//停止除霜模式温度
    u8 DefaultRoomRunMode[4];//库的运行方式，0，加热，1制冷
    u8 LevelCool;//制冷等级
    u8 FreeVend;//免费销售，0，不免费，1，免费
    u8 CoolTempLevel[4]; //制冷等级
    u8 HotTempLevel[4];	 // 制热等级
    u8 SelloutMode;//售空后销售1瓶，0，不设定，1，设定
    u8 NumContinueVend;//连续销售个数
    u8 PayoutTime; //自动退币时间
    u16 WinRate;//中奖率
    u16 LampLight;//环境光设定，低于这个值时开启日光灯
    u16 MaxMoney;//最大收钱
    u8  MaxBill;//最大纸币收钱个数
    u8 ForceVend;//强制购买
    u8 BillStopTime[2];//纸币机禁止时间带，开始和结束时间，小时，最大23
    u8 SysEnableCoin;//容许硬币机 0，使能，1禁止，系统中是否容许硬币机
    u8 SysEnalbeBill; //容许纸币机 0，使能，1禁止，系统中是否容许纸币机
    enum SubSystem SystemTestMode;//系统测试模式，1，测试售卖过程，2测试中间运行
    u8 SystemVendMdoe;//0,现金销售，1，卡销售，2，现金和卡同时销售
    u8 CompressorDalayTime;//压缩机延时启动时间
    u16 DrinkType;/*饮料机类型个数*/
    u16 FoodType;/*饮料机类型个数*/

};


/*运行实时参数*/
struct SysRunRealPara
{
    u8  SysRunMode;//系统运行模式，0，待机模式中，1，遥控器控制模式中 2，销售模式中
    u8  VendState;//系统状态，0正常，1，停止销售
    u8  BetMode;//0，不是工作在中奖模式,1 中奖模式
    u8  Betting;//正在抽奖
    u8  Coin5State;//5角硬币状态，0正常，1找零不足
    u8  Coin10State;//1元硬币状态，0，正常，1，找零不足
    u8  BillState;//纸币状态，0，正常，1，停止收纸币
    u8  fCoin_Exist;//硬币机存在 0，不存在   1存在
    u8  fBill_Exist;//纸币机存在 0，不存在   1存在
    u8  fMefrost;//0,没有在除霜，1，正在除霜
    u16 UserCount;//用户余额
    u8  ChannelNullTimes[ChannelMaxLen + 1]; //货道售空开关变为0时，货道内还有多少货
    u8  DoorState;//门状态
    u16 Temperature[8];//温度传感器【库1内温度】【库1蒸发器温度】【库2内温度】【库2蒸发器温度】【库3内温度】【库3蒸发器温度】【库4内温度】【库4蒸发器温度】
    u16 AmbientLight;//环境光亮度
    u8  fBillStop;//纸币机禁止标示,0禁止，1禁止
    u8  RoomRunMode[4]; //库运行模式，0，停止，1加热，2制冷
    u8  fLamp;//0日光灯灭，1日光灯亮
    u32 SuspendStartTime;//暂停开始时间，系统内部计数器为基准
    u32 SystemstorageRDloc;//  写入系统储存位置
    u32 SystemstorageWDloc;//  读取系统储存位置
    u8  UnconditionalState;//是否在强制状态 ，0不是，1是的
    u8 fChannelUnion[ChannelMaxLen + 1]; //交替小组是否存在可以销售的货道
    u16 SoftVersion[8]; //0,主板，1 IC基板，2 前显示器，3遥控器
    u16 ContVendTimes;//连续购买的次数
    u8 CommMode;/*售货机运行状态，0：正常状态，1：设定状态*/
    u32 SelloutState[2];
};

/*时间带*/
struct FunRunTime
{
    u8 RunTimeID;
    u8 StartTimeH;
    u8 StartTimeM;
    u8 EndTimeH;
    u8 EndTimeM;
    u8 fValid;
};

struct SysSetPraStr
{
    u16  MachineType;//构造模式;
    //u8  UseKeyToChannel[KeyMaxLen+1];//用户按键对应的货道号
    u8   ChannelToGroup[ChannelGroupMaxLen + 1][ChannelMaxLen + 1]; //货道小组
    struct UseKeyPara  UseKeyState;//用户按键属性
    /*结构数组定义区*/
    struct SystemParaStr      SystemPara;//系统基本属性，系统构造
    struct FoodSystemParaStr  FoodSystemPara;//食品机的参数
    struct ChannelIMMPara     ChnnelIMM[ChannelMaxLen + 1]; //货道固有属性	。与系统构造相关
    struct Channelpara        Channels[ChannelMaxLen + 1]; //货道属性
    struct DrinkTypeStr       DrinkTypes[MAXDRINKTYPE + 1]; /*饮料类型*/
    struct FoodChanelParaStr  FoodChanels[MAXFOODCHANNEL + 1]; //食品机货道属性
    struct FoodTypeStr        FoodTypes[MAXFOODTYPE + 1];			 //食品机销售类型

    struct FunRunTime         VendTimes[22];  //销售时间表
    struct FunRunTime         CoolTimes[22];  //制冷时间表
    struct FunRunTime         HotTimes[22];	 //加热时间表
    struct SysRunSettingPara  SystemSettingPara;	//系统设定的状态
};
extern struct SysSetPraStr AllSysSetPra;

struct VendLog
{
    u8   fValid;
    u8   RTCTime[6];
    u16   PutinMoney;
    u16   VendMoeny;
    u16   PayoutMoney;
    u16   RemainMoney;
};


//extern struct Channelpara AllSysSetPra.Channels[ChannelMaxLen+1];

/*统计数据*/
struct StatData
{


    u32 PeriodVendData;  //期间总销售个数 ，可以清除
    u32 PeriodVendMoneyData;  //期间总销售钱数 ，可以清除
    u32 ChannelVendData[ChannelMaxLen + 1]; //期间各个货道销售个数数据 ，可以清除
    u32 ChannelVendMoneyData[ChannelMaxLen + 1]; //期间各个货道销售钱数数据 ，可以清除
    u32 GroupVendData[ChannelGroupMaxLen + 1]; //期间小组销售个数数据统计   ，可以清除
    u32 GroupVendMoneyData[ChannelGroupMaxLen + 1]; //期间小组销售钱数数据统计   ，可以清除
    u16 ChannelMaxPrice[11]; //货道价格最高的10个数据,价格从高到低排序
    u32	PriceVendData[11];//期间各价格销售个数数据 ，可以清除
    u32	PriceVendMoneyData[11];//期间各价格销售个数数据 ，可以清除
    u32	PrizeVendData;//期间中奖销售个数数据 ，可以清除
    u32	PrizeVendMoneyData;//期间中奖销售个数数据 ，可以清除
    /*以下不可清除*/
    u32 TotalVendData;//累计销售个数，不能清除
    u32 TotalVendMoneyData;//累计销售钱数，不能清除
    u32 TestVendData;//测试销售出货个数    不能清除
    u32 CashVendData;//现金销售的个数     不能清除
    u32 CashVendMoneyData;//现金销售的钱数  不能清除
    u32 NotCashVendData;//非现金销售的个数   不能清除
    u32 NotCashVendMoneyData; //非现金销售的钱数  不能清除
    u32 HistoryVendData;//历史累计销售个数
    u32 HistoryVendMoneyData;//历史累计销售钱数
    u32 PrizeVendChannelData[ChannelMaxLen + 1];
    u32 PrizeVendChannelMoneyData[ChannelMaxLen + 1];

};
/*运行记录*/
struct RunLog
{
    u8 DoorOpenTimeLog[16][6];//过去10次开门时间
    u16 SysErrLog[16];	 //过去16次系统错误
    struct VendLog VendLogData[5];//过去5次的销售记录
};
extern struct SaveSystemLogFlash LogRDBuf, LogWDBuf;
extern struct SysRunRealPara SystemRealPara;
extern struct StatData VendData;
extern struct RunLog   RunlogData;
//
//extern struct FoodChanelParaStr AllSysSetPra.FoodChanels[MAXFOODCHANNEL+1]; //食品机货道属性
//extern struct FoodParaStr       AllSysSetPra.FoodTypes[MAXFOODTYPE+1];			 //食品机销售类型

//extern struct FoodSystemParaStr AllSysSetPra.FoodSystemPara;//食品机的参数
#endif

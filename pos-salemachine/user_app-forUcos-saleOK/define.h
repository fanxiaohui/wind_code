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

/*ϵͳ���õ��Ľṹ����*/
/*C���������������ƣ���Ҫ������ݺͺ������ѽ�����*/
#ifndef C_Class
#define C_Class struct
#endif



/*ϵͳ״̬��*/
enum SysState
{
    State_Idle = 0, //����״̬
    State_PutInCoin, //Ͷ��״̬
    State_CaseChooseGood,//�ֽ�ѡ��
    State_FreeChooseGood,//���ѡ��
    State_BetChooseGood,//�н�ѡ��
    state_CaseChooseFood,
    State_Change,//����
    State_COMM_Change,//���ػ�����
    State_Setting,//����
    State_Bet,//�齱
    State_Card,//ˢ��
    State_FoodCard,
    State_DoDeduct,//ָʾ�۷�
    State_TakeoutCoin,//ָʾ����
    State_TakeoutDrinkGood_Channel,//ָʾ����
    State_TakeoutDrinkGood_Type,//ָʾ����
    State_TakeoutDrink_Channel,//����������
    State_TakeoutDrink_Type,//�����ͳ���
    State_TakeoutFood_Type,
    State_COMM_CARD,//���ڹ��ػ�������
};

/************************************************************************************/
/*CAN���߷������ݶ���
ID:CAN���߷���ʱ��ID��,�������ַ��͵�Ŀ���
CANType:CANID��չ���������ַ��͵�����
CANTxBuff��CAN���߷��͵�ʱ������
fTx: �Ƿ�Ϊ��
*/
struct CANTxStruct
{
    u8 ID;	  //
    u8 CANType;
    u8 CANTxBuff[8];
    u8 fTx;
};

/*ϵͳ����ṹ
 ErrType����������
 ErrPara���������
*/
struct SysErrStruct
{
    u8 ErrType;
    u16 ErrPara;
};


/*CAN���߽��սṹ����
RxMsg CAN���߽��սṹ
next  ִ����һ���ṹָ��
*/
struct CANRxList
{
    CanRxMsg RxMsg;
    struct CANRxList *next;
} ;

/*ϵͳ���Խṹ����*/
struct SytTeststr
{
    u8 StrLen;//�ַ�����
    u8 *pStr; //�ַ���ָ��
};

/*֪ͨ�ۿ�Ľṹ
*/
struct COMM_Mfun_str
{
    u8 fDeal;		/*������ϣ�0������ϣ�1���ڴ���*/
    u16 DSN;/*�ӳ�ִ�е����*/
    u8 MassageType;	 //��Ϣ����	 1~5 1��ָʾ���ң�2��ָʾ���� 3��ָʾ�۷� 4������������ 5�������ͳ���
    u16 Data; //��Ϣ����
    u16 DealRes; //������
    /*0ִ����ɣ�������ɣ�
    1 ����ִ��
    2 ִ����ϣ�����ʧ�ܣ�ʧ��ԭ��δ���㹻����Ǯ
    3ִ����ϣ�����ʧ�ܣ�ʧ��ԭ���޻�
    4 ִ����ϣ�����ʧ�ܣ�ʧ��ԭ�򣬲������ԣ������Ǽ۸񲻶ԣ��޴˻�����
    */
} ;

struct OpenChannelstr
{
    u8 Channel;
    u8 SN;
};
/*��MDB����������״̬����*/
struct MDBDSstr
{
    u8 DSType;	//��Ϣ��������,
    u8 DSDate[4]; //��������
};


/*ϵͳ��־�ȼ��ȼ�*/
enum SysLogLevel
{
    SYSLOGLEVEL_GNL = 0, //��������ʱ�Ĳ�������־  ,��Ͷ�ң��˱ҵ���������
    SYSLOGLEVEL_WAR = 1, //�����������־  ,�弶������Ӳ�ҷ��������
    SYSLOGLEVEL_ERR = 2,   //�����������־  ��ϵͳͳ�ƴ�����CAN��������
    SYSLOGLEVEL_MAX = 3 //
};
/*ϵͳ��־��ϵͳ*/
enum SubSystem
{
    SUBSYSTEM_MDB = 0, //Ǯ�Ҳ���ʱ������־
    SUBSYSTEM_SYSCTR = 1, //ϵͳ�ճ���������־����ѹ��������
    SUBSYSTEM_VEND = 2, //�������̲�������־���簴�����򿪳���װ�ã�LED�Ƶ�
    SUBSYSTEM_SYSSETTING = 3, /*ϵͳ�趨������*/
    SUBSYSTEM_MAX = 4 //
};
struct SystemLog
{

    enum SubSystem SubSys;//��ϵͳ����
    enum SysLogLevel LogLevel;//��־�ȼ�
    u8 FirLogType;//��һ��������
    u8 SecLogType;//�ڶ���������
    u16 LogData;
    u8 fFree;//��ֵ״̬
};


/*��λ��ͨѶ*/
struct RemoteComm
{
    u8  CommType;//ͨѶ��ʾ
    u8  StrLen;//�ַ�������
    u8  SN;
    u8  CMD;
    u8  *pCommStrng;//�ַ���ָ��
};
struct SaveSystemLogFlash
{
    u8 FirLogType;//��һ��������
    u8 SecLogType;//�ڶ���������
    u16 LogData;
    u8 TimeStamp[6];
};

//#define   LED1(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_2,State)
//#define   LED2(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_3,State)
//#define   LED3(State)	GPIO_WriteBit(GPIOE,GPIO_Pin_4,State)

#define   LED1	GPIO_WriteBit(GPIOE,GPIO_Pin_2,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2)))
#define   LED2	GPIO_WriteBit(GPIOE,GPIO_Pin_3,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3)))
#define   LED3	GPIO_WriteBit(GPIOE,GPIO_Pin_4,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_4)))

#define SoftVer        200//����汾V1.0.2



/*��������*/
#define CANTXQSize     40   //CAN�������ݶ��г���
#define SysErrQSize    40	//ϵͳ������г���
#define SysLogQSize    40	//ϵͳ���Զ��г���
#define LEDDSQSize     20	//LED��ʾ���г���
#define TIMEOUTQSize   50	//LED��ʾ���г���
#define COMMQSize      20	//LED��ʾ���г���
#define COMM_StateQSize  10   /*�򹤿ػ���������״̬*/


#define KeyMaxLen          32//�������İ�������	  
#define	ChannelMaxLen      50//��������������
#define ChannelGroupMaxLen 10 //�������С����Ŀ��ͳ����
#define ChannelUnionMaxLen 30//���彻������С����Ŀ
#define MaxRoom            4 //������Ŀ

#define MAXDRINKTYPE      100
#define MAXFOODTYPE        100
#define MAXFOODCHANNEL     255

#define MAXHTEMP           180 /*������¶�*/
#define MAXLTEMP           80  /*	������¶�*/

#define STX1 0X5A
#define STX2 0XA5

#define Soft 4		   /*����ϵͳ������ĸ���*/

/*���峣��CAN����ͨѶID*/
/*���巢�������̰��ID*/
#define KEYLEDCANID      0X06 //����������LED�ƿ���
#define KEYLEDCANTYPE_LED 0X01
#define KEYLEDCANTYPE_FLASH 0X02
//#define KEYLEDFLASHCANID 22 //��������LED����˸����

/*���巢����ǰ��ʾ����ID*/
#define LEDDSCANID             0X05 //ǰ��ʾ����ʾͨѶID
#define LEDDSCANTYPE_NUM       0X01 //ǰ��ʾ��������ʾ
#define LEDDSCANTYPE_BUZZER    0X02 //ǰ��ʾ����ʾͨѶID ������������
#define LEDDSCANTYPE_HAND      0X03 //ǰ��ʾ��ʾң���ϵ�����
#define LEDDSCANTYPE_HAND_FLASH 	   0X04 //ǰ��ʾ��ң��ģʽ����˸

/*���巢����IC�����ID*/
#define ICBASECANID      0X07
#define ICBASETYPE  0X01//��������


/*���巢�͸�ң�صĵ�ID*/
#define   HANDCANID          0X08  //ң����ͨѶID
#define   HANDCANTYPE_NUM	 0X01  //ң����ʾ����
#define   HANDCANTYPE_FLASH	 0X02  //ң������˸����
#define   HANDCANTYPE_LED	 0X03  //ң���ϵ�LED״̬����


//#define BUZZERSCANID  16 //ǰ��ʾ����ʾͨѶID ������������

//#define HANDLEDDS1	30 //ǰ��ʾ��ң����ģʽ���ݿ���ID
//
//#define HANDCANID1	10 //ң����������ͨѶID  ������ʾ
//#define HANDCANID2	11 //ң����������ͨѶID����˸����
//#define HANDCANID3	14 //ң����������ͨѶID. LED�ƿ���

/*��ң���������������*/
#define FORM_HAND_SendCANID         15  // ���������ͨѶID
#define FORM_HAND_CANTYPE_KEY       0x01//ͨѶ���� ��������
#define FORM_HAND_CANTYPE_SWI       0x02//ͨѶ���� ���ض���
#define FORM_HAND_CANTYPE_TIMEOUT   0x03//ͨѶ��ʱ
/*��LED��ʾ�������������*/
#define FORM_LEDDS_CANTXID          16//��ǰ��ʾ����������
#define FORM_LEDDS_CANTYPE_LIGHT    0X01//���ͻ���������
/*��IC���巢���������*/
#define FORM_ICBASE_CANID                  17//��IC���巢���������ID
#define FORM_ICBASE_CANTYPE_TEMP_H         0x01 //�����¶ȸ�4λ
#define FORM_ICBASE_CANTYPE_TEMP_L         0x02 //�����¶ȵ�4λ
#define FORM_ICBASE_CANTYPE_SELLOUTSTATE   0x03 //�����¶�
#define FORM_ICBASE_CANTYPE_FANERR         0x04 //���ͷ��ȴ���
/*���û���������������*/
#define   FORM_KEY_CANID 18			  /*���û���������������ID*/
#define   FORM_KEY_CANTYPE_KEY    0X01 /*��������*/
#define   FORM_KEY_CANTYPE_TIMEOUT 0X02	/*	��ʱ����*/

/*����U��������ʱ��CAN����ID*/
#define LEDDSUPDATA	 200//ǰ��ʾ������ID
#define ICUPDATA     201//ic��������ID
#define HANDUPDATA   202//ң��������ID




/*
������볣��������{�������ͣ�����ȼ�}
�ȼ�1:���ش���ֹͣ����
�ȼ�2��һ����󣬽���������
*/
#define COIN_COMM_ERR {10,1}//Ӳ�һ�ͨѶ�쳣
#define COIN_CHANGLE_SW {15,1}//�˱ҿ����쳣
#define COIN_CHANGLE_ERR {161}//���㲻��
#define COIN_SAFE_SW     {17,1}//��ȫ�����쳣
#define COIN_EXAMINE_ERR {19,1}//Ӳ��ѡ��װ���쳣
#define COIN_5NULL_ERR   {20,1}//5�ǿչܿ����쳣
#define COIN_10NULL_ERR  {21,1}//5�ǿչܿ����쳣
#define DALAY_ERR        {30,1}//ѡ��̵���������
#define SELECT_KEY_ERR   {34,1}//ѡ��ť����
#define TIME_ERR         {50,2}// ��ʱ���쳣
#define MEMORIZER_ERR    {51,1}//�洢���쳣
#define NO_SHIPMENT_MODE {52,2}//δ�趨����װ��ģʽ
#define NO_SET_SECKEY_ERR {54,2}//δ�趨ѡ��ť
#define MORE_PRICE_ERR    {59,2}//�۸��趨����
#define	BUS_COMM_ERR      {60,1}//ͨѶ�߶�·
#define CTRL_BOARD_ERR    {62,1}//��غ�ͨѶ�쳣
#define PRINTER_COMM_ERR  {64,1}//��ӡ��ͨѶ�쳣
#define HAND_COMM_ERR     {66,2}//ң����ͨѶ�쳣
#define BILL_COMM_ERR     {71,1}//ֽ�һ�ͨѶ�쳣
#define COOLHOT_COMM_ERR  {77,1}//��ȴ���ȵ�ԪͨѶ�쳣
#define BILL_JAM_ERR      {80,1}//ֽ�Ҷ���
#define BILL_CHANGLE_ERR  {81,1}//ֽ��֧�������쳣
#define BILL_FULL_ERR     {82,1}//ֽ�ҽ���Ѿ�װ��
#define BILL_IDENFITY_ERR {84,1}//ֽ��ʶ�����
#define BILL_OPEN_ERR     {86,1}//ֽ�ҽ���
#define BILL_PULLOUT_ERR  {87,2}//ֽ�Ұγ��쳣
#define PRINTER_ERR       {130,2}//��ӡ���쳣
#define PRINTER_JAM       {131,2}//��ӡ����ֽ
#define IC_SEC_KEY_ERR    {150,2}//ICѡ��ťδ�趨
#define COOLHOT_SET_ERR   {250,1}//��ȴ/�����趨�쳣
#define TEMP_SENSER1_ERR  {251,1}//�¶���������1�쳣
#define TEMP_SENSER2_ERR  {252,1}//�¶���������2�쳣
#define TEMP_SENSER3_ERR  {253,1}//�¶���������3�쳣
#define TEMP_SENSER4_ERR  {254,1}//�¶���������4�쳣
#define TEMP_SENSER5_ERR  {255,1}//�¶���������5�쳣
#define TEMP_SENSER6_ERR  {256,1}//�¶���������6�쳣
#define TEMP_SENSER7_ERR  {257,1}//�¶���������7�쳣
#define TEMP_SENSER8_ERR  {258,1}//�¶���������8�쳣
#define EXFAN_ERR         {300,2}//ѹ�������ȴ��� /*��ʱΪ2��Ӳ����ʱû��ʵ��*/
#define CONFIG_MODE_ERR   {2362,1}//����ģʽ�쳣

#define LEDDDS_COMM_ERR   {3000,1}//ǰ��ʾ��ͨѶ�쳣
#define USEKEY_COMM_ERR   {3001,2}//��������ͨѶ�쳣
#define SYSMEM_ERR        {3002,1}//ϵͳ�洢������

#define CHANNEL_PRICE1_ERR  {3010,2}//�л���δ�趨�ֽ�۸�
#define CHANNEL_PRICE2_ERR  {3011,2}//�л���δ�趨���۸�

#define CLEAR_ERR 0X8000//��������

#define CashVendMode 0
#define BetVendMode 1
#define FreeVendMode 2
#define BetTestVendMode 3
#define TestVendMode 4
#define CardVendMode 5


#define MaxPayoutTime  600/*����Զ��˱�ʱ��60S*/
#define MaxClearAmount 1200/*���ǿ������������ʱ��120S*/


/*����ϵͳLOG��Ϣ*/
#define  PUTCOIN  0X11 /*Ӳ��Ͷ������*/
#define  PAYCOIN 0X12/*Ӳ����������*/
#define  PAYLEFT  0X13/*����ʣ��*/


#define  PUTBILL 0X21 /*ֽ��Ͷ������*/
#define  PAYBILL 0X22 /*ֽ����������*/
#define  PUTBILLBOX 0X23/*ֽ�ҽ��볮��*/
/*���ػ�ͨѶ*/
#define SYSSTATE		0X01 /*״̬��ѯ*/
#define CHECKRES        0X02/*��ѯ��ʱִ�еĽ��*/
#define CHECKACCOUNT    0X03 /*��ѯ�û����*/
#define SETRUNMODE      0X20 /*��������ģʽ*/
#define SETDRINKTYPE    0X21 /*�������͸���*/
#define SETTYPEPRICE    0X22/*������Ʒ���ͼ۸�*/
#define SETTYPECODE     0X23/*������Ʒ���ʹ���*/
#define SETCHANNELPARA  0X24/*������Ʒ���ʹ���*/
#define SETROOMTEMP     0X31/*���ÿ��¶�*/

#define REPORTRUNMODE      0X40 /*��ȡ����ģʽ*/
#define REPORTDRINKTYPE	   0X41 /*��ȡ���͸���*/
#define REPORTDRINKTYPEPRICE    0X42/*��ȡ�������ͼ۸�*/
#define REPORTTYPECODE     0X43/*��ȡ���ʹ���*/
#define REPORTCHANNELPARA  0X44 /*��ȡ��������*/
#define REPORTMACHINECODE  0X45 /*��ȡ�����������*/
//#define REPORTPYEVALID     0X46 /*��ȡ�����Ƿ����*/
#define REPORTFOOFPRICE     0X46 /*��ȡ��ʳƷ�����ͼ۸�*/


#define REPORTROOMRUNMODE  0X50 /*��ÿ�Ĺ�����ʽ*/
#define REPORTROOMTEMP	   0X51/*��ȡ��������¶�*/
#define REPORTROOMREALTEMP 0X52 /*	��ȡ���ʵ���¶�*/
#define REPORTSELLOUTSTATE 0X60 /*��ȡ�ۿ�״̬*/
#define REPORTRUNERR       0X61 /*��ȡ����*/
#define REPORTCHANGENUM    0X62 /*��ȡ��Ǯ��Ŀ*/
#define REPORTTYPESELLOUT   0X63 /*��ȡ���͵��ۿ���Ϣ*/

//extern  OS_EVENT  *CANTXQSem;
//extern	OS_EVENT  *SysErrQSem;
extern	OS_EVENT  *SysLogQSem;
extern	OS_EVENT  *LEDDSQSem;
extern  OS_EVENT  *TIMEOUTQSem;  //ϵͳͨѶ��ʱ��
extern  OS_EVENT  *COMMQSem;
extern OS_EVENT  *COMM_StateQSem;  //�û�������λ����ѯ��ϵͳ״̬

extern OS_EVENT *KEYBOX;
extern OS_EVENT *COMM_Mfun_BOX;
extern OS_EVENT *fOpenChannelOver; //�����ɹ����ر�ʾ
extern OS_EVENT *BillSwithBOX;
extern OS_EVENT *WriteFlashBOX;
extern OS_EVENT *OpenchannelBOX;   //�򿪻�����������
extern OS_EVENT *CardVendBOX;   //����������
extern OS_EVENT *BetVendBOX;   //�н���������
extern OS_EVENT *FreeVendBOX;   //�����������
extern OS_EVENT *ENKeyBOX;   //ʹ�ܰ�������

extern OS_EVENT *LOGMutex;//ϵͳ��־�����ź���
extern OS_EVENT *BuzzMutex;//���������ƻ����ź���
extern OS_EVENT *CommMutex;//ͨѶ�����ź���
extern OS_EVENT *OpenChannelMutex;//ϵͳ�򿪻��������ź���


extern OS_EVENT *USART1Mutex;//ϵͳ����1�����ź���
extern OS_EVENT *USART2Mutex;//ϵͳ����2�����ź���
extern OS_EVENT *USART3Mutex;//ϵͳ����3�����ź���
extern OS_EVENT *USART4Mutex;//ϵͳ����4�����ź���
extern OS_EVENT *USART5Mutex;//ϵͳ����5�����ź���
extern OS_EVENT *CANMutex;//ϵͳ����5�����ź���

//extern	OS_MEM *CommRAMA_Ptr;
//extern	OS_MEM *CommRAMB_Ptr;


extern u32 SysTime;//ϵͳʱ����� 0.1sһ��

//extern u8 SysKeyLen;//�û���������
//extern u8 SysChannelLen;//��������
//extern u8 SysSuspendTime; //��ͣ����ʱ��
//extern u16 AllSysSetPra.MachineType;//����ģʽ
//extern u16 SysErr;

extern u8 NowTime[6];

//extern u8  UseKeyToChannel[KeyMaxLen+1];//�û�������Ӧ�Ļ�����

extern u16 SysErrCode[100][2];//���ϴ���
//extern u8  UseKeyToChannel[KeyMaxLen+1];//�û�������Ӧ�Ļ�����
//
//extern u8  AllSysSetPra.ChannelToGroup[ChannelGroupMaxLen+1][ChannelMaxLen+1];//����С��
//extern const struct SystemParaStr SystemParaType[10];

struct SystemParaStr
{
    u8 SysKeyLen;//�û���������
    u8 SysChannelLen;//��������
    u8 Rooms;//�м����⣬���Ϊ4
    u8 RoomPara[4];//�����ԣ�0������Ч��1�����ȣ�2���䣬3����������

};
/*����ʳƷ��������*/
struct FoodSystemParaStr
{
    u16 SysFoodType;//ʳƷ������
    u16 SysFoodChanel;//�����ĸ���
};
/*ʳƷ����������*/
struct FoodChanelParaStr
{
    u8 FoodChanelState;//״̬��0 ���� 1����
    u16 FoodType;//ʳƷ����

};
//
struct FoodTypeStr
{
    u16 FoodTypeCode;//ʳƷ�������
    u16 FoodTypePrice;//ʳƷ�۸�  �Խ�Ϊ��λ
    u8  FoodTypeValid;//ʳƷ�Ƿ���ۣ������ڲ�������飬0 ������ 1����
    u8  FoodTypeEn;//�����ⲿ�����Ƿ����
};


/*ϵͳ��������*/
struct ChannelIMMPara
{
    u8  ChannelRoom;//�����ĸ���
    u8 ChannelaType;//�������ͣ�0��Ϊ���ָ�ϲ��� 1�ɷָ�ϲ�
};
/*�������ͣ���Ϊ�������������������ڻ������������ù���Ӧ�����Ļ������ڸ�������������Ϊ����������*/

struct Channelpara
{
    u8 ChannelState;//����״̬���Ƿ�Ϊ����״̬��0��������1��ͣ������2�ۿգ�ֹͣ����
    u8 ChannelNull;//������ʾ�Ƿ�Ϊ�� 0���ۿ� 1��δ�ۿ�
    u8 ChannelNullSell;//������ʾΪ�պ��Ѿ����۵Ĵ���
    u8	ChannelGroup;//С�����	  ���������
    u8  ChannelOpenClass;//��������࣬����ͬһ�����������ۣ����߸������������Թ��ڴ����С�
    u8  fChannelOpen;//������ʾ�����������
    u8  SuspendON;//��ͣ����ʱ���Ƿ�����0������ 1������
    u32 SuspendStartTime;//������ͣ���ۿ�ʼʱ��
    u8  VendTime[3];//����ʱ�䣬  VendTime[0]��ʱ���1��VendTime[1]��ʱ���2��VendTime[2]��ʱ���3
    u8  VendTimeMode;//һ������ģʽ
    u8  ChannelCut;//�����Ƿ�ָ�  1�ָ0�ϲ�
    u8  MulMainCHannel;//��������ע���������ֵ
    u8  ChannelKey;//������������İ���ֵ
    u16 ChannelPrice1;//�۸�1
    u16 ChannelPrice2;//�۸�2
    u16 ChannelPrice3;//�۸�3
    u32 ChannelCode;//���۴���
    u16 LCDKey;//��Ӧ��LCD�ϵļ�ֵ
    u16  ChannelDrinkTyp;/*������Ӧ���������ͣ���Ӧ��LCD��*/
};

struct DrinkTypeStr
{
    u16 DrinkTypeCode;//���ϴ������
    u16 DrinkTypePrice;//���ϼ۸�  �Խ�Ϊ��λ
    u8  DrinkTypeValid;//�����Ƿ���ۣ������ڲ��������	  0,������ 1����
    u8  DrinkTypeEn;//�����ⲿ�����Ƿ����				  0 ������ 1����
    u16 DrinkTypeLSChannel;/*�������ϴγ����Ļ���*/
};

/*�û���������*/
struct UseKeyPara
{
    u8  UseKeyToChannel[KeyMaxLen + 1]; //����ע��Ļ���
    u8  UseKeyVal[KeyMaxLen + 1]; //�����Ƿ���Ч  ,0�˰������޿����ۻ���,������ֱ��ע������͸�������
    u8  Roulette[KeyMaxLen + 1]; //�Ƿ����̶İ�����0�����ǣ�1��
    u8  UseKeyLed[KeyMaxLen + 1]; //�����Ƿ���Ч���������������Ļ����Լ��������ۻ���
};
//extern struct UseKeyPara AllSysSetPra.UseKeyState;//�û���������
/*�����������õ���ģʽ*/
struct SysRunSettingPara
{
    u8 WeekVendMode[7];//һ��������ģʽ
    u8 WeekCoolMode[7];//һ��������ģʽ
    u8 WeekHotMode[7];//һ���ڼ���ģʽ
    u8 SusPendTime;//ϵͳ��ͣ����ʱ��
    u8 SusPendMode;//ϵͳ��ͣ���۷�ʽ ��0�Զ���1 ���Զ�
    u8 LampRunTime[4];//�չ������ʱ�� �ֱ�ʱ�俪ʼʱ��Сʱ�����ӣ�����ʱ��Сʱ������
    u8 LampONOFF; //0���رգ�1��������2���Զ�
    u8 EnergyONOFF;//0�����ܽ���Ϊ�أ�1����
    u8 EnergySave;//0���չ�ƣ�1���ȣ�3�չ�Ʒ��ȣ�4�ر�
    /*�չ���Զ�ʱ����ģʽ��
    0������ʱ�䣨����1���趨���رյ�
    1�����ݹ��ߴ�����������2���������ⰵʱ���򿪵�
    2�����ݹ��ߴ�����������2������������ʱ���򿪵�
    3������1,��������һ�����رյ�
    4������1������2ͬʱ���㣬�رյ�
    */
    u8 LampRunMode;
    u8 HotTemp[4][2];//�����¶�	 [���ޣ����ȿ����¶�][���ޣ�����ֹͣ�¶�]
    u8 CoolTemp[4][2];//�����¶� [���ޣ����俪���¶�][���ޣ�����ֹͣ�¶�]
    u8 MefrostTemp;//ֹͣ��˪ģʽ�¶�
    u8 DefaultRoomRunMode[4];//������з�ʽ��0�����ȣ�1����
    u8 LevelCool;//����ȼ�
    u8 FreeVend;//������ۣ�0������ѣ�1�����
    u8 CoolTempLevel[4]; //����ȼ�
    u8 HotTempLevel[4];	 // ���ȵȼ�
    u8 SelloutMode;//�ۿպ�����1ƿ��0�����趨��1���趨
    u8 NumContinueVend;//�������۸���
    u8 PayoutTime; //�Զ��˱�ʱ��
    u16 WinRate;//�н���
    u16 LampLight;//�������趨���������ֵʱ�����չ��
    u16 MaxMoney;//�����Ǯ
    u8  MaxBill;//���ֽ����Ǯ����
    u8 ForceVend;//ǿ�ƹ���
    u8 BillStopTime[2];//ֽ�һ���ֹʱ�������ʼ�ͽ���ʱ�䣬Сʱ�����23
    u8 SysEnableCoin;//����Ӳ�һ� 0��ʹ�ܣ�1��ֹ��ϵͳ���Ƿ�����Ӳ�һ�
    u8 SysEnalbeBill; //����ֽ�һ� 0��ʹ�ܣ�1��ֹ��ϵͳ���Ƿ�����ֽ�һ�
    enum SubSystem SystemTestMode;//ϵͳ����ģʽ��1�������������̣�2�����м�����
    u8 SystemVendMdoe;//0,�ֽ����ۣ�1�������ۣ�2���ֽ�Ϳ�ͬʱ����
    u8 CompressorDalayTime;//ѹ������ʱ����ʱ��
    u16 DrinkType;/*���ϻ����͸���*/
    u16 FoodType;/*���ϻ����͸���*/

};


/*����ʵʱ����*/
struct SysRunRealPara
{
    u8  SysRunMode;//ϵͳ����ģʽ��0������ģʽ�У�1��ң��������ģʽ�� 2������ģʽ��
    u8  VendState;//ϵͳ״̬��0������1��ֹͣ����
    u8  BetMode;//0�����ǹ������н�ģʽ,1 �н�ģʽ
    u8  Betting;//���ڳ齱
    u8  Coin5State;//5��Ӳ��״̬��0������1���㲻��
    u8  Coin10State;//1ԪӲ��״̬��0��������1�����㲻��
    u8  BillState;//ֽ��״̬��0��������1��ֹͣ��ֽ��
    u8  fCoin_Exist;//Ӳ�һ����� 0��������   1����
    u8  fBill_Exist;//ֽ�һ����� 0��������   1����
    u8  fMefrost;//0,û���ڳ�˪��1�����ڳ�˪
    u16 UserCount;//�û����
    u8  ChannelNullTimes[ChannelMaxLen + 1]; //�����ۿտ��ر�Ϊ0ʱ�������ڻ��ж��ٻ�
    u8  DoorState;//��״̬
    u16 Temperature[8];//�¶ȴ���������1���¶ȡ�����1�������¶ȡ�����2���¶ȡ�����2�������¶ȡ�����3���¶ȡ�����3�������¶ȡ�����4���¶ȡ�����4�������¶ȡ�
    u16 AmbientLight;//����������
    u8  fBillStop;//ֽ�һ���ֹ��ʾ,0��ֹ��1��ֹ
    u8  RoomRunMode[4]; //������ģʽ��0��ֹͣ��1���ȣ�2����
    u8  fLamp;//0�չ����1�չ����
    u32 SuspendStartTime;//��ͣ��ʼʱ�䣬ϵͳ�ڲ�������Ϊ��׼
    u32 SystemstorageRDloc;//  д��ϵͳ����λ��
    u32 SystemstorageWDloc;//  ��ȡϵͳ����λ��
    u8  UnconditionalState;//�Ƿ���ǿ��״̬ ��0���ǣ�1�ǵ�
    u8 fChannelUnion[ChannelMaxLen + 1]; //����С���Ƿ���ڿ������۵Ļ���
    u16 SoftVersion[8]; //0,���壬1 IC���壬2 ǰ��ʾ����3ң����
    u16 ContVendTimes;//��������Ĵ���
    u8 CommMode;/*�ۻ�������״̬��0������״̬��1���趨״̬*/
    u32 SelloutState[2];
};

/*ʱ���*/
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
    u16  MachineType;//����ģʽ;
    //u8  UseKeyToChannel[KeyMaxLen+1];//�û�������Ӧ�Ļ�����
    u8   ChannelToGroup[ChannelGroupMaxLen + 1][ChannelMaxLen + 1]; //����С��
    struct UseKeyPara  UseKeyState;//�û���������
    /*�ṹ���鶨����*/
    struct SystemParaStr      SystemPara;//ϵͳ�������ԣ�ϵͳ����
    struct FoodSystemParaStr  FoodSystemPara;//ʳƷ���Ĳ���
    struct ChannelIMMPara     ChnnelIMM[ChannelMaxLen + 1]; //������������	����ϵͳ�������
    struct Channelpara        Channels[ChannelMaxLen + 1]; //��������
    struct DrinkTypeStr       DrinkTypes[MAXDRINKTYPE + 1]; /*��������*/
    struct FoodChanelParaStr  FoodChanels[MAXFOODCHANNEL + 1]; //ʳƷ����������
    struct FoodTypeStr        FoodTypes[MAXFOODTYPE + 1];			 //ʳƷ����������

    struct FunRunTime         VendTimes[22];  //����ʱ���
    struct FunRunTime         CoolTimes[22];  //����ʱ���
    struct FunRunTime         HotTimes[22];	 //����ʱ���
    struct SysRunSettingPara  SystemSettingPara;	//ϵͳ�趨��״̬
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

/*ͳ������*/
struct StatData
{


    u32 PeriodVendData;  //�ڼ������۸��� ���������
    u32 PeriodVendMoneyData;  //�ڼ�������Ǯ�� ���������
    u32 ChannelVendData[ChannelMaxLen + 1]; //�ڼ�����������۸������� ���������
    u32 ChannelVendMoneyData[ChannelMaxLen + 1]; //�ڼ������������Ǯ������ ���������
    u32 GroupVendData[ChannelGroupMaxLen + 1]; //�ڼ�С�����۸�������ͳ��   ���������
    u32 GroupVendMoneyData[ChannelGroupMaxLen + 1]; //�ڼ�С������Ǯ������ͳ��   ���������
    u16 ChannelMaxPrice[11]; //�����۸���ߵ�10������,�۸�Ӹߵ�������
    u32	PriceVendData[11];//�ڼ���۸����۸������� ���������
    u32	PriceVendMoneyData[11];//�ڼ���۸����۸������� ���������
    u32	PrizeVendData;//�ڼ��н����۸������� ���������
    u32	PrizeVendMoneyData;//�ڼ��н����۸������� ���������
    /*���²������*/
    u32 TotalVendData;//�ۼ����۸������������
    u32 TotalVendMoneyData;//�ۼ�����Ǯ�����������
    u32 TestVendData;//�������۳�������    �������
    u32 CashVendData;//�ֽ����۵ĸ���     �������
    u32 CashVendMoneyData;//�ֽ����۵�Ǯ��  �������
    u32 NotCashVendData;//���ֽ����۵ĸ���   �������
    u32 NotCashVendMoneyData; //���ֽ����۵�Ǯ��  �������
    u32 HistoryVendData;//��ʷ�ۼ����۸���
    u32 HistoryVendMoneyData;//��ʷ�ۼ�����Ǯ��
    u32 PrizeVendChannelData[ChannelMaxLen + 1];
    u32 PrizeVendChannelMoneyData[ChannelMaxLen + 1];

};
/*���м�¼*/
struct RunLog
{
    u8 DoorOpenTimeLog[16][6];//��ȥ10�ο���ʱ��
    u16 SysErrLog[16];	 //��ȥ16��ϵͳ����
    struct VendLog VendLogData[5];//��ȥ5�ε����ۼ�¼
};
extern struct SaveSystemLogFlash LogRDBuf, LogWDBuf;
extern struct SysRunRealPara SystemRealPara;
extern struct StatData VendData;
extern struct RunLog   RunlogData;
//
//extern struct FoodChanelParaStr AllSysSetPra.FoodChanels[MAXFOODCHANNEL+1]; //ʳƷ����������
//extern struct FoodParaStr       AllSysSetPra.FoodTypes[MAXFOODTYPE+1];			 //ʳƷ����������

//extern struct FoodSystemParaStr AllSysSetPra.FoodSystemPara;//ʳƷ���Ĳ���
#endif

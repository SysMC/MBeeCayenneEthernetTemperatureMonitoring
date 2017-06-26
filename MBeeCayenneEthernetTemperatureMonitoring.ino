/*********************************************************************
 * INCLUDES
 */
#include <SimpleTimer.h> 
#include <CayenneEthernet.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define VIRTUAL_PIN_TEMPERATURE1 V1
#define VIRTUAL_PIN_BATTERY1 V2
#define VIRTUAL_PIN_TEMPERATURE2 V3
#define VIRTUAL_PIN_BATTERY2 V4
#define VIRTUAL_PIN_TEMPERATURE3 V5
#define VIRTUAL_PIN_BATTERY3 V6

#define NODE_ID_SIZE 2

 /*********************************************************************
 * TYPEDEFS
 */
 
typedef struct
{
  uint8_t id[NODE_ID_SIZE];
}nodeID_t;
typedef struct
{
  nodeID_t destinationID;
  uint8_t channel;
  uint16_t dutyCycle;
}pwmRemoteControl_t;

typedef struct
{
  nodeID_t destinationID;
  uint8_t level;
}directionRemoteControl_t;

typedef struct
{
  uint8_t sof = 0x7E;
  uint8_t lengthMsb = 0x00;
  uint8_t lengthLsb;
}apiFrameHeader_t;

typedef struct 
{
  apiFrameHeader_t frameHeader; 
  uint8_t frameType;                
  nodeID_t source;               
  uint8_t rssi;                     
  uint8_t options;                  
  uint8_t temperature;              
  uint8_t vBatt;                   
  uint8_t checkSum;    
}receivePacket_t;

static receivePacket_t receivePacket;  //Объявляем переменную принятого пакета структурного типа receivePacket_t.
static int8_t temperature1;
static uint8_t Vbatt1;
static int8_t temperature2;
static uint8_t Vbatt2;
static int8_t temperature3;
static uint8_t Vbatt3;

/*********************************************************************
 * LOCAL VARIABLES
 */
char token[] = "Your Token"; //Устанавливаем ключ проекта на Cayenne Dashboard.

static const nodeID_t sensorID6 = {0x00, 0x06};
static const nodeID_t sensorID7 = {0x00, 0x07};
static const nodeID_t sensorID8 = {0x00, 0x08};

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * FUNCTIONS
 */
void setup()
{
  Serial.begin(9600); //Объявляем компонент Serial.
  Cayenne.begin(token); //Объявляем компонент Cayenne.
 }


void loop()
{
  Cayenne.run();
  if(Serial.available() >= sizeof(receivePacket_t)) //Если доступно сообщение в буфере.
    getSamplingData();
}

void getSamplingData(void)
{
  uint8_t i;  //Объявляем переменную цикла.
  uint8_t *buffer_p;  //Объявляем переменную указателя на буфер.
  uint8_t checkSum;  //Объявляем переменную контрольной суммы.
  buffer_p = (uint8_t*)&receivePacket;  //Объявляем переменную типа указатель на буфер с байтами.
  while(Serial.available()) //Ищем начало фрейма.
  {
    *buffer_p = Serial.read(); 
    if(*buffer_p == 0x7E)
      break;
  }
  if (*buffer_p++ == 0x7E)
  {
    for (i = 0; i < sizeof(receivePacket_t) - 1; i++) 
    {
      if(Serial.available())
       *buffer_p++ = Serial.read();  //Считываем байты из UART пока их количество не станет равным длине ожидаемого пакета минус байт уже считанного заголовка.
      else
       break;
    }
    if (i >= sizeof(receivePacket_t) - 1)
    {
      if (getCheckSum(&receivePacket.frameType, receivePacket.frameHeader.lengthLsb) == receivePacket.checkSum)  //Проверяем контрольные суммы принятого пакета.
      {
        if (nodeAddressesCompare(&receivePacket.source, &sensorID6))
        {
          temperature1 = receivePacket.temperature;
          Vbatt1 = receivePacket.vBatt;
        }
        if (nodeAddressesCompare(&receivePacket.source, &sensorID7))
        {
          temperature2 = receivePacket.temperature;
          Vbatt2 = receivePacket.vBatt;
        }
        if (nodeAddressesCompare(&receivePacket.source, &sensorID8))
        {
          temperature3 = receivePacket.temperature;
          Vbatt3 = receivePacket.vBatt;
        }
      }
    }
  }
  buffer_p = (uint8_t*)&receivePacket; //Инициализируем указатель буфера.
  while(Serial.available()) //Чистим буфер.
    *buffer_p = Serial.read(); 
}

uint8_t getCheckSum(uint8_t* buffer_p, uint8_t len) 
{
  unsigned int checkSum = 0;  //Объявляем переменную для хранения контрольной суммы.
  uint8_t count;  //Объявляем переменную счетчика байт.
  for (count = 0; count < len; count++) //Пока счетчик меньше длины пакета к нему прибавляется по единичке.
  {
    checkSum += *buffer_p; //К контрольной сумме будет прибавляться очередное число считанное по буферу.
    buffer_p++;  //Указатель следующего считываемого байта увеличивается на 1.
  }
  return 0xFF - checkSum & 0x00FF;  //Функция возвращает разницу 0xFF и младшего байта контрольной суммы.
}

bool nodeAddressesCompare(nodeID_t *addr1_p, nodeID_t *addr2_p)
{
  if (((addr1_p->id[0]) == (addr2_p->id[0])) && ((addr1_p->id[1]) == (addr2_p->id[1])))
    return true; 
  else
    return false;
}

CAYENNE_OUT (VIRTUAL_PIN_TEMPERATURE1) 
{
  Cayenne.celsiusWrite(VIRTUAL_PIN_TEMPERATURE1, temperature1);
}

CAYENNE_OUT(VIRTUAL_PIN_BATTERY1)
{
  float voltage = (float)Vbatt1/51;
  Cayenne.virtualWrite(VIRTUAL_PIN_BATTERY1, voltage);
}


CAYENNE_OUT (VIRTUAL_PIN_TEMPERATURE2) 
{
  Cayenne.celsiusWrite(VIRTUAL_PIN_TEMPERATURE2, temperature2);
}

CAYENNE_OUT(VIRTUAL_PIN_BATTERY2)
{
  float voltage = (float)Vbatt2/51;
  Cayenne.virtualWrite(VIRTUAL_PIN_BATTERY2, voltage);
}


CAYENNE_OUT (VIRTUAL_PIN_TEMPERATURE3) 
{
  Cayenne.celsiusWrite(VIRTUAL_PIN_TEMPERATURE3, temperature3);
}

CAYENNE_OUT(VIRTUAL_PIN_BATTERY3)
{
  float voltage = (float)Vbatt3/51;
  Cayenne.virtualWrite(VIRTUAL_PIN_BATTERY3, voltage);
}

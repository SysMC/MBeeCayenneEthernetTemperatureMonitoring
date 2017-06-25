//ВНИМАНИЕ! Компилятор Arduino некорректно и определяет длину перечислимого типа enum, всегда считая его равным 2 байта. Поэтому для правильной работы функции sizeof в структурах и union'ах, если максимальная длина элемента 
//перечислимого типа помещается в 1 байт, ВСЕГДА надо использовать вместо enum тип uint8_t (byte).

#define NODE_ID_SIZE 2
#define MAX_APP_PAYLOAD 50
#define PWM_DUTY_CYCLE_MAX 13000
/*********************************************************************
Параметры API-режима
*********************************************************************/
//Параметры поля Transmit options
#define NO_TRANSMIT_OPTIONS 0
#define DISABLE_ACKNOWLEDGE_BIT 0

//Параметры поля Remote command options
#define NO_REMOTE_COMMAND_OPTION              0 //Записывает параметр в теневой регистр без применения.
#define APPLY_CHANGES_NO_SAVE_COMMAND_OPTION  1 //Применяет сделанные изменения без сохранения их во флеш.
#define APPLY_CHANGES_COMMAND_OPTION          2 //Применяет сделанные изменения и записывает во флеш обновленные параметры.

//Параметры поля Receive options
#define PACKET_ACKNOWLEDGED_BIT 0
#define PACKET_WAS_BROADCAST_BIT  1

#define API_FRAME_START_DELIMITER 0x7E
#define RF_PAYLOAD_MAX_LENGTH (MAX_APP_PAYLOAD - sizeof(route_t)) //Максимальная полезная нагрузка эфирного пакета.
#define TX_REQUEST_PAYLOAD_MAX_LENGTH (RF_PAYLOAD_MAX_LENGTH - sizeof(txRequestRfPayloadHeader_t)) //Максимальная нагрузка эфирного пакета типа TRANSMIT_REQUEST_API_FRAME

#define API_FRAME_PARAMETER_VALUE_MAX_LENGTH 8 //Максимальная длина параметра команды, передаваемого в API-фрейме, в байтах.

/*********************************************************************
 * TYPEDEFS
 */
typedef int8_t rssi_t;

typedef struct
{
  uint8_t id[NODE_ID_SIZE];
}nodeID_t;

typedef struct
{
  uint8_t b[2];
}uint16BigEndian_t;

typedef struct
{
  uint8_t b[4];
}uint32BigEndian_t;

/*********************************************************************
 ФОРМАТЫ, СВЯЗАННЫЕ С ОРГАНИЗАЦИЕЙ ВВОДА/ВЫВОДА
*********************************************************************/
//Тип, определяющий режим работы линии ввода/вывода. ВНИМАНИЕ!! При модификации необходимо изменить константу PIN_MODES_MAX_NUMBER.
typedef enum
{
  IO_DISABLED = 0, //Линия настраивается на ввод и подключается pullup.
  IO_NOT_AVAILABLE = 1,
  IO_ADC = 2,
  IO_DIGITAL_INPUT = 3,
  IO_DIGITAL_OUTPUT_LO = 4,
  IO_DIGITAL_OUTPUT_HI = 5,
  IO_UART_TX = 6, //Выход UART TX. Может быть присвоен только одной линии в каждый момент времени.
  IO_UART_RX = 7, //Вход UART RX. Может быть присвоен только одной линии в каждый момент времени.  
  IO_UART_CTS = 8, //Вход UART CTS. Может быть присвоен только одной линии в каждый момент времени.  
  IO_UART_RTS = 9, //Выход UART CTS. Может быть присвоен только одной линии в каждый момент времени.
  IO_SYSTEM_LED = 10, //Выход. Может быть присвоен только одной линии в каждый момент времени.
  IO_SLEEP_REQUEST = 11, //Вход. Может быть присвоен только одной линии в каждый момент времени.
  IO_SLEEP_STATUS = 12, //Выход. Может быть присвоен только одной линии в каждый момент времени.
  IO_COUNTER_INPUT1 = 13, //Может быть присвоен только одной линии в каждый момент времени.
  IO_COUNTER_INPUT2 = 14, //Может быть присвоен только одной линии в каждый момент времени.
  IO_WAKEUP_INPUT = 15,
  IO_PWM1 = 16,
  IO_PWM2 = 17,
  IO_PWM3 = 18,
  IO_PWM4 = 19
}pinMode_t;

//Тип для управления рабочим циклом PWM.
typedef uint16BigEndian_t pwmDutyCycle_t;

/*********************************************************************
 ФОРМАТЫ ЭФИРНЫХ ПАКЕТОВ
*********************************************************************/
//Маршрутный заголовок
typedef struct
{
  uint8_t packetType;
  uint8_t transactionID;
  nodeID_t nextHopID;
  nodeID_t previousHopID;
  nodeID_t destinationID; 
  nodeID_t sourceID;
}route_t;

//Тип эфирного пакета для Transmit Request API-фрейма без опций передачи.
typedef struct
{
  uint8_t rfData[RF_PAYLOAD_MAX_LENGTH];
}txRequestNoOptionsRfPayload_t;

//Тип заголовка для эфирного пакета для Transmit Request API-фрейма с опциями передачи.
typedef struct
{
  uint8_t transmitOptions;
}txRequestRfPayloadHeader_t;

//Тип эфирного пакета для Transmit Request API-фрейма с опциями передачи.
typedef struct
{
  txRequestRfPayloadHeader_t header;
  uint8_t rfData[TX_REQUEST_PAYLOAD_MAX_LENGTH];
}txRequestRfPayload_t;

//Тип заголовка для эфирного пакета для Remote AT command request API-фрейма.
typedef struct
{
  uint8_t remoteCommandOptions;
  uint8_t commandCode;
}remoteAtCommandRequestRfPayloadHeader_t;

//Тип эфирного пакета для Remote AT command request API-фрейма.
typedef struct
{
  remoteAtCommandRequestRfPayloadHeader_t header;
  uint8_t commandParameter[API_FRAME_PARAMETER_VALUE_MAX_LENGTH];
}remoteAtCommandRequestRfPayload_t;

//Тип эфирного пакета для IO data sample API-фрейма.
typedef struct
{
  uint8_t b[RF_PAYLOAD_MAX_LENGTH];
}rawDateRfPayload_t;

//Базовый тип эфирного пакета
typedef struct
{
  route_t route;
  union
  {
    txRequestNoOptionsRfPayload_t txRequestNoOptions;
    txRequestRfPayload_t txRequest;
    remoteAtCommandRequestRfPayload_t remoteAtCommand;
    rawDateRfPayload_t rawDate;
  }payload;
}rfFrame_t;

/*********************************************************************
 ФОРМАТЫ и константы API-фреймов
*********************************************************************/
//Идентификаторы API-фрейма, совместимые с XBee.
typedef enum
{
  MODEM_STATUS_API_FRAME = 0x8A,
  AT_COMMAND_IMMEDIATE_APPLY_API_FRAME = 0x07, //Идентификатор введен SysMC. AT-команда, содержащаяся во фрейме, применяется непосредственно после получения без сохранения измененого параметра во флеше.
  AT_COMMAND_API_FRAME = 0x08,
  AT_COMMAND_QUEUE_PARAMETER_VALUE_API_FRAME = 0x09,
  AT_COMMAND_RESPONSE_API_FRAME = 0x88,
  REMOTE_AT_COMMAND_REQUEST_API_FRAME = 0x17, 
  REMOTE_AT_COMMAND_RESPONSE_API_FRAME = 0x97,
  TRANSMIT_REQUEST_NO_OPTIONS_API_FRAME = 0x0F, //Идентификатор, введенный SysMC, предназначенный для передачи данных без байта опций. Нужен для увеличения полезной нагрузки.
  TRANSMIT_REQUEST_API_FRAME = 0x10, //Идентификатор, используемый на большинстве модулей DIGI.
  TRANSMIT_REQUEST_PRO_API_FRAME = 0x01, //Идентификатор, используемый а модулях XBee PRO.
  RECEIVE_PACKET_API_FRAME = 0x90,
  RECEIVE_PACKET_PRO_API_FRAME = 0x81, //Идентификатор, используемый а модулях XBee PRO.
  IO_DATA_SAMPLE_API_FRAME = 0x92
}apiFrameType_t;

typedef enum
{
  HARDWARE_RESET_MODEM_STATUS = 0x00,
  RESERVE_MODEM_STATUS = 0x01
}modemStatus_t;

typedef enum
{
  
  OK_COMMAND_STATUS = 0x00,
  ERROR_COMMAND_STATUS = 0x01,
  INVALID_CODE_COMMAND_STATUS = 0x02,
  INVALID_PARAMETER_COMMAND_STATUS = 0x03
}commandStatus_t;

//Формат API-фрейма с ответом на API-фрейм с AT-командой.
typedef struct
{
  //apiFrameType_t frameType; //См. примечание в начале файла!
  uint8_t frameType;
  uint8_t frameID;
  char code[2];
  //commandStatus_t commandStatus;
  uint8_t commandStatus;
  uint8_t parameterValue[API_FRAME_PARAMETER_VALUE_MAX_LENGTH];
}atCommandResponseApiFrameData_t;

//Формат заголовка поля data API-фрейма, предназначенного для передачи в UART пакета, принятого по эфиру
typedef struct
{
  uint8_t frameType;
  nodeID_t sourceID;
  rssi_t rssi;
  uint8_t options;
}rxApiFrameDataHeader_t;

//Формат поля data API-фрейма, предназначенного для передачи в UART пакета, принятого по эфиру
typedef struct
{
  rxApiFrameDataHeader_t header;
  uint8_t payload[TX_REQUEST_PAYLOAD_MAX_LENGTH]; 
}rxApiFrameData_t;

//Хост->модем
//Формат API-фрейма с AT-командой.
typedef struct
{
  uint8_t frameType;
  uint8_t frameID;
  unsigned char code[2];
  uint8_t parameterValue[API_FRAME_PARAMETER_VALUE_MAX_LENGTH];
}atCommandApiFrameData_t;

//Формат заголовка поля data API-фрейма без опций передачи, предназначенного для передачи по эфиру.
typedef struct
{
  uint8_t frameType;
  uint8_t frameID;
  nodeID_t destinationID;
}txRequestNoOptionsApiFrameDataHeader_t;

//Формат Transmit Request API-фрейма без опций передачи.
typedef struct
{
  txRequestNoOptionsApiFrameDataHeader_t header;
  uint8_t rfData[TX_REQUEST_PAYLOAD_MAX_LENGTH]; //Название поля выбрано с целью соответствия терминологии DIGI
}txRequestNoOptionsApiFrameData_t;

//Формат заголовка поля data API-фрейма с опциями передачи, предназначенного для передачи по эфиру.
typedef struct
{
  uint8_t frameType;
  uint8_t frameID;
  nodeID_t destinationID;
  uint8_t transmitOptions;
}txRequestApiFrameDataHeader_t;

//Формат Transmit Request API-фрейма с опциями передачи.
typedef struct
{
  txRequestApiFrameDataHeader_t header;
  uint8_t rfData[TX_REQUEST_PAYLOAD_MAX_LENGTH]; //Название поля выбрано с целью соответствия терминологии DIGI
}txRequestApiFrameData_t;

//Формат заголовка API-фрейма, предназначенного для передачи по эфиру AT-команды удаленному узлу.
typedef struct
{
  uint8_t frameType;
  uint8_t frameID;
  nodeID_t destinationID;
  uint8_t remoteCommandOptions;
  unsigned char code[2];
}remoteAtCommandRequestApiFrameDataHeader_t;

//Формат API-фрейма, предназначенного для передачи по эфиру AT-команды удаленному узлу.
typedef struct
{
  remoteAtCommandRequestApiFrameDataHeader_t header;
  uint8_t commandParameter[API_FRAME_PARAMETER_VALUE_MAX_LENGTH]; 
}remoteAtCommandRequestApiFrameData_t;

//Базовый тип для API-фрейма
typedef struct
{
  uint8_t sof = 0x7E;
  uint8_t lengthMsb = 0x00;
  uint8_t lengthLsb;
  union
  {
    //modemStatus_t modemStatus; //См. примечание в начале файла!
    uint8_t modemStatus;
    txRequestNoOptionsApiFrameData_t txRequestNoOptions;
    txRequestApiFrameData_t txRequest;
    rxApiFrameData_t rx;
    atCommandApiFrameData_t atCommand;
    atCommandResponseApiFrameData_t atResponse;
    remoteAtCommandRequestApiFrameData_t remoteAtCommand;
  }frameData;
  uint8_t checkSum;
}apiFrame_t;

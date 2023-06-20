#ifndef RCU_DIAGNOSTIC_CONFIG_H
#define RCU_DIAGNOSTIC_CONFIG_H

#include "../RCUDataTypes.h"


#define ARCON_CANOPEN_NODE_ID 0x65
#define ARCON_SDO_RESPONSE_ID 0x5E5

#define AUTEC_CANOPEN_NODE_ID 0xA
#define AUTEC_SDO_RESPONSE_ID 0x58A


typedef struct
{
    RCUBrands rcu;
    uint8_t CAN_OPEN_NODE_ID = AUTEC_CANOPEN_NODE_ID; // for arcon: 0x65 (102), for autec 0xA (10)
    uint16_t CAN_OPEN_RESPONSE_ID = AUTEC_SDO_RESPONSE_ID; //0x5E5 for arcon, 0x58A for autec
    uint16_t HEARTBEAT_TIME = 500;                   // [ms]
    uint16_t HEARTBEAT_TIMEOUT = HEARTBEAT_TIME * 3; // [ms]
} tsRCUConstants;

typedef enum
{
    eResponseState_SendRequest = 0,
    eResponseState_Waiting,
    eResponseState_Arrived,
    eResponseState_Timeout,
    eResponseState_Finished,
    eResponseState_Error
} teResponseState;

typedef enum
{
    eSdoResponse_Write = 0,
    eSdoResponse_Read,
    eSdoResponse_SendReadStr,
    eSdoResponse_ReadStr,
    eSdoResponse_Error,
    eSdoResponse_Invalid
} teSdoResponse;

typedef struct
{
    teSdoResponse response;
    uint16_t index;
    uint8_t subIndex;
    uint64_t data;
} tsSdoObject;

typedef struct
{
    uint16_t index;
    uint8_t subindex;
    QString label;
    QString description;
    QString data;
}tsRCUSDO;



//ARCON
#define  ARCON_SDO_CONFIG \
/* index          subindex        label                              description                     is2read    */ \
X(0x1000,            0x00,       "Device type"                      ,     "DEVICE_TYPE            " ,   true  )   \
X(0x1001,            0x00,       "Error register"                   ,     "ERROR_REGISTER         " ,   true  )   \
X(0x1008,            0x00,       "Manufacturer device name"         ,     "MANUFACTURER_DEV_NAME  " ,   true  )   \
X(0x1009,            0x00,       "Manufacturer hardware version"    ,     "MANUFACTURER_HW_VERSION" ,   true  )   \
X(0x100A,            0x00,       "Manufacturer software version"    ,     "MANUFACTURER_SW_VERSION" ,   true  )   \
X(0x1018,            0x01,       "Vendor id"                        ,     "VENDOR_ID              " ,   true  )   \
X(0x1018,            0x02,       "Product code"                     ,     "PRODUCT_CODE           " ,   true  )   \
X(0x1018,            0x03,       "Revision number"                  ,     "REVISION_NUMBER        " ,   true  )   \
X(0x1018,            0x04,       "Serial number"                    ,     "SERIAL_NUMBER          " ,   true  )   \
X(0x1014,            0x00,       "COB-ID EMCY message"              ,     "COB-ID_EMCY          "   ,   true  )   \
X(0x2000,            0x00,       "CANopen Baudrate"                 ,     "Baudrate          "      ,   true  )   \
X(0x2001,            0x00,       "CANopen node id"                  ,     "node id          "       ,   true  )   \
X(0x2005,            0x00,       "CANopen PDO Transmit Cycle time"  ,     "PDO transmit cycle time" ,   true  )   \
X(0x0000,            0x00,       "Arcon Remote Controller"          ,     "RCU Brand"               ,   false  )   \

//HETRONIC

//it won't be read from SDO object, this is just for the structure
#define   HETRONIC_SDO_CONFIG \
/* index          subindex        label                              name                        is2read     */ \
X(0x0000,            0x00,       "Hetronic Remote Controller"  ,     "RCU Brand"              ,   false    )    \
X(0x0000,            0x00,       "Hetronic Remote Controller"  ,     "RCU Brand"              ,   false    )    \
X(0x0000,            0x00,       "Hetronic Remote Controller"  ,     "RCU Brand"              ,   false    )    \
X(0x0000,            0x00,       "Hetronic Remote Controller"  ,     "RCU Brand"              ,   false    )    \
X(0x0000,            0x00,       "Hetronic Remote Controller"  ,     "RCU Brand"              ,   false    )    \



//AUTEC

#define  AUTEC_SDO_CONFIG \
/* index          subindex        label                              description                 is2read     */ \
X(0x1000,            0x00,       "Device type"                 ,     "DEVICE_TYPE            ",    true    )   \
X(0x1001,            0x00,       "Error register"              ,     "ERROR_REGISTER         ",    true    )   \
X(0x1002,            0x00,       "Manufacturer status register",     "MANUFACTURER_STATUS_REG",    true    )   \
X(0x1003,            0x00,       "Number of errors"            ,     "NUMBER_OF_ERRORS       ",    true    )   \
X(0x1003,            0x01,       "Standard error field 1"      ,     "STANDARD_ERR_FIELD1    ",    true    )   \
X(0x1003,            0x02,       "Standard error field 2"      ,     "STANDARD_ERR_FIELD2    ",    true    )   \
X(0x1003,            0x03,       "Standard error field 3"      ,     "STANDARD_ERR_FIELD3    ",    true    )   \
X(0x1003,            0x04,       "Standard error field 4"      ,     "STANDARD_ERR_FIELD4    ",    true    )   \
X(0x1003,            0x05,       "Standard error field 5"      ,     "STANDARD_ERR_FIELD5    ",    true    )   \
X(0x1003,            0x06,       "Standard error field 6"      ,     "STANDARD_ERR_FIELD6    ",    true    )   \
X(0x1003,            0x07,       "Standard error field 7"      ,     "STANDARD_ERR_FIELD7    ",    true    )   \
X(0x1003,            0x08,       "Standard error field 8"      ,     "STANDARD_ERR_FIELD8    ",    true    )   \
/*X(0x1008,            0x00,       "Manufacturer device name"    ,     "MANUFACTURER_DEV_NAME  ",  , true    ) */  \
X(0x1009,            0x00,       "Manufacturer hardware version",    "MANUFACTURER_HW_VERSION",    true    )   \
X(0x100A,            0x00,       "Manufacturer software version",    "MANUFACTURER_SW_VERSION",    true    )   \
X(0x1018,            0x01,       "Vendor id"                   ,     "VENDOR_ID              ",    true    )   \
X(0x1018,            0x02,       "Product code"                ,     "PRODUCT_CODE           ",    true    )   \
X(0x1018,            0x03,       "Revision number"             ,     "REVISION_NUMBER        ",    true    )   \
X(0x1018,            0x04,       "Serial number"               ,     "SERIAL_NUMBER          ",    true    )   \
X(0x1029,            0x01,       "Communication error"         ,     "COMMUNICATION_ERROR    ",    true    )   \
X(0x2000,            0x00,       "Bit rate"                    ,     "BIT_RATE               ",    true    )   \
X(0x2001,            0x00,       "node id"                     ,     "NODE_ID                ",    true    )   \
X(0x0000,            0x00,       "Autec Remote Controller"     ,     "RCU Brand"              ,   false    )   \



#endif	// END: RCU_DIAGNOSTIC_CONFIG_H

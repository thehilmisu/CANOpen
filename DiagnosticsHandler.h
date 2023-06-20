#ifndef RCU_DIAGNOSTICS_HANDLER_H
#define RCU_DIAGNOSTICS_HANDLER_H


//
//-------------------------------------------------------------------------------------------------
//
// Include directives
//
#include <QObject>
#include <QTimer>
#include <QList>
#include <sstream>
#include <QFile>
#include <QTextStream>

#include "canOpenDefs.h"
#include "CanDeviceConfig.h"
#include "CanDriver/CanTypes.h"
#include "RCUDiagnosticConfig.h"


#include "singleton.h"

#include "stdio.h"
#include "stdint.h"

#include <QDebug>


//
//-------------------------------------------------------------------------------------------------
//
// Public definitions
//
#define         TIMER_TIMEOUT_INTERVAL      1000
#define         MINIMUM_DIGIT_COUNT         2
#define         MAX_RETRY_COUNT             4

#define         PARAMETER_FILE_PATH         "/home/mototok/res/db/parameters.txt"
//
//-------------------------------------------------------------------------------------------------
//
// Class declaration
//
class RCUDiagnosticsHandler : public Singleton<RCUDiagnosticsHandler>
{
    Q_OBJECT

public:
    explicit RCUDiagnosticsHandler();
    ~RCUDiagnosticsHandler();

    // These function should be used when added to the CAN Gateway configuration list
    static void TxSdoReceived(canmsg_t msg);

    void SetRemoteController(RCUBrands rcu);
    void onDiagnosticsUpdateRequest();
    
public Q_SLOTS:
    void onTimeout();
    void onRequestTimerTimeout();

private:
    void constructSDOConfig();
    void sendReadRequest(uint16_t idx, uint8_t subIdx, teCanOpenSdoCommands cmd);
    void saveAllParameters();
    QList<tsRCUSDO*> parameters2Read;
    QList<tsRCUSDO*> parametersConstant;
    
    teResponseState mResponseState;
    QTimer mTimeoutTimer;
    QTimer *mRequestTimer;
    tsSdoObject mSdoObjectRxd;
    tsRCUConstants mRCUConstants;

    //some usefull utility functions
    //TODO: convert std::string operations to QString ones
    std::string hexToASCII(uint64_t data);
    uint64_t canDataTo64BitsValue(unsigned char* dp, quint8 const idx);
    uint32_t canDataTo32BitsValue(unsigned char* dp, quint8 const idx);
    uint32_t canDataTo24BitsValue(unsigned char* dp, quint8 const idx);
    uint16_t canDataTo16BitsValue(unsigned char* dp, quint8 const idx);
    bool bit16ValueToCanData(quint16 const val, unsigned char* dp, quint8 const idx);

};

#endif // RCU_DIAGNOSTICS_HANDLER_H

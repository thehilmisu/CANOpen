
//
//-------------------------------------------------------------------------------------------------
//
// Include directives
//
#include "RCUDiagnosticsHandler.h"

#include <QMetaObject>
#include <QtCore>
#include <QtGlobal>

#include "CanGateway.h"
#include "CanMessageHandling/CanMsgTypes.h"
#include "alleviations.h"
#include "Tracer.h"
#include "DiagClient.h"


//
//-------------------------------------------------------------------------------------------------
//
// Class constructor and destructor
//
RCUDiagnosticsHandler::RCUDiagnosticsHandler():
    Singleton<RCUDiagnosticsHandler>(*this),
    mResponseState(eResponseState_SendRequest),
    mTimeoutTimer(0)
{
    TRACE_INFO("%s()", __FUNCTION__);

    mRequestTimer = new QTimer();
    
}

RCUDiagnosticsHandler::~RCUDiagnosticsHandler()
{
    mTimeoutTimer.stop();
    mRequestTimer->stop();
    disconnect(&mTimeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    disconnect(mRequestTimer, SIGNAL(timeout()), this, SLOT(onRequestTimerTimeout()));

    delete mRequestTimer;
}


//
//-------------------------------------------------------------------------------------------------
//
// Public Q_SLOT class function definitions
//
void RCUDiagnosticsHandler::SetRemoteController(RCUBrands rcu)
{
    mRCUConstants.rcu = rcu;
    switch (rcu)
    {
    case ARCON:
        mRCUConstants.CAN_OPEN_NODE_ID = ARCON_CANOPEN_NODE_ID;
        break;
    case AUTEC:
        mRCUConstants.CAN_OPEN_NODE_ID = AUTEC_CANOPEN_NODE_ID;
        break;
    
    default:
        mRCUConstants.CAN_OPEN_NODE_ID = ARCON_CANOPEN_NODE_ID;
        break;
    }

    
    connect(&mTimeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    mTimeoutTimer.setSingleShot(true);
    mTimeoutTimer.setInterval(TIMER_TIMEOUT_INTERVAL);
    mTimeoutTimer.stop();

    connect(mRequestTimer, SIGNAL(timeout()), this, SLOT(onRequestTimerTimeout()));
    mRequestTimer->setInterval(TIMER_TIMEOUT_INTERVAL);
   

    memset(&mSdoObjectRxd, 0, sizeof(mSdoObjectRxd));
    constructSDOConfig();
}

void RCUDiagnosticsHandler::onTimeout()
{
//    TRACE_INFO("%s()", __FUNCTION__);

    TRACE_WARN("Timeout on response from device");

    mTimeoutTimer.stop();
    mResponseState = eResponseState_Timeout;
}


//
//-------------------------------------------------------------------------------------------------
//
// Public class function definitions
//

void RCUDiagnosticsHandler::TxSdoReceived(canmsg_t msg)
{
    RCUDiagnosticsHandler* inst = RCUDiagnosticsHandler::getInstance();
    teCanOpenSdoCommands cmd = (teCanOpenSdoCommands)msg.data[0];
    tsSdoObject obj;

    TRACE_INFO("%s() cmd : %s", __FUNCTION__, TRACE_TO_STRING(TO_QHEXSTRING(msg.data[0], 2)));

    // First of all, stop the timeout timer as we have now some response
    inst->mTimeoutTimer.stop();

    obj.index = inst->canDataTo16BitsValue(msg.data, 1);
    obj.subIndex = msg.data[2];

    switch (cmd)
    {
        case eCanOpenSdoCommand_ReadResponse_1Byte:
            obj.response = eSdoResponse_Read;
            obj.data = msg.data[4];
        break;
        case eCanOpenSdoCommand_ReadResponse_2Byte:
            obj.response = eSdoResponse_Read;
            obj.data = inst->canDataTo16BitsValue(msg.data, 4);
        break;
        case eCanOpenSdoCommand_ReadResponse_3Byte:
            obj.response = eSdoResponse_Read;
            obj.data = inst->canDataTo24BitsValue(msg.data, 4);
        break;
        case eCanOpenSdoCommand_ReadResponse_String:
            obj.response = eSdoResponse_SendReadStr;
            obj.data = inst->canDataTo32BitsValue(msg.data, 4);
        break;
        case eCanOpenSdoCommand_Read_String_Autec:
            obj.response = eSdoResponse_ReadStr;
            obj.data = inst->canDataTo64BitsValue(msg.data, 1);
        break;
        case eCanOpenSdoCommand_Read_String_Arcon:
            obj.response = eSdoResponse_ReadStr;
            obj.data = inst->canDataTo64BitsValue(msg.data, 1);
        break;
        case eCanOpenSdoCommand_ReadResponse_4Byte:
            obj.response = eSdoResponse_Read;
            obj.data = inst->canDataTo32BitsValue(msg.data, 4);
        break;

        case eCanOpenSdoCommand_WriteResponse:
            obj.response = eSdoResponse_Write;
        break;

        case eCanOpenSdoCommand_ErrorResponse:
            obj.response = eSdoResponse_Error;
        break;

        case eCanOpenSdoCommand_WriteReq_1Byte:         // Fall-through
        case eCanOpenSdoCommand_WriteReq_2Byte:         // Fall-through
        case eCanOpenSdoCommand_WriteReq_3Byte:         // Fall-through
        case eCanOpenSdoCommand_WriteReq_4Byte:         // Fall-through
        case eCanOpenSdoCommand_ReadRequest:            // Fall-through
        default:
            TRACE_ERR("SDO requests are not supported here");
            obj.response = eSdoResponse_Invalid;
        break;
    }

//    TRACE_INFO("SDO cmd %s, response %d, idx %s:%s, data %s",
//            TRACE_TO_STRING(TO_QHEXSTRING(cmd, 2)),
//            obj.response,
//            TRACE_TO_STRING(TO_QHEXSTRING(obj.index, 4)),
//            TRACE_TO_STRING(TO_QHEXSTRING(obj.subIndex, 2)),
//            TRACE_TO_STRING(TO_QHEXSTRING(obj.data, 8)));

    inst->mSdoObjectRxd = obj;
    inst->mResponseState = eResponseState_Arrived;
}


//
//-------------------------------------------------------------------------------------------------
//
// Private class function definitions
//

void RCUDiagnosticsHandler::constructSDOConfig()
{
    tsRCUSDO *temp = nullptr;

    parameters2Read.clear();
    parametersConstant.clear();

    switch (mRCUConstants.rcu)
    {
        case AUTEC:
        
           #define X(__index, __subindex, __label, __desc, __is_to_read) \
                temp = new tsRCUSDO();\
                temp->index = __index;\
                temp->subindex = __subindex;\
                temp->label = QString(__label);\
                temp->description = __desc;\
                if(__is_to_read) \
                    parameters2Read.append(temp); \
                else { \
                    temp->data = temp->label; \
                    parametersConstant.append(temp);} \

                AUTEC_SDO_CONFIG
            break;
        case ARCON:
            #define X(__index, __subindex, __label, __desc, __is_to_read) \
                temp = new tsRCUSDO();\
                temp->index = __index;\
                temp->subindex = __subindex;\
                temp->label = QString(__label);\
                temp->description = __desc;\
                if(__is_to_read) \
                    parameters2Read.append(temp); \
                else { \
                    temp->data = temp->label; \
                    parametersConstant.append(temp);} \


                ARCON_SDO_CONFIG
            break;
        case HETRONIC:
            #define X(__index, __subindex, __label, __desc, __is_to_read) \
                temp = new tsRCUSDO();\
                temp->index = __index;\
                temp->subindex = __subindex;\
                temp->label = QString(__label);\
                temp->description = __desc;\
                if(__is_to_read) \
                    parameters2Read.append(temp); \
                else { \
                    temp->data = temp->label; \
                    parametersConstant.append(temp);} \
                    

                HETRONIC_SDO_CONFIG
            break;
        default:
            break;
    }

    mRequestTimer->start();
    
}

void RCUDiagnosticsHandler::onDiagnosticsUpdateRequest()
{
    TRACE_INFO("RCU diagnostics update requested ....");
    mResponseState = eResponseState_SendRequest;
    constructSDOConfig();
    mTimeoutTimer.stop();
}

void RCUDiagnosticsHandler::onRequestTimerTimeout()
{
    static int pIndex = 0;
    static int retryCount = 0;
    
    switch (mResponseState)
    {
        case eResponseState_SendRequest:
        {
            uint16_t index = parameters2Read.at(pIndex)->index;
            uint8_t subindex = parameters2Read.at(pIndex)->subindex;

            TRACE_INFO("idx = 0x%04X, subIdx = %d", index, subindex);

            sendReadRequest(index, subindex, eCanOpenSdoCommand_ReadRequest);

            mTimeoutTimer.start(1000);
            mResponseState = eResponseState_Waiting;
        }
        break;
        case eResponseState_Waiting:
        {
            //do nothing
            //TODO: decide whether to do something here
        }
        break;
        case eResponseState_Arrived:
        {
            if (eSdoResponse_Read == mSdoObjectRxd.response)
            {
                QString msg = "Received response data: ";
                msg += QString("%1").arg(TO_QHEXSTRING(mSdoObjectRxd.data, 8));
                msg += " integer parameter has arrived";

                parameters2Read.at(pIndex)->data = "0x"+QString::number(mSdoObjectRxd.data,16);
                
                TRACE_INFO("%s", TRACE_TO_STRING(msg));
                pIndex++;
                if(pIndex < parameters2Read.count())
                {
                    mResponseState = eResponseState_SendRequest;
                }
                else
                {
                    mResponseState = eResponseState_Finished;
                    pIndex = 0;
                }
                retryCount = 0;

            }
            else if(eSdoResponse_SendReadStr == mSdoObjectRxd.response)
            {
                sendReadRequest(0, 0, eCanOpenSdoCommand_ReadStringRequest);

                mTimeoutTimer.start(1000);
                mResponseState = eResponseState_Waiting;
            }
            else if(eSdoResponse_ReadStr == mSdoObjectRxd.response)
            {
                QString msg = "Received response data: ";
                msg += QString("%1").arg(TO_QHEXSTRING(mSdoObjectRxd.data, 8));

                msg += " string parameter has arrived";

                parameters2Read.at(pIndex)->data = QString::fromStdString(hexToASCII(mSdoObjectRxd.data));
                
                TRACE_INFO("%s", TRACE_TO_STRING(msg));
                TRACE_INFO("visible string data is : %s", TRACE_TO_STRING(QString::fromStdString(hexToASCII(mSdoObjectRxd.data))));
                pIndex++;
                if(pIndex < parameters2Read.count())
                {
                    mResponseState = eResponseState_SendRequest;
                }
                else
                {
                    mResponseState = eResponseState_Finished;
                    pIndex = 0;
                }
                retryCount = 0;
            }
            else if(eSdoResponse_Error == mSdoObjectRxd.response)
            {
                
                //in case of an error ask the parameter again, retry that MAX_RETRY_COUNT times
                //error response is: 0x80
                retryCount++;
                if(retryCount >= MAX_RETRY_COUNT)
                {
                    retryCount = 0;
                    //if we have reached the maximum retry count, skip the parameter and ask for the next
                    pIndex++;
                    TRACE_WARN("Maximum retry has been reached. Skipping the parameter");
                }
                
                mResponseState = eResponseState_SendRequest;
                
            }
        }
        break;

        case eResponseState_Finished:
        {   
            TRACE_INFO("Finished reading parameters");
            pIndex = 0;
            retryCount = 0;
            mRequestTimer->stop();
            saveAllParameters();
        }
        break;

        case eResponseState_Timeout:
        {   
            //mResponseState = eResponseState_Start;
            TRACE_ERR("Timeout occured ");

            retryCount++;
            if(retryCount >= MAX_RETRY_COUNT)
            {
                //maximum retry count has been reached.
                //increment parameter index to send next parameter
                pIndex++;
                retryCount = 0;
                TRACE_WARN("Maximum retry has been reached on timeout. Skipping the parameter");
            }
            mResponseState = eResponseState_SendRequest;

        }
        break;
        case eSdoResponse_Invalid:
        {   
            TRACE_ERR("Invalid response");
            mResponseState = eResponseState_SendRequest;
        
        }
        break;

        default:
        {
            TRACE_ERR("Unexpected response state (%d)", mResponseState);
            mResponseState = eResponseState_SendRequest;
        }
        break;
    }
}

void RCUDiagnosticsHandler::sendReadRequest(uint16_t idx, uint8_t subIdx, teCanOpenSdoCommands cmd)
{
    TRACE_INFO("%s(), idx = 0x%04X, subIdx = %d", __FUNCTION__,idx, subIdx);

    canmsg_t msg;

    msg.cob = 0;
    msg.id = CAN_OPEN_SDO_RECEIVE_SID | mRCUConstants.CAN_OPEN_NODE_ID;
    msg.length = 8;
    msg.flags = CAN_FRAME_NO_FORMAT;
    msg.data[0] = cmd;
    
    msg.data[1] = (idx & 0xFF);
    msg.data[2] = ((idx >> 8) & 0xFF);

    msg.data[3] = subIdx;

    msg.data[4] = 0;
    msg.data[5] = 0;
    msg.data[6] = 0;
    msg.data[7] = 0;

    CanGateway::GetInstance()->transmitMsg(eCanDevice_CAN_A, msg);
    CanGateway::PrintCanMsg(msg);

}

void RCUDiagnosticsHandler::saveAllParameters()
{
    QFile file(PARAMETER_FILE_PATH);
    
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QFile::Truncate))
        TRACE_ERR("File could not be opened...");

    QTextStream out(&file);

    for(auto item : parametersConstant)
        out << item->index << "," << item->label << "," << item->description << "," << item->data << "," << "\n";

    for(auto item : parameters2Read)
        out << item->index << "," << item->label << "," << item->description << "," << item->data << "," << "\n";

    out.flush();
    file.close();

    TRACE_INFO("parameters saved to a file");
    parametersConstant.clear();
    parameters2Read.clear();
}

std::string RCUDiagnosticsHandler::hexToASCII(uint64_t data)
{
    std::ostringstream ss;
    ss << std::hex << data;

    //raw hex string is a reversed form of the string
    std::string raw = ss.str();
    
    std::string hex = "";

    //check if you have at least one (2-hex digits) character in hex string
    if(raw.length() >= MINIMUM_DIGIT_COUNT)
    {
        //reverse the hex string 
        for(int i=raw.length()-MINIMUM_DIGIT_COUNT;i>=0;i=i-MINIMUM_DIGIT_COUNT)
            hex.append(raw.substr(i,MINIMUM_DIGIT_COUNT));
    }
    else
    {
        TRACE_ERR("String length is smaller than expected");
        return "";
    }
    
    // initialize the ASCII code string as empty.
    std::string ascii = "";
    for (uint8_t i = 0; i < hex.length(); i += 2)
    {
        // extract two characters from hex string
        std::string part = hex.substr(i, 2);
 
        // change it into base 16 and
        // typecast as the character
        char ch = stoul(part, nullptr, 16);
 
        // add this char to final ASCII string
        ascii += ch;
    }

    return ascii;
}

bool RCUDiagnosticsHandler::bit16ValueToCanData(quint16 const val, unsigned char* dp, quint8 const idx)
{
//    TRACE_INFO("%s(), val = 0x%04X, idx = %d", __FUNCTION__, val, idx);

    bool success = true;

    if (NULL != dp)
    {
        if (6 >= idx)
        {
//            TRACE_INFO("val = %d (0x%04X)", val, val);
            dp[idx] = (quint16)val & 0xFF;
            dp[idx + 1] = (quint16)val >> 8;
//            TRACE_INFO("dp[idx] = %d (0x%02X), dp[idx+1] = %d (0x%02X)",
//                    dp[idx], dp[idx], dp[idx + 1], dp[idx + 1]);
        }
        else
        {
            TRACE_ERR("Index %d too great", idx);
            success = false;
        }
    }
    else
    {
        TRACE_ERR("Null pointer");
        success = false;
    }

    return success;
}

uint16_t RCUDiagnosticsHandler::canDataTo16BitsValue(unsigned char* dp, quint8 const idx)
{
//    TRACE_INFO("%s(), idx = %d", __FUNCTION__, idx);
    uint16_t val = 0;

    if (NULL != dp)
    {
        if (6 >= idx)
        {
//            TRACE_INFO("dp[idx] = %d (0x%02X), dp[idx+1] = %d (0x%02X)",
//                    dp[idx], dp[idx], dp[idx + 1], dp[idx + 1]);
            val = ((dp[idx + 1]) << 8) | (dp[idx]);
//            TRACE_INFO("val = %d (0x%04X)", val, val);
        }
        else
        {
            TRACE_ERR("Index %d too great", idx);
        }
    }
    else
    {
        TRACE_ERR("Null pointer");
    }

    return val;
}

uint32_t RCUDiagnosticsHandler::canDataTo24BitsValue(unsigned char* dp, quint8 const idx)
{
//    TRACE_INFO("%s(), idx = %d", __FUNCTION__, idx);
    uint32_t val = 0;

    if (NULL != dp)
    {
        if (5 >= idx)
        {
//            TRACE_INFO("dp[idx] = %d (0x%02X), dp[idx+1] = %d (0x%02X),  dp[idx+2] = %d (0x%02X)",
//                        dp[idx], dp[idx], dp[idx + 1], dp[idx + 1], dp[idx + 2], dp[idx + 2]);
            val = ((dp[idx + 2]) << 16) | ((dp[idx + 1]) << 8) | (dp[idx]);
//            TRACE_INFO("val = %ld (0x%08X)", val, val);
        }
        else
        {
            TRACE_ERR("Index %d too great", idx);
        }
    }
    else
    {
        TRACE_ERR("Null pointer");
    }

    return val;
}

uint32_t RCUDiagnosticsHandler::canDataTo32BitsValue(unsigned char* dp, quint8 const idx)
{
//    TRACE_INFO("%s(), idx = %d", __FUNCTION__, idx);
    uint32_t val = 0;

    if (NULL != dp)
    {
        if (4 >= idx)
        {
//            TRACE_INFO("dp[idx] = %d (0x%02X), dp[idx+1] = %d (0x%02X),  dp[idx+2] = %d (0x%02X),  dp[idx+3] = %d (0x%02X)",
//                        dp[idx], dp[idx], dp[idx + 1], dp[idx + 1],
//                        dp[idx + 2], dp[idx + 2], dp[idx + 3], dp[idx + 3]);
            val = ((dp[idx + 3]) << 24) | ((dp[idx + 2]) << 16) | ((dp[idx + 1]) << 8) | (dp[idx]);
//            TRACE_INFO("val = %ld (0x%08X)", val, val);
        }
        else
        {
            TRACE_ERR("Index %d too great", idx);
        }
    }
    else
    {
        TRACE_ERR("Null pointer");
    }

    return val;
}

uint64_t RCUDiagnosticsHandler::canDataTo64BitsValue(unsigned char* dp, quint8 const idx)
{
//    TRACE_INFO("%s(), idx = %d", __FUNCTION__, idx);
    uint64_t val = 0;

    if (NULL != dp)
    {
        if (8 >= idx)
        {
//            TRACE_INFO("dp[idx] = %d (0x%02X), dp[idx+1] = %d (0x%02X),  dp[idx+2] = %d (0x%02X),  dp[idx+3] = %d (0x%02X)",
//                        dp[idx], dp[idx], dp[idx + 1], dp[idx + 1],
//                        dp[idx + 2], dp[idx + 2], dp[idx + 3], dp[idx + 3]);
            val = (((uint64_t)dp[idx + 5]) << 40) | (((uint64_t)dp[idx + 4]) << 32) | (((uint64_t)dp[idx + 3]) << 24) | 
                  ((dp[idx + 2]) << 16) | ((dp[idx + 1]) << 8) | (dp[idx]);
//            TRACE_INFO("val = %ld (0x%08X)", val, val);
        }
        else
        {
            TRACE_ERR("Index %d too great", idx);
        }
    }
    else
    {
        TRACE_ERR("Null pointer");
    }

    return val;
}

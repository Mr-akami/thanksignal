#include "mbed.h"
#include "BLE.h"

#define NEED_CONSOLE_OUTPUT 1 /* Set this if you need debug messages on the console;
                               * it will have an impact on code-size and power consumption. */
#define WIRED

#if NEED_CONSOLE_OUTPUT
Serial  pc(USBTX, USBRX);
#define DEBUG(...) { pc.printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

//（石川）キャラクタリスティックのサイズ（単位：Byte）
#define IMAGEID_BYTE 1

Serial  serial(p19,p16);

const static uint8_t  DEVICE_NAME[] = "ThankSignal";//（石川）デバイス名の設定
static volatile bool  triggerSensorPolling = false;

BLEDevice  ble;

/* LEDs for indication: */
DigitalOut  oneSecondLed(LED1);        /* LED1 is toggled every second. */
DigitalOut  advertisingStateLed(LED2); /* LED2 is on when we are advertising, otherwise off. */

//（石川）サービスとキャラクタリスティックのUUIDを登録
const uint8_t UUID_THANKSIGNAL_SERVICE[] = {0x89,0x07,0x6e,0x79,0x82,0x23,0x44,0x24,0xbe,0xd6,0x0f,0xa5,0x21,0x1b,0x84,0xe3};
const uint8_t UUID_IMAGEID_CHAR[] = {0xea,0x02,0x52,0x2e,0xc4,0x52,0x45,0x39,0xbe,0x33,0xdf,0x1f,0x91,0xb1,0xc8,0xa3};

//（石川）キャラクタリスティックのメモリの設定
uint8_t image_id[IMAGEID_BYTE] = {0};

//（石川）Gattに登録するキャラクタリスティックの設定
//（石川）UUID, メモリへのポインタ，値の最小サイズ，値の最大サイズ，キャラクタリスティックのプロパティの設定
GattCharacteristic ImageID_Char(UUID_IMAGEID_CHAR, image_id,
                            sizeof(image_id),
                            sizeof(image_id),
                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE
                        |   GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

//（石川）Gattに登録するキャラクタリスティクをまとめたメモリの宣言と代入
GattCharacteristic *ThankSignal_chars[] = {&ImageID_Char};

//（石川）Gattに登録するサービスの宣言と代入
GattService ThankSignal_Service = GattService(UUID_THANKSIGNAL_SERVICE,
                            ThankSignal_chars,
                            sizeof(ThankSignal_chars)/sizeof(GattCharacteristic *));

/* Health Thermometer Service */ 
uint8_t             thermTempPayload[5] = { 0, 0, 0, 0, 0 };

GattCharacteristic  tempChar (GattCharacteristic::UUID_TEMPERATURE_MEASUREMENT_CHAR,
                                thermTempPayload, 5, 5,
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE);
/* Battery Level Service */
uint8_t            batt = 100;     /* Battery level */
uint8_t            read_batt = 0;  /* Variable to hold battery level reads */
GattCharacteristic battLevel ( GattCharacteristic::UUID_BATTERY_LEVEL_CHAR,     
                                 (uint8_t *)batt, 1, 1,
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *battChars[] = {&battLevel, };
GattService        battService(GattService::UUID_BATTERY_SERVICE, battChars,
                                sizeof(battChars) / sizeof(GattCharacteristic *));

uint16_t             uuid16_list[] = {GattService::UUID_BATTERY_SERVICE};

uint32_t quick_ieee11073_from_float(float temperature);

static Gap::ConnectionParams_t connectionParams;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)    // Mod
{
    advertisingStateLed = 1;
    
    DEBUG("Disconnected handle %u, reason %u\r\n", params->handle, params->reason);
    DEBUG("Restarting the advertising process\r\n");
    ble.gap().startAdvertising();
}

void onConnectionCallback(const Gap::ConnectionCallbackParams_t *params)   //Mod
{
    advertisingStateLed = 0;

    DEBUG("connected. Got handle %u\r\n", params->handle);

    connectionParams.slaveLatency = 1;
    if (ble.gap().updateConnectionParams(params->handle, &connectionParams) != BLE_ERROR_NONE) {
        DEBUG("failed to update connection paramter\r\n");
    }
}

void periodicCallback(void)
{
    oneSecondLed = !oneSecondLed; /* Do blinky on LED1 while we're waiting for BLE events */

    /* Note that the periodicCallback() executes in interrupt context, so it is safer to do
     * heavy-weight sensor polling from the main thread. */
    triggerSensorPolling = true;
}

//（石川）キャラクタリスティックに変更が加わったときのコールバック関数
void DataWrittenCallback(const GattWriteCallbackParams *params)
{
    if (params->handle == ImageID_Char.getValueAttribute().getHandle() && (params->len == IMAGEID_BYTE)) {
        memcpy(image_id, params->data, params->len);
        ble.gattServer().write(ImageID_Char.getValueHandle(), image_id, IMAGEID_BYTE);
        pc.printf("imageID:%d\n", image_id[0]);
        
        serial.putc(0xFF);            // start byte
        if(image_id[0] == 0xFF) {
            image_id[0] = 0xFE;
        }
        serial.putc(image_id[0]);    // image id

        return;
    }
}

/**************************************************************************/
/*!
    @brief  Program entry point
*/
/**************************************************************************/

int main(void)
{
    
    /* Setup blinky led */
    oneSecondLed = 1;
       
    DEBUG("Initialising the nRF51822\r\n");
    ble.init();
    DEBUG("Init done\r\n");
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(onConnectionCallback);
    ble.gattServer().onDataWritten(DataWrittenCallback);

    ble.gap().getPreferredConnectionParams(&connectionParams);

    /* setup advertising */
    ble.gap().setDeviceName(DEVICE_NAME);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t*)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_THERMOMETER);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (const uint8_t *)UUID_THANKSIGNAL_SERVICE, sizeof(UUID_THANKSIGNAL_SERVICE));
    
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.gap().startAdvertising();
    advertisingStateLed = 1;
    DEBUG("Start Advertising\r\n");

    ble.gattServer().addService(battService);
    DEBUG("Add Service\r\n");

    ble.addService(ThankSignal_Service);
    
    while (true) {
        if (triggerSensorPolling) {

        } else {
            ble.waitForEvent();
        }
    }
}


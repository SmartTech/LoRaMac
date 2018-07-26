
typedef struct __packed packed_s {
  uint64_t addr;
  uint16_t ppm[8];
  uint8_t  rssi;
} packed_t; // 25 byte;

#define USE_MODEM_LORA
//#define USE_MODEM_FSK

//------------------------------------------------

#define TX_OUTPUT_POWER     20 // dBm
#define RX_TIMEOUT_VALUE    50 // ms
#define BUFFER_SIZE         64 // Define the payload size here

//------------------------------------------------

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              2         // [0: 125 kHz,1: 250 kHz,2: 500 kHz,3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,2: 4/6,3: 4/7,4: 4/8]
#define LORA_PREAMBLE_LENGTH                        6         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true      // 
#define LORA_IQ_INVERSION_ON                        false     // 
#define LORA_TIMEOUT                                50        // ms

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                50000     // bps
#define FSK_BANDWIDTH                               1000000   // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           1000000   // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true      //
#define FSK_TIMEOUT                                 50        // ms

#else
    #error "Please define a modem in the compiler options."
#endif

//------------------------------------------------

//#define REGION_AS923
//#define REGION_AU915
//#define REGION_CN470
//#define REGION_CN779
//#define REGION_EU433
//#define REGION_EU868
//#define REGION_KR920
//#define REGION_IN865
//#define REGION_US915
#define REGION_RU864

//------------------------------------------------
#if defined( REGION_AS923 )
#define RF_FREQUENCY              923000000 // Hz
#elif defined( REGION_AU915 )
#define RF_FREQUENCY              915000000 // Hz
#elif defined( REGION_CN470 )
#define RF_FREQUENCY              470000000 // Hz
#elif defined( REGION_CN779 )
#define RF_FREQUENCY              779000000 // Hz
#elif defined( REGION_EU433 )
#define RF_FREQUENCY              433000000 // Hz
#elif defined( REGION_EU868 )
#define RF_FREQUENCY              868000000 // Hz
#elif defined( REGION_KR920 )
#define RF_FREQUENCY              920000000 // Hz
#elif defined( REGION_IN865 )
#define RF_FREQUENCY              865000000 // Hz
#elif defined( REGION_US915 )
#define RF_FREQUENCY              915000000 // Hz
#elif defined( REGION_RU864 )
#define RF_FREQUENCY              864000000 // Hz
#else
    #error "Please define a frequency band in the compiler options."
#endif



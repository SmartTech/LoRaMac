
#define DEBUG         Serial
#define DEBUG_BAUD    115200

typedef struct __packed packed_s {
  uint64_t addr;
  uint16_t ppm[8];
  uint8_t  rssi;
} packed_t; // 25 byte;

#define USE_MODEM_LORA
//#define USE_MODEM_FSK

//------------------------------------------------

#define TX_OUTPUT_POWER     20 // dBm
#define RX_TIMEOUT_VALUE   200 // ms
#define BUFFER_SIZE         20 // Define the payload size here

//------------------------------------------------

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              2         // [0: 125 kHz,1: 250 kHz,2: 500 kHz,3: Reserved]
#define LORA_SPREADING_FACTOR                       8         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,2: 4/6,3: 4/7,4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false     // 
#define LORA_IQ_INVERSION_ON                        false     // 
#define LORA_FHSS_ENABLED                           true      // 
#define LORA_NB_SYMB_HOP                            4         // 
#define LORA_CRC_ENABLED                            true      //
#define LORA_TIMEOUT                                100       // ms

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                50000     // bps
#define FSK_BANDWIDTH                               1000000   // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           1000000   // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true      //
#define FSK_TIMEOUT                                 100       // ms
#define FSK_CRC_ENABLED                             true      // 

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
#define REGION_US915
//#define REGION_RU864

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

/*!
 * Frequency hopping frequencies table
 */
const uint32_t HoppingFrequencies[] =
{
    916500000,
    923500000,
    906500000,
    917500000,
    917500000,
    909000000,
    903000000,
    916000000,
    912500000,
    926000000,
    925000000,
    909500000,
    913000000,
    918500000,
    918500000,
    902500000,
    911500000,
    926500000,
    902500000,
    922000000,
    924000000,
    903500000,
    913000000,
    922000000,
    926000000,
    910000000,
    920000000,
    922500000,
    911000000,
    922000000,
    909500000,
    926000000,
    922000000,
    918000000,
    925500000,
    908000000,
    917500000,
    926500000,
    908500000,
    916000000,
    905500000,
    916000000,
    903000000,
    905000000,
    915000000,
    913000000,
    907000000,
    910000000,
    926500000,
    925500000,
    911000000
};

#ifndef DEBUG
struct {
    template<typename... ARGS> void begin(ARGS...) {}
    template<typename... ARGS> void print(ARGS...) {}
    template<typename... ARGS> void println(ARGS...) {}
    template<typename... ARGS> void printf(ARGS...) {}
    template<typename... ARGS> void write(ARGS...) {}
    template<typename... ARGS> int  available(ARGS...) {}
    template<typename... ARGS> byte read(ARGS...) {}
   } DEBUG;
#endif
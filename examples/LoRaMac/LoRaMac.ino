#include <radio.h>
#include "RF_config.h"

#define LORA_LED    PB6
#define LORA_BTN    PA0
#define LORA_NSS    PB0
#define LORA_RST    PH1
#define LORA_VDD    PH0
#define LORA_CTRL   PA4
#define LORA_DIO0   PB10
#define LORA_DIO1   PB11
#define LORA_DIO2   PB12
#define LORA_DIO3   PIN_UNCONNECTED
#define LORA_DIO4   PIN_UNCONNECTED
#define LORA_DIO5   PIN_UNCONNECTED

void OnTxDone( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnTxTimeout( void );
void OnRxTimeout( void );
void OnRxError( void );

static RadioEvents_t RadioEvents;

typedef enum {
    RADIO_LOWPOWER,
    RADIO_RX_DONE,
    RADIO_RX_TIMEOUT,
    RADIO_RX_ERROR,
    RADIO_TX_DONE,
    RADIO_TX_TIMEOUT,
} States_t;

States_t State = RADIO_LOWPOWER;

uint16_t BufferSize = BUFFER_SIZE;
uint8_t  Buffer[BUFFER_SIZE];

int8_t   RssiValue = 0;
int8_t   SnrValue = 0;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

SPIClass RadioSPI;

packed_t packed;

void setup() 
{
    DEBUG.begin(DEBUG_BAUD);
    delay(1000);
    pinMode(LORA_LED, OUTPUT);

    DEBUG.println("Run");

    for(uint8_t i=0; i<sizeof(packed.ppm)/sizeof(uint16_t); i++) packed.ppm[i] = 1000;
    packed.addr = 100;
    packed.rssi = 0;
    
    // Radio initialization
    DEBUG.print("Init Radio...");
    RadioEvents.TxDone            = OnTxDone;
    RadioEvents.RxDone            = OnRxDone;
    RadioEvents.RxError           = OnRxError;
    RadioEvents.TxTimeout         = OnTxTimeout;
    RadioEvents.RxTimeout         = OnRxTimeout;
    RadioEvents.CadDone           = onCadDone;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    Radio.initIRQ(LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
    Radio.initDIO(LORA_NSS, LORA_RST, LORA_CTRL, LORA_VDD);
    Radio.init(&RadioSPI, &RadioEvents);
    DEBUG.println("OK!");

    DEBUG.print("Set channel from HoppingFrequencies[0] : ");
    DEBUG.print(HoppingFrequencies[0]);
    Radio.SetChannel( HoppingFrequencies[0] );
    DEBUG.println(" OK!");

    DEBUG.println("Config Radio :");
#if defined( USE_MODEM_LORA )

    DEBUG.println("-----------------------------");
    DEBUG.print  ("- MODE            : "); DEBUG.println("LORA");
    DEBUG.print  ("- RF_FREQUENCY    : "); DEBUG.println(RF_FREQUENCY);
    DEBUG.print  ("- POWER           : "); DEBUG.println(TX_OUTPUT_POWER);
    DEBUG.print  ("- BANDWIDTH       : "); DEBUG.println(LORA_BANDWIDTH);
    DEBUG.print  ("- CODINGRATE      : "); DEBUG.println(LORA_CODINGRATE);
    DEBUG.print  ("- SPREADING_FACTOR: "); DEBUG.println(LORA_SPREADING_FACTOR);
    DEBUG.print  ("- PREAMBLE_LENGTH : "); DEBUG.println(LORA_PREAMBLE_LENGTH);
    DEBUG.print  ("- FIX_LENGTH_ON   : "); DEBUG.println(LORA_FIX_LENGTH_PAYLOAD_ON);
    DEBUG.print  ("- IQ_INVERSION_ON : "); DEBUG.println(LORA_IQ_INVERSION_ON);
    DEBUG.println("-----------------------------");
    DEBUG.println();
    
    DEBUG.print("Set TX config... ");
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                                   LORA_IQ_INVERSION_ON, LORA_TIMEOUT );
    DEBUG.println("OK!");

    DEBUG.print("Set RX config... ");
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                                   LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                                   LORA_IQ_INVERSION_ON, true );
    DEBUG.println("OK!");

#elif defined( USE_MODEM_FSK )

    DEBUG.println("-----------------------------");
    DEBUG.print  ("- MODE            : "); DEBUG.println("FSK");
    DEBUG.print  ("- RF_FREQUENCY    : "); DEBUG.println(RF_FREQUENCY);
    DEBUG.print  ("- POWER           : "); DEBUG.println(TX_OUTPUT_POWER);
    DEBUG.print  ("- FDEV            : "); DEBUG.println(FSK_FDEV);
    DEBUG.print  ("- DATARATE        : "); DEBUG.println(FSK_DATARATE);
    DEBUG.print  ("- BANDWIDTH       : "); DEBUG.println(FSK_BANDWIDTH);
    DEBUG.print  ("- AFC_BANDWIDTH   : "); DEBUG.println(FSK_AFC_BANDWIDTH);
    DEBUG.print  ("- PREAMBLE_LENGTH : "); DEBUG.println(FSK_PREAMBLE_LENGTH);
    DEBUG.print  ("- FIX_LENGTH_ON   : "); DEBUG.println(FSK_FIX_LENGTH_PAYLOAD_ON);
    DEBUG.println("-----------------------------");
    DEBUG.println();

    DEBUG.print("Set TX config... ");
    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  FSK_CRC_ENABLED, 0, 0, 0, FSK_TIMEOUT );
    DEBUG.println("OK!");
                                  
    DEBUG.print("Set RX config... ");
    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                               0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                               0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, FSK_CRC_ENABLED,
                               0, 0, false, true );
    DEBUG.println("OK!");
#else
    #error "Please define a radio mode in RF_config.h"
#endif

//#if defined RF_FREQUENCY
//    DEBUG.print("Set frequency... ");
//    Radio.SetChannel( RF_FREQUENCY );
//    DEBUG.println("OK!");
//#else
//    #error "Please define a frequency band in RF_config.h"
//#endif


//    uint8_t role = 0; // 0 - reciever; 1- transmitter
//    if(role) {
//      DEBUG.print("Start transmit... ");
//      packed.rssi = 100;
//      Radio.Send( (uint8_t*)&packed, sizeof(packed) );
//      packed.rssi = 0;
//    } else {
//      DEBUG.print("Start recieve... ");
//      Radio.Rx( RX_TIMEOUT_VALUE );
//    }
//    DEBUG.println("OK!");

    Radio.Rx( RX_TIMEOUT_VALUE );
}

void loop() {

  static uint32_t rx_count    = 0;
  static uint32_t tx_count    = 0;
  static uint32_t rx_error    = 0;
  static uint32_t tx_timeout  = 0;
  static uint32_t cycle_count = 0;
  static uint32_t lastPackedMillis = 0;
  static uint32_t timeOnAir   = 0;
  static uint32_t packeds     = 0;

  static uint32_t lastMillis = 0;
  if(millis() - lastMillis >= 1000) {
    digitalWrite(LORA_LED, !digitalRead(LORA_LED));
    DEBUG.print("cycle_count: "); DEBUG.println(cycle_count);
    cycle_count = 0;
    if(rx_count) {
      DEBUG.print("rx_count  : "); DEBUG.println(rx_count);
    }
    if(tx_count) {
      DEBUG.print("tx_count  : "); DEBUG.println(tx_count);
    }
    if(rx_error) {
      DEBUG.print("rx_error  : "); DEBUG.println(rx_error);
    }
    if(tx_timeout) {
      DEBUG.print("tx_timeout: "); DEBUG.println(tx_timeout);
    }
    if(packed.rssi) {
      DEBUG.print("RSSI      : "); DEBUG.println(packed.rssi);
      DEBUG.print("Bitrate   : "); DEBUG.println(sizeof(packed)*8000/timeOnAir);
      DEBUG.print("Packed/s  : "); DEBUG.println(1000/timeOnAir);
      DEBUG.print("TimeOnAir : "); DEBUG.println(timeOnAir);
      DEBUG.print("Packeds   : "); DEBUG.println(packeds);
      DEBUG.print("Packed    : ");
      for(uint8_t i=0; i<sizeof(packed.ppm)/sizeof(uint16_t); i++) {
        DEBUG.print("["); DEBUG.print(packed.ppm[i]); DEBUG.print("]");
      }
      DEBUG.println();
      packed.rssi = 100;
      packeds = 0;
    }
    lastMillis = millis();
  }

  Radio.handle();

//  switch( State ) {
//
//      case RADIO_RX_DONE :
//          timeOnAir = millis() - lastPackedMillis;
//          lastPackedMillis = millis();
//          packeds++;
//          rx_count++;
//          rx_error = 0;
//          
//          Radio.Rx( RX_TIMEOUT_VALUE );
//          State = RADIO_LOWPOWER;
//          break;
//          
//      case RADIO_TX_DONE :
//          tx_count++;
//          tx_timeout = 0;
//          packed.rssi = 100;
//          Radio.Send( (uint8_t*)&packed, sizeof(packed) );
//          packed.rssi = 0;
//          State = RADIO_LOWPOWER;
//          break;
//      
//      case RADIO_RX_TIMEOUT :
//      case RADIO_RX_ERROR :
//          rx_error++;
//          Radio.Rx( RX_TIMEOUT_VALUE );
//          State = RADIO_LOWPOWER;
//          break;
//      
//      case RADIO_TX_TIMEOUT :
//          tx_timeout++;
//          packed.rssi = 100;
//          Radio.Send( (uint8_t*)&packed, sizeof(packed) );
//          packed.rssi = 0;
//          State = RADIO_LOWPOWER;
//          break;
//
//      case RADIO_LOWPOWER :
//      default: break; // Set low power
//
//  }
//
//  return;
  
  //---------------------------------------------------
  
  static bool isMaster = true;

  switch( State ) {

      case RADIO_RX_DONE :
        if( isMaster == true )
        {
              if( BufferSize > 0 )
              {
                  if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 ) {
                      // Send the next PING frame
                      strcpy( ( char* )Buffer, ( char* )PingMsg );
                      // We fill the buffer with numbers for the payload
                      for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
                      Radio.Send( Buffer, BufferSize );
                  }
                  else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 ) {
                      // A master already exists then become a slave
                      isMaster = false;
                      // Send the next PONG frame
                      strcpy( ( char* )Buffer, ( char* )PongMsg );
                      // We fill the buffer with numbers for the payload
                      for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
                      Radio.Send( Buffer, BufferSize );
                  } else {
                      // valid reception but neither a PING or a PONG message
                      // Set device as master ans start again
                      isMaster = true;
                      Radio.Rx( RX_TIMEOUT_VALUE );
                  }
              }
          } else {
              if( BufferSize > 0 ) {
                  if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 ) {
                      strcpy( ( char* )Buffer, ( char* )PongMsg );
                      for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
                      Radio.Send( Buffer, BufferSize );
                  } else {
                      // valid reception but not a PING as expected
                      // Set device as master and start again
                      isMaster = true;
                      Radio.Rx( RX_TIMEOUT_VALUE );
                  }
              }
          }
          rx_error = 0;
          rx_count++;
          State = RADIO_LOWPOWER;
          break;
        
      case RADIO_TX_DONE :
          tx_count++;
          Radio.Rx( RX_TIMEOUT_VALUE );
          State = RADIO_LOWPOWER;
          break;
          
      case RADIO_RX_TIMEOUT :
          if( isMaster == true ) {
              // Send the next PING frame
              strcpy( ( char* )Buffer, ( char* )PingMsg );
              for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
              Radio.Send( Buffer, BufferSize );
          } else {
              Radio.Rx( RX_TIMEOUT_VALUE );
          }
          State = RADIO_LOWPOWER;
          break;
          
      case RADIO_RX_ERROR :
          if( isMaster == true ) {
              // Send the next PING frame
              strcpy( ( char* )Buffer, ( char* )PingMsg );
              for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
              Radio.Send( Buffer, BufferSize );
          } else {
              // Send the next PONG frame
              strcpy( ( char* )Buffer, ( char* )PongMsg );
              for( uint8_t i = 4; i < BufferSize; i++ ) Buffer[i] = i - 4;
              Radio.Send( Buffer, BufferSize );
          }
          rx_error++;
          State = RADIO_LOWPOWER;
          break;

      case RADIO_TX_TIMEOUT :
          Radio.Rx( RX_TIMEOUT_VALUE );
          State = RADIO_LOWPOWER;
          break;
            
      case RADIO_LOWPOWER : break;
      default: State = RADIO_LOWPOWER; break; // Set low power
  }
  
  cycle_count++;

}

void OnTxDone( void )
{
    Radio.SetChannel( HoppingFrequencies[0] );
    Radio.Sleep( );
    State = RADIO_TX_DONE;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.SetChannel( HoppingFrequencies[0] );
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RADIO_RX_DONE;
}

void OnTxTimeout( void )
{
    Radio.SetChannel( HoppingFrequencies[0] );
    Radio.Sleep( );
    State = RADIO_TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.SetChannel( HoppingFrequencies[0] );
    Radio.Sleep( );
    Buffer[BufferSize] = 0;
    State = RADIO_RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.SetChannel( HoppingFrequencies[0] );
    Radio.Sleep( );
    State = RADIO_RX_ERROR;
}
 
void OnFhssChangeChannel( uint8_t channelIndex )
{
    DEBUG.print("Change channel to HoppingFrequencies[");
    DEBUG.print(channelIndex);
    DEBUG.print("] : ");
    DEBUG.print(HoppingFrequencies[channelIndex]);
    Radio.SetChannel( HoppingFrequencies[channelIndex] );
    DEBUG.println(" OK!");
}

void onCadDone( bool channelActivityDetected ) {
    DEBUG.println("CadDone!");
}

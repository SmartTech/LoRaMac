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
    Serial.begin(115200);
    delay(1000);
    pinMode(LORA_LED, OUTPUT);

    Serial.println("Run");

    for(uint8_t i=0; i<sizeof(packed.ppm)/sizeof(uint16_t); i++) packed.ppm[i] = 1000;
    packed.addr = 100;
    packed.rssi = 0;
    
    // Radio initialization
    RadioEvents.TxDone    = OnTxDone;
    RadioEvents.RxDone    = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError   = OnRxError;

    Serial.print("Init Radio...");
    Radio.initIRQ(LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
    Radio.initDIO(LORA_NSS, LORA_RST, LORA_CTRL, LORA_VDD);
    Radio.init(&RadioSPI, &RadioEvents);
    Serial.println("OK!");

    Serial.println("Config Radio :");
#if defined( USE_MODEM_LORA )

    Serial.println("-----------------------------");
    Serial.print  ("- MODE            : "); Serial.println("LORA");
    Serial.print  ("- RF_FREQUENCY    : "); Serial.println(RF_FREQUENCY);
    Serial.print  ("- POWER           : "); Serial.println(TX_OUTPUT_POWER);
    Serial.print  ("- BANDWIDTH       : "); Serial.println(LORA_BANDWIDTH);
    Serial.print  ("- CODINGRATE      : "); Serial.println(LORA_CODINGRATE);
    Serial.print  ("- SPREADING_FACTOR: "); Serial.println(LORA_SPREADING_FACTOR);
    Serial.print  ("- PREAMBLE_LENGTH : "); Serial.println(LORA_PREAMBLE_LENGTH);
    Serial.print  ("- FIX_LENGTH_ON   : "); Serial.println(LORA_FIX_LENGTH_PAYLOAD_ON);
    Serial.print  ("- IQ_INVERSION_ON : "); Serial.println(LORA_IQ_INVERSION_ON);
    Serial.println("-----------------------------");
    Serial.println();
    
    Serial.print("Set TX config... ");
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, LORA_TIMEOUT );
    Serial.println("OK!");

    Serial.print("Set RX config... ");
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    Serial.println("OK!");

#elif defined( USE_MODEM_FSK )

    Serial.println("-----------------------------");
    Serial.print  ("- MODE            : "); Serial.println("FSK");
    Serial.print  ("- RF_FREQUENCY    : "); Serial.println(RF_FREQUENCY);
    Serial.print  ("- POWER           : "); Serial.println(TX_OUTPUT_POWER);
    Serial.print  ("- FDEV            : "); Serial.println(FSK_FDEV);
    Serial.print  ("- DATARATE        : "); Serial.println(FSK_DATARATE);
    Serial.print  ("- BANDWIDTH       : "); Serial.println(FSK_BANDWIDTH);
    Serial.print  ("- AFC_BANDWIDTH   : "); Serial.println(FSK_AFC_BANDWIDTH);
    Serial.print  ("- PREAMBLE_LENGTH : "); Serial.println(FSK_PREAMBLE_LENGTH);
    Serial.print  ("- FIX_LENGTH_ON   : "); Serial.println(FSK_FIX_LENGTH_PAYLOAD_ON);
    Serial.println("-----------------------------");
    Serial.println();

    Serial.print("Set TX config... ");
    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, FSK_TIMEOUT );
    Serial.println("OK!");
                                  
    Serial.print("Set RX config... ");
    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                               0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                               0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                               0, 0, false, true );
    Serial.println("OK!");
#else
    #error "Please define a radio mode in RF_config.h"
#endif


#if defined RF_FREQUENCY
    Serial.print("Set frequency... ");
    Radio.SetChannel( RF_FREQUENCY );
    Serial.println("OK!");
#else
    #error "Please define a frequency band in RF_config.h"
#endif


    uint8_t role = 0; // 0 - reciever; 1- transmitter
    if(role) {
      Serial.print("Start transmit... ");
      packed.rssi = 100;
      Radio.Send( (uint8_t*)&packed, sizeof(packed) );
      packed.rssi = 0;
    } else {
      Serial.print("Start recieve... ");
      Radio.Rx( RX_TIMEOUT_VALUE );
    }
    Serial.println("OK!");
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
    Serial.print("cycle_count: "); Serial.println(cycle_count);
    cycle_count = 0;
    if(rx_count) {
      Serial.print("rx_count  : "); Serial.println(rx_count);
    }
    if(tx_count) {
      Serial.print("tx_count  : "); Serial.println(tx_count);
    }
    if(rx_error) {
      Serial.print("rx_error  : "); Serial.println(rx_error);
    }
    if(tx_timeout) {
      Serial.print("tx_timeout: "); Serial.println(tx_timeout);
    }
    if(packed.rssi) {
      Serial.print("RSSI      : "); Serial.println(packed.rssi);
      Serial.print("Bitrate   : "); Serial.println(sizeof(packed)*8000/timeOnAir);
      Serial.print("Packed/s  : "); Serial.println(1000/timeOnAir);
      Serial.print("TimeOnAir : "); Serial.println(timeOnAir);
      Serial.print("Packeds   : "); Serial.println(packeds);
      Serial.print("Packed    : ");
      for(uint8_t i=0; i<sizeof(packed.ppm)/sizeof(uint16_t); i++) {
        Serial.print("["); Serial.print(packed.ppm[i]); Serial.print("]");
      }
      Serial.println();
      packed.rssi = 100;
      packeds = 0;
    }
    lastMillis = millis();
  }

  Radio.handle();

  switch( State ) {

      case RADIO_RX_DONE :
          timeOnAir = millis() - lastPackedMillis;
          lastPackedMillis = millis();
          packeds++;
          rx_count++;
          rx_error = 0;
          
          Radio.Rx( RX_TIMEOUT_VALUE );
          State = RADIO_LOWPOWER;
          break;
          
      case RADIO_TX_DONE :
          tx_count++;
          tx_timeout = 0;
          packed.rssi = 100;
          Radio.Send( (uint8_t*)&packed, sizeof(packed) );
          packed.rssi = 0;
          State = RADIO_LOWPOWER;
          break;
      
      case RADIO_RX_TIMEOUT :
      case RADIO_RX_ERROR :
          rx_error++;
          Radio.Rx( RX_TIMEOUT_VALUE );
          State = RADIO_LOWPOWER;
          break;
      
      case RADIO_TX_TIMEOUT :
          tx_timeout++;
          packed.rssi = 100;
          Radio.Send( (uint8_t*)&packed, sizeof(packed) );
          packed.rssi = 0;
          State = RADIO_LOWPOWER;
          break;

      case RADIO_LOWPOWER :
      default: break; // Set low power

  }

  return;
  
  //---------------------------------------------------
  
  static bool isMaster = true;

  switch( State ) {

      case RADIO_RX_DONE :
        if( isMaster == true )
          {
              if( BufferSize > 0 )
              {
                  if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                  {
                      // Send the next PING frame
                      Buffer[0] = 'P';
                      Buffer[1] = 'I';
                      Buffer[2] = 'N';
                      Buffer[3] = 'G';
                      // We fill the buffer with numbers for the payload
                      for( uint8_t i = 4; i < BufferSize; i++ )
                      {
                          Buffer[i] = i - 4;
                      }
                      Radio.Send( Buffer, BufferSize );
                  }
                  else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                  { // A master already exists then become a slave
                      isMaster = false;
                      Radio.Rx( RX_TIMEOUT_VALUE );
                  }
                  else // valid reception but neither a PING or a PONG message
                  {    // Set device as master ans start again
                      isMaster = true;
                      Radio.Rx( RX_TIMEOUT_VALUE );
                  }
              }
          }
          else
          {
              if( BufferSize > 0 )
              {
                  if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                  {
                      // Send the reply to the PONG string
                      Buffer[0] = 'P';
                      Buffer[1] = 'O';
                      Buffer[2] = 'N';
                      Buffer[3] = 'G';
                      // We fill the buffer with numbers for the payload
                      for( uint8_t i = 4; i < BufferSize; i++ )
                      {
                          Buffer[i] = i - 4;
                      }
                      Radio.Send( Buffer, BufferSize );
                  }
                  else // valid reception but not a PING as expected
                  {    // Set device as master and start again
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
          
      case RADIO_RX_ERROR :
          if( isMaster == true )
          {
              // Send the next PING frame
              Buffer[0] = 'P';
              Buffer[1] = 'I';
              Buffer[2] = 'N';
              Buffer[3] = 'G';
              for( uint8_t i = 4; i < BufferSize; i++ )
              {
                  Buffer[i] = i - 4;
              }
              Radio.Send( Buffer, BufferSize );
          }
          else
          {
              Radio.Rx( RX_TIMEOUT_VALUE );
          }
          rx_error++;
          State = RADIO_LOWPOWER;
          break;

      case RADIO_TX_TIMEOUT :
          Radio.Rx( RX_TIMEOUT_VALUE );
          State = RADIO_LOWPOWER;
          break;
            
      case RADIO_LOWPOWER :
      default: break; // Set low power
  }
  
  cycle_count++;

}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = RADIO_TX_DONE;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    if(size==sizeof(packed)) memcpy( &packed, payload, size );
    //memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RADIO_RX_DONE;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = RADIO_TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RADIO_RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RADIO_RX_ERROR;
}

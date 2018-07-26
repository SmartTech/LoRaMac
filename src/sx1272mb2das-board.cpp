
#include <Arduino.h>
#include "radio.h"
#include "sx1272-board.h"

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1272GetPaSelect( uint32_t channel );

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
	SX1272InitIRQ,
	SX1272InitDIO,
    SX1272Init,
    SX1272Handle,
    SX1272GetStatus,
    SX1272SetModem,
    SX1272SetChannel,
    SX1272IsChannelFree,
    SX1272Random,
    SX1272SetRxConfig,
    SX1272SetTxConfig,
    SX1272CheckRfFrequency,
    SX1272GetTimeOnAir,
    SX1272Send,
    SX1272SetSleep,
    SX1272SetStby,
    SX1272SetRx,
    SX1272StartCad,
    SX1272SetTxContinuousWave,
    SX1272ReadRssi,
    SX1272Write,
    SX1272Read,
    SX1272WriteBuffer,
    SX1272ReadBuffer,
    SX1272SetMaxPayloadLength,
    SX1272SetPublicNetwork,
    SX1272GetWakeupTime
};


void SX1272IoInit( void )
{
	if(SX1272.nss  != PIN_UNCONNECTED) {
		pinMode(SX1272.nss, OUTPUT);
	}
	if(SX1272.vdd  != PIN_UNCONNECTED) {
		pinMode(SX1272.vdd, OUTPUT);
		digitalWrite(SX1272.vdd, HIGH);
	}
}

void SX1272IoDeInit( void )
{
	if(SX1272.nss != PIN_UNCONNECTED) pinMode(SX1272.nss, INPUT);
	if(SX1272.vdd != PIN_UNCONNECTED) pinMode(SX1272.vdd, INPUT);
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
	if(SX1272.dio0 != PIN_UNCONNECTED) attachInterrupt( SX1272.dio0, irqHandlers[0], RISING );
	if(SX1272.dio1 != PIN_UNCONNECTED) attachInterrupt( SX1272.dio1, irqHandlers[1], RISING );
	if(SX1272.dio2 != PIN_UNCONNECTED) attachInterrupt( SX1272.dio2, irqHandlers[2], RISING );
	if(SX1272.dio3 != PIN_UNCONNECTED) attachInterrupt( SX1272.dio3, irqHandlers[3], RISING );
	if(SX1272.dio4 != PIN_UNCONNECTED) attachInterrupt( SX1272.dio4, irqHandlers[4], RISING );
}

/*!
 * \brief Enables/disables the TCXO if available on board design.
 * \param [IN] state TCXO enabled when true and disabled when false.
 */
static void SX1272SetBoardTcxo( uint8_t state )
{
    // No TCXO component available on this board design.
#if 0
    if( state == true ) {
        TCXO_ON( );
        delay( BOARD_TCXO_WAKEUP_TIME );
    } else {
        TCXO_OFF( );
    }
#endif
}

uint32_t SX1272GetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX1272Reset( void )
{
	if(SX1272.rst==PIN_UNCONNECTED) return;
    // Enables the TCXO if available on the board design
    SX1272SetBoardTcxo( true );
	pinMode(SX1272.rst, OUTPUT);
	digitalWrite(SX1272.rst, HIGH);
    delay(1);
	digitalWrite(SX1272.rst, LOW);
	pinMode(SX1272.rst, INPUT);
    delay(6);
}

void SX1272SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1272Read( REG_PACONFIG );
    paDac = SX1272Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1272GetPaSelect( SX1272.Settings.Channel );

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 ) paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        else             paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
		
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
			power = constrain(power, 5, 20);
            paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
			power = constrain(power, 2, 17);
            paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
		power = constrain(power, -1, 14);
        paConfig = ( paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }
    SX1272Write( REG_PACONFIG, paConfig );
    SX1272Write( REG_PADAC, paDac );
}

static uint8_t SX1272GetPaSelect( uint32_t channel )
{
    //return RF_PACONFIG_PASELECT_RFO;
	return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1272SetAntSwLowPower( bool status )
{
    if( RadioIsActive != status )
    {
        RadioIsActive = status;

        if( status == false )
        {
            SX1272SetBoardTcxo( true );
            SX1272AntSwInit( );
        }
        else
        {
            SX1272SetBoardTcxo( false );
            SX1272AntSwDeInit( );
        }
    }
}

void SX1272AntSwInit( void )
{
	if(SX1272.ctrl==PIN_UNCONNECTED) return;
	pinMode(SX1272.ctrl, OUTPUT);
	digitalWrite(SX1272.ctrl, LOW);
}

void SX1272AntSwDeInit( void )
{
	if(SX1272.ctrl==PIN_UNCONNECTED) return;
	digitalWrite(SX1272.ctrl, LOW);
	pinMode(SX1272.ctrl, INPUT);
}

void SX1272SetAntSw( uint8_t opMode )
{
    switch( opMode )
    {
    case RFLR_OPMODE_TRANSMITTER:
        if(SX1272.ctrl != PIN_UNCONNECTED) digitalWrite( SX1272.ctrl, HIGH );
        break;
    case RFLR_OPMODE_RECEIVER:
    case RFLR_OPMODE_RECEIVER_SINGLE:
    case RFLR_OPMODE_CAD:
    default:
        if(SX1272.ctrl != PIN_UNCONNECTED) digitalWrite( SX1272.ctrl, LOW );
        break;
    }
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

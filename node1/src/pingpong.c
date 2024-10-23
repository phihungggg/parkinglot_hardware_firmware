/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_system.h"
#include "tremo_gpio.h"
#include "tremo_timer.h"

#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 // Hz

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 // Hz

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 // Hz

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_US915_HYBRID )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             14        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                50000     // bps
#define FSK_BANDWIDTH                               50000   // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           83333   // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
    RX_WRONG_PACKET
}States_t;

#define RX_TIMEOUT_VALUE                            1800
#define BUFFER_SIZE                                 5 // Define the payload size here
#define BUFFER2_SIZE                                5




extern gpio_t *g_test_gpiox;
extern uint8_t g_test_pin;
extern uint8_t gss;
extern uint8_t count ;
extern uint8_t Signal_LED;
//extern uint8_t gss1;
//uint8_t gss2;
uint8_t check_PONG2  =0;
uint8_t txdone=0;
uint8_t v_send_data=0;
uint8_t v_resend_data=0;
const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";
const uint8_t Pong2Msg[] = "PONG2";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE]="HUHU";

uint8_t Buffer2;
uint8_t g_gpio_interrupt_flag=0;

volatile States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

uint32_t ChipId[2] = {0};

void send_data(gpio_level_t getstatus);
void resend_data(gpio_level_t getstatus);
void deal_with_PONG(uint8_t *Bf, uint16_t sizeofbf );
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/**
 * Main application entry point.
 */
int app_start( void )
{
    bool isMaster = true;
    uint8_t i;
    uint32_t random;

    printf("PingPong test Start!\r\n");

    (void)system_get_chip_id(ChipId);

    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );
    

    Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif



    while( 1 )
    {
    
        if (g_gpio_interrupt_flag==1)
        {
            g_gpio_interrupt_flag=0;
            printf(" send data \n");
            DelayMs(10);
        send_data(gpio_read(g_test_gpiox,g_test_pin));
        // timer_cmd(TIMER0, true);
            v_send_data=1;
        }


        if (State==TX && v_send_data==1)
        {
        v_send_data=0;
        printf(" resend data \n");
        State=LOWPOWER;
        send_data(gpio_read(g_test_gpiox,g_test_pin));
        

        v_resend_data=1;
       // timer_cmd(TIMER0,true);
        }





        if (State==TX && v_resend_data==1)
        {   v_resend_data=0;
        State=LOWPOWER;
        Radio.RxBoosted(2500);
      

        }

        if (State==RX_TIMEOUT)
        {
            count++;   
            gpio_toggle(g_test_gpiox,Signal_LED);

            if (count<3)
            {State=TX;
            v_resend_data=1;}

            
        }
       
        if (count==3)
        { count=0;
             State=TX;
           v_send_data=1;

        }


        if (State==RX_WRONG_PACKET)
        {   printf(" wrong packet");
            // gpio_write(g_test_gpiox,Signal_LED,GPIO_LEVEL_LOW);
              Radio.RxBoosted(2500);
            State=LOWPOWER;
        }

        if (State==RX)
        {
        send_data(gpio_read(g_test_gpiox,g_test_pin));

        }


      // printf(" %s ",Buffer);
        // Process Radio IRQ
        Radio.IrqProcess( );
        
      // printf(" %s ",Buffer);
        // Process Radio IRQ
        Radio.IrqProcess( );
    }
}

void resend_data(gpio_level_t getstatus)
{
Buffer[0]='P';
Buffer[1]='I';
Buffer[2]='N';
Buffer[3]='G';
Buffer[4]='2';
Buffer[5]=getstatus ? '1' : '0';


    Radio.Send(Buffer,sizeof(Buffer)+1);

printf(" send data %d \n",getstatus);
 //DelayMs( 10 );
//Radio.IrqProcess( );
 
}



void send_data(gpio_level_t getstatus)
{
 
 Buffer[0]='P';
Buffer[1]='I';
Buffer[2]='N';
Buffer[3]='G';
Buffer[4]='2';
Buffer[5]=getstatus ? '1' : '0';



    Radio.Send(Buffer,sizeof(Buffer)+1);
// printf(" %s ",Buffer2);
            
printf(" send data %d \n",getstatus);
 //DelayMs( 10 );
//Radio.IrqProcess( );

}

void deal_with_PONG(uint8_t *Bf, uint16_t sizeofbf )
{
    if( strncmp( ( const char* )Bf, ( const char* )Pong2Msg, 5 ) == 0 )  
  {printf(" pong2 received ");
  gpio_write(g_test_gpiox,Signal_LED,GPIO_LEVEL_HIGH);
            }
    else 
    {
       // printf(" continue to receive");
     State=RX_WRONG_PACKET;
    Radio.Rx(0);
    }
}



void OnTxDone( void )
{printf(" on TX done \n");
    Radio.Sleep( );
    State = TX;
    txdone=1;
}


void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    printf(" On Rx done ");
    deal_with_PONG(Buffer,BufferSize);
     timer_cmd(TIMER0,true); 
   // Radio.Rx( 0);
}

void OnTxTimeout( void )
{    printf(" on TX timeout \n");
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    printf("OnRxTimeout\r\n");
    Radio.Sleep( );
    State = RX_TIMEOUT;
    // timer_cmd(TIMER0,true); 
    
}

void OnRxError( void )
{   printf(" on RX Error \n");
    Radio.Sleep( );
    State = RX_ERROR;
}

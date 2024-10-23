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
#include "tremo_timer.h"

uint8_t count=0;
uint8_t rxdone=0;
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
    TX_TIMEOUT
}States_t;

#define RX_TIMEOUT_VALUE                            1800
#define BUFFER_SIZE                                 7 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint8_t txdone=0; 
//extern uint8_t gss;
//extern uint8_t gss1;
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE]="HUHU7U";

volatile States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;
uint8_t getstatusnode[3];
uint32_t ChipId[2] = {0};

char sensor_information[2];
//uint8_t count=0;
//uint8_t checkReceive=0;
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


void deal_with_node_messages(void);


void send_data(char nodenumber);

void resend_data(uint8_t nodenumber);

/**
 * Main application entry point.
 */
int app_start( void )
{
   

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

    Radio.RxBoosted( 0 );

    while( 1 )
    {
        DelayMs(10);
           // if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
          //  printf(" %s ", Buffer);
        
        
            //deal_with_node_messages();
          //  if (checkReceive==1)
           // {
                  //  checkReceive=0;
                    //deal_with_node_messages();

           // }

            if (rxdone==1)
            {   //printf(" Timer0 occur");
               rxdone=0;
               deal_with_node_messages();
               // timer_cmd(TIMER0,false);
               // timer_cmd(TIMER1, true);
               
               // Radio.RxBoosted(0);
                /* code */
            }
         /* 
            if ( count>=0 && count<4 && txdone==1)
            {
                    txdone=0;
                    char nodenumber= (char)Buffer[4];
                    send_data(nodenumber);
                if (count==3)
                count=-2;
            }
*/

            if(txdone)
            {   txdone=0;

                Radio.RxBoosted(0);
            }
        // Process Radio IRQ



        Radio.IrqProcess( );
    }
}

void send_data(char nodenumber)
{
    uint8_t data[6]="PONG" ;
    data[4]=nodenumber;
   // printf("i did send %c",data[4]);
    //printf("%s",data);
    Radio.Send(data,6);
}

void resend_data(uint8_t nodenumber)
{



}



void deal_with_node_messages(void)
{   
    uint8_t Buffer2[5];
    for(int i=0;i<4;++i)
    {
        Buffer2[i]=Buffer[i];
       // printf(" %c ",Buffer2[i]);
    }
    

        if( strncmp( ( const char* )Buffer2, ( const char* )PingMsg, 4 ) == 0 )   
    { //  printf(" %s ", (char*)Buffer2);
     //   printf(" sao no eo chay ");
   
    sensor_information[0] = (char)Buffer[4];
    sensor_information[1]= (char)Buffer[5];
    //for (int j=0;j<2;++j)
            send_data(*sensor_information);
            for (int i=0;i<2;++i)
            printf("%c",sensor_information[i]);
}

}


void OnTxDone( void )
{//printf(" on TX done \n");
    Radio.Sleep( );
    State = TX;
    txdone=1;
    count++;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  //  printf(" On Rx Done");
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
   rxdone=1;
   // printf( " rssivalue : %d \n snrvalue : %d ",RssiValue,SnrValue);
    
   // checkReceive=1;
   // deal_with_node_messages();
  //  timer_cmd(TIMER0, true);
   // for (int i=0;i<100000;++i)
    //    {}
    // Radio.Rx(0);
}

void OnTxTimeout( void )
{   // printf(" on TX timeout \n");
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
  //  printf("OnRxTimeout\r\n");
    Radio.Sleep( );
    State = RX_TIMEOUT;
    
}

void OnRxError( void )
{  // printf(" on RX Error \n");
    Radio.Sleep( );
    State = RX_ERROR;
}

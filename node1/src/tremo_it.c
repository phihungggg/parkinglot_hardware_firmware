
#include "tremo_it.h"
#include "tremo_gpio.h"
#include "stdio.h"
#include "tremo_timer.h"
extern void RadioOnDioIrq(void);
extern void RtcOnIrq(void);
//extern void send_data(gpio_level_t getstatus);
extern gpio_t *g_test_gpiox;
extern uint8_t g_test_pin;
extern uint8_t g_gpio_interrupt_flag;
//extern uint8_t count;
//extern uint_fast16_t gss;
/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{

    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
}

/**
 * @brief  This function handles PWR Handler.
 * @param  None
 * @retval None
 */
void PWR_IRQHandler()
{
}

/******************************************************************************/
/*                 Tremo Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_cm4.S).                                               */
/******************************************************************************/

void LORA_IRQHandler()
{
    RadioOnDioIrq();
}

void RTC_IRQHandler(void)
{
    RtcOnIrq();
}




void GPIO_IRQHandler(void)
{
    if (gpio_get_interrupt_status(g_test_gpiox, g_test_pin) == SET) {
        gpio_clear_interrupt(g_test_gpiox, g_test_pin);
        g_gpio_interrupt_flag = 1;
   
        //printf(" interrupt does occur ");

    }
}




void TIMER0_IRQHandler(void)
{ //send_data(gpio_read(g_test_gpiox,g_test_pin));
 
    gptim0_IRQHandler();
    //printf(" chotto matte");
   //timer_cmd(TIMER0, false); 
}


void TIMER1_IRQHandler(void)
{
      gptim1_IRQHandler();


}
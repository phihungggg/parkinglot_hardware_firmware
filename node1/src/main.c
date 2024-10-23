#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "tremo_delay.h"
#include "rtc-board.h"
#include "tremo_timer.h"

gpio_t *g_test_gpiox = GPIOA;
uint8_t g_test_pin = GPIO_PIN_5;
uint8_t Signal_LED = GPIO_PIN_15;

uint8_t count = 0;
extern int app_start(void);

 uint8_t gss=0;
 uint8_t gss1=0;
void timer_init_hehe(timer_gp_t* TIMERx)
{
timer_init_t timerx_init;

    timer_config_interrupt(TIMERx, TIMER_DIER_UIE, ENABLE);

    timerx_init.prescaler          = 65000-1;  //sysclock defaults to 24M, is divided by (prescaler + 1) to 1k
    timerx_init.counter_mode       = TIMER_COUNTERMODE_UP;
    timerx_init.period             = 1000;   //time period is ((1 / 1k) * 1000) 
    timerx_init.clock_division     = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
    timer_init(TIMERx, &timerx_init);

    timer_generate_event(TIMERx, TIMER_EGR_UG, ENABLE);
    timer_clear_status(TIMERx, TIMER_SR_UIF);

    //timer_cmd(TIMERx, true);

    printf(" init timer ");


}
void timer1_init_hehe(timer_gp_t* TIMERx)
{
timer_init_t timerx_init;

    timer_config_interrupt(TIMERx, TIMER_DIER_UIE, ENABLE);

    timerx_init.prescaler          = 10000-1;  //sysclock defaults to 24M, is divided by (prescaler + 1) to 1k
    timerx_init.counter_mode       = TIMER_COUNTERMODE_UP;
    timerx_init.period             = 1000;   //time period is ((1 / 1k) * 1000) 
    timerx_init.clock_division     = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
    timer_init(TIMERx, &timerx_init);

    timer_generate_event(TIMERx, TIMER_EGR_UG, ENABLE);
    timer_clear_status(TIMER1, TIMER_SR_UIF);

    //timer_cmd(TIMERx, true);

    printf(" init timer ");


}


void init_led()
{
        rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    gpio_init(GPIOA, Signal_LED, GPIO_MODE_OUTPUT_PP_HIGH);

}

void uart_log_init(void)
{
    // uart0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = UART_BAUDRATE_115200;
    uart_init(CONFIG_DEBUG_UART, &uart_config);
    uart_cmd(CONFIG_DEBUG_UART, ENABLE);
}
void interrupt_configuration()
{

 gpio_init(g_test_gpiox, g_test_pin, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, g_test_pin, GPIO_INTR_RISING_FALLING_EDGE);

    /* NVIC config */
    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 1);

    

     NVIC_EnableIRQ(TIMER0_IRQn);
   NVIC_SetPriority(TIMER0_IRQn, 2);


    NVIC_EnableIRQ(TIMER1_IRQn);
   NVIC_SetPriority(TIMER1_IRQn, 2);

}
void board_init()
{
    rcc_enable_oscillator(RCC_OSC_XO32K, true);

    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER0, true);
   // rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER1, true);
    // rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER1, true);
    //rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER2, true);
   // rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER3, true);
    
    delay_ms(100);
    pwr_xo32k_lpm_cmd(true);
    
    uart_log_init();

    RtcInit();
}

void gptim0_IRQHandler(void)
{   
    //count++;
    //timer_cmd(TIMER0, false);
    bool state;
    gss=1;
    timer_get_status(TIMER0, TIMER_SR_UIF, &state);

    if (state) {
        timer_clear_status(TIMER0, TIMER_SR_UIF);
    }
    // 
}


   void  gptim1_IRQHandler(void)
   {

 bool state;
    gss1=1;
    timer_get_status(TIMER1, TIMER_SR_UIF, &state);

    if (state) {
        timer_clear_status(TIMER1, TIMER_SR_UIF);
    }
    // 


   }
int main(void)
{
    // Target board initialization
    board_init();
    timer_init_hehe(TIMER0);
    init_led();
   // timer_init_hehe(TIMER1);
    interrupt_configuration();
    app_start();
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif

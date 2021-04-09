#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#define DEBOUNCE_DELAY 300U

osThreadId_t threadLed_id;
osSemaphoreId_t push_button_id;

uint32_t lastTimeRun = 0;

void SW1_Handler()
{
  int now = osKernelGetTickCount();
  
  if(now - DEBOUNCE_DELAY >= lastTimeRun) {
    lastTimeRun = now;
    osSemaphoreRelease(push_button_id);   
  }
  
  GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0); 
}

void InitSW1()
{ 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilita??o
  
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTJ_BASE ,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
  GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);
  GPIOIntRegister(GPIO_PORTJ_BASE, SW1_Handler);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
  
  osDelay(100);
}

void threadLed(void *arg){
  uint8_t count = 0;
  
  while(1){
    osSemaphoreAcquire(push_button_id, osWaitForever);
    
    count++;
    count &= 0x0F;
    if(count>0x0F) count = 0;
  
    LEDWrite(LED4 | LED3 | LED2 | LED1, count);
  } // while
} // threadLed

void main(void){
  SystemInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);
  InitSW1();
  
  osKernelInitialize();

  threadLed_id = osThreadNew(threadLed, NULL, NULL);

  push_button_id = osSemaphoreNew(1, 0, NULL); // espaços ocupados = 0
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main

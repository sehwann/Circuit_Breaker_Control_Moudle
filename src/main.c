/**
 **************************************************************************
 * File Name    : main.c
 * Description  : None
 * Version      : V1.0.0
 **************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#pragma pack(1)
#include <stdio.h>
#include <string.h>
#include "at32f4xx.h"
#include "at32_board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "peta.h"

#define TRUE 			    	1
#define FALSE 			        0
#define RS232_MAX_PACKET	    10
#define RS485_MAX_PACKET	    22

#define countof(a)   (sizeof(a) / sizeof(*(a)))
    
#define DEBUG               0


// Global Variables
// RS485
static uint8_t rs485_count=0;
static uint8_t RS485_Msg[RS485_MAX_PACKET];
uint8_t RS485_RxBuffer[RS485_MAX_PACKET];
uint8_t response_remote_status = 0;
static uint8_t at_rsp[28] = {0x41, 0x54, 0x2A, 0x52, 0x58, 0x5F, 0x52, 0x53, 0x50, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A};
// RS232
static uint8_t rs232_count=0;
uint8_t RS232_RxBuffer[RS232_MAX_PACKET];
static uint8_t ELB_Status = 0;
static uint32_t ELB_count = 0;     
static uint32_t before_ELB_count = 0;                            
                            
//ADC
extern uint16_t ADCConvertedValue[2];
static uint16_t adc_current = 0;
static uint16_t adc_voltage = 0;
static int32_t debug_current = 0;
static int32_t debug_voltage = 0;


// Begin main.c
// General Function
uint8_t CheckSum(int len, uint8_t* msg)
{
    uint8_t cs = 0x00;
    uint8_t i = 0;
    
    for (i=0; i< len; i++, msg++) 
    {
        cs += *msg;        
    }
    cs = cs & 0xFF;
    return cs;
}

int Status_Compare(uint8_t a[], uint8_t b[], int size) 
{
    int i;
    for (i=0; i<size; i++) {
        if (a[i] != b[i]) {
            return FALSE;
        }
        else{
            continue;
        }
    } 
    return TRUE;    
}


void RS485RxTask_Function(void *pvParameters)
{	
	NVIC_Configuration();
    while(1){}
}

/*
41 54 2A 54 58 5F 52 45 51 30 30 30 30 30 30 30 01 01 01 2B 0D 0A
41 54 2A 54 58 5F 52 45 51 30 30 30 30 30 30 30 01 01 02 2B 0D 0A
41 54 2A 54 58 5F 52 45 51 30 30 30 30 30 30 30 01 01 03 2B 0D 0A
41 54 2A 54 58 5F 52 45 51 30 30 30 30 30 30 30 01 01 03 2B 0D 0A
*/
void RS485Task_Function(void *pvParameters)
{	
    uint16_t len;
    // const uint8_t* at_req = "AT*TX_REQ"; 
    const char* at_req = "AT*TX_REQ"; 
    
    
    uint8_t ELB_Air_On[8] =  {0xA1, 0x80, 0x03, 0xC1, 0x30, 0x00, 0xF1, 0xDA };
    uint8_t ELB_Air_Off[8] = {0xA1, 0x80, 0x03, 0xC2, 0x31, 0x00, 0xF3, 0xDA };    
    uint8_t voltage[2];
    uint8_t current[2];
    
    while (1) {
        
        if ((RS485_RxBuffer[rs485_count-1] == 0x0D)  || (rs485_count >= RS485_MAX_PACKET)) {
            
#if DEBUG
            SendData(USART1, RS485_RxBuffer, rs485_count-1);
#endif
            // Limition of packet length
            len = (rs485_count >= RS485_MAX_PACKET) ? RS485_MAX_PACKET : rs485_count;        
            // RS485_Msg <- Buffer
            memcpy(RS485_Msg, RS485_RxBuffer, RS485_MAX_PACKET);                      
            if((strstr((char*) RS485_Msg, at_req)) && (strstr((char*) RS485_Msg, "\r\n")) && (len == 22)) {
                at_rsp[16] = 0x15;  // Main ID : 0x15 = 21              
                at_rsp[17] = 0x02;  // ELB Type
                at_rsp[18] = 0x01;  //Number of ELB                
                at_rsp[19] = 0x01;  // Sub ID
                memcpy(voltage, &adc_voltage, 2);	//ADCConvertedValue[0] : Voltage
                at_rsp[20] = voltage[1];
                at_rsp[21] = voltage[0];            
                memcpy(current, &adc_current, 2);	//ADCConvertedValue[1] : C
                at_rsp[22] = current[1];
                at_rsp[23] = current[0];                
                // Send
                if (RS485_Msg[18] == 0x01){
                    // debug 
                    debug_current = 0;
                    debug_voltage = 0;
                    response_remote_status = 0;
                    at_rsp[24] = ELB_Status;
                    at_rsp[25] = CheckSum(25, at_rsp);
                    GPIOB->BSRE ^= GPIO_Pins_12;
                    SendData(USART2, at_rsp, 28);
                    GPIOB->BRE ^= GPIO_Pins_12;
                }
                // ELB ON
                else if (RS485_Msg[18] == 0x02){
                    debug_current = 0;
                    debug_voltage = 0;
                    response_remote_status = 0;
                    at_rsp[24] = ELB_Status;
                    at_rsp[25] = CheckSum(25, at_rsp);
                    GPIOB->BSRE ^= GPIO_Pins_12;
                    SendData(USART2, at_rsp, 28);
                    GPIOB->BRE ^= GPIO_Pins_12;
                    SendData(USART3, ELB_Air_On, 8);                    
                }                
                // ELB OFF
                else if (RS485_Msg[18] == 0x03) {
                    response_remote_status = 3;
                    SendData(USART3, ELB_Air_Off, 8);                
                }
                // DEBUG 
                else if (RS485_Msg[18] == 0x11) {
                    USART_SendData(USART1, (uint16_t) 0xCC);
                    debug_voltage = 1;
                }
                else if (RS485_Msg[18] == 0x12) {
                    USART_SendData(USART1, (uint16_t) 0xDD);
                    debug_current = 1;
                }
            } // if
            // Initialization of RxBuffer
            memset(&RS485_RxBuffer[0], 0, RS485_MAX_PACKET);
            rs485_count = 0;
        }
        vTaskDelay(50);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_INT_RDNE) != RESET) {
        USART_ClearITPendingBit(USART2, USART_INT_RDNE);
        RS485_RxBuffer[rs485_count++] = USART_ReceiveData(USART2);
    }
    if(USART_GetITStatus(USART2, USART_INT_TDE) != RESET) {   
        USART_INTConfig(USART2, USART_INT_TDE, DISABLE);
    }
}

void RS232RxTask_Function(void *pvParameters)
{	
	NVIC_Configuration();
    while(1){}
}

void RS232Task_Function(void *pvParameters)
{	
    // UP                       1 << 0 LED 0
    uint8_t status0[10] = {0xA0, 0x81, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xDA};
    // DOWN : MANUAL            1 << 1 LED 1
    uint8_t status1[10] = {0xA0, 0x81, 0x05, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0xDA};
    // DOWN : REMOTE            1 << 2 LED 1 BLINK
    uint8_t status2[10] = {0xA0, 0x81, 0x05, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0xDA};
    // DOWN : LEAK INSPECTION   1 << 3 LED ALL BLINK 
    uint8_t status3[10] = {0xA0, 0x81, 0x05, 0x02, 0x02, 0x0A, 0x01, 0x01, 0x0A, 0xDA};
    // STATUS : LEAK            1 << 4 LED 2
    uint8_t status4[8] = {0xA0, 0x80, 0x03, 0xC1, 0x30, 0x32, 0xC3, 0xDA};
    // STATUS : LEAK            1 << 5 LED 3
    uint8_t status5[8] = {0xA0, 0x80, 0x03, 0xC1, 0x30, 0x33, 0xC2, 0xDA};
    
    while (1) {
        if ((RS232_RxBuffer[rs232_count-1] == 0xDA) || (rs232_count >= RS232_MAX_PACKET)) {
            // ELB Count
            ELB_count++;

#if DEBUG
            SendData(USART1, RS232_RxBuffer, rs232_count - 1);
#endif          
            if (rs232_count <= RS232_MAX_PACKET) {
                if (Status_Compare(RS232_RxBuffer, status0, rs232_count)) {
                    ELB_Status = 0x01;                        
                    GPIOB->BSRE = GPIO_Pins_5;  // LED2 (off)
                    GPIOB->BSRE = GPIO_Pins_6;  // LED3 (off)
                    GPIOB->BSRE = GPIO_Pins_7;  // LED4 (off)
                }
                else if (Status_Compare(RS232_RxBuffer, status1, rs232_count)) {                        
                    ELB_Status = 1<<1;
                    GPIOB->BRE = GPIO_Pins_5;   // LED2 (on)
                    GPIOB->BSRE = GPIO_Pins_6;  // LED3 (off)
                    GPIOB->BSRE = GPIO_Pins_7;  // LED4 (off)
                }
                else if (Status_Compare(RS232_RxBuffer, status2, rs232_count)) {    
                    ELB_Status = 1<<2;
                    GPIOB->BSRE = GPIO_Pins_5;  // LED2 (off)
                    GPIOB->BSRE = GPIO_Pins_6;  // LED3 (off)
                    GPIOB->BSRE = GPIO_Pins_7;  // LED4 (off)
                }
                else if (Status_Compare(RS232_RxBuffer, status3, rs232_count)) {    
                    ELB_Status = 1<<3;
                    GPIOB->BSRE = GPIO_Pins_5;  // LED2 (off)
                    GPIOB->BSRE = GPIO_Pins_6;  // LED3 (off)
                    GPIOB->BSRE = GPIO_Pins_7;  // LED4 (off)
                }
                else if (Status_Compare(RS232_RxBuffer, status4, rs232_count)) {    
                    ELB_Status = 1<<4;
                    GPIOB->BSRE = GPIO_Pins_5;  // LED2 (off)
                    GPIOB->BRE = GPIO_Pins_6;   // LED3 (on)
                    GPIOB->BSRE = GPIO_Pins_7;  // LED4 (off)
                }
                else if (Status_Compare(RS232_RxBuffer, status5, rs232_count)) {    
                    ELB_Status = 1<<5;
                    GPIOB->BSRE = GPIO_Pins_5;  // LED2 (off)
                    GPIOB->BSRE = GPIO_Pins_6;  // LED3 (off)
                    GPIOB->BRE = GPIO_Pins_7;   // LED4 (on)
                }
                // Response remote status
                /*
                if ((response_remote_status == 2) && (ELB_Status == 1)) {
                    response_remote_status = 0;
                    at_rsp[24] = 1;
                    at_rsp[25] = CheckSum(25, at_rsp);
                    GPIOB->BSRE ^= GPIO_Pins_12;
                    SendData(USART2, at_rsp, 28);
                    GPIOB->BRE ^= GPIO_Pins_12;
                }
                */
                
                // After status On -> OFF THEN Send response msg
                if ((response_remote_status == 3) && (ELB_Status == 4)) {
                    response_remote_status = 0;
                    at_rsp[24] = 4;
                    at_rsp[25] = CheckSum(25, at_rsp);
                    GPIOB->BSRE ^= GPIO_Pins_12;
                    SendData(USART2, at_rsp, 28);
                    GPIOB->BRE ^= GPIO_Pins_12;
                }                
            }
            // Initialization of RxBuffer            
            rs232_count = 0;
            memset(&RS232_RxBuffer[0], 0, RS232_MAX_PACKET);
        }
        vTaskDelay(50);
    }
}

// RS232 Hanlder
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_INT_RDNE) != RESET) {
        USART_ClearITPendingBit(USART3, USART_INT_RDNE);
        RS232_RxBuffer[rs232_count++] = USART_ReceiveData(USART3);
    }
    if(USART_GetITStatus(USART3, USART_INT_TDE) != RESET) {   
        USART_INTConfig(USART3, USART_INT_TDE, DISABLE);
    }
}

// Status Function
void Status_Function(void *pvParameters)
{	
    while(1)
    {
        if (before_ELB_count == ELB_count) {        
            GPIOB->BRE = GPIO_Pins_5;  // LED2 (on)
            GPIOB->BRE = GPIO_Pins_6;  // LED3 (on)
            GPIOB->BRE = GPIO_Pins_7; // LED4 (on)
            ELB_Status = 1<<6;
        } 
        else {
            before_ELB_count = ELB_count;
        }        
        vTaskDelay(5000);
    }
}

// Voltage Task
void Voltage_Function(void *pvParameters)
{	
    uint16_t min_voltage = 4096;
    uint16_t max_voltage = 0;
    uint16_t now_voltage = 0;    
    int32_t i;
    uint8_t send_voltage[9] = {0xAA, 0x00, 0x00, 0xBB, 0x00, 0x00, 0xCC, 0x00, 0x00};
    uint8_t p_max_voltage[2];
    uint8_t p_min_voltage[2];
    uint8_t p_voltage[2];
    float k1 = 110.0f/560.0f;
    float k2 = 639.12f;
    
    while(1)
    {
        min_voltage = 4096;
        max_voltage = 0;        

        for (i=0; i<120; i++) {
            now_voltage = ADCConvertedValue[0];	//ADCConvertedValue[0] : Voltage
            if (now_voltage >= max_voltage)
                max_voltage = now_voltage;
            if (now_voltage <= min_voltage)
                min_voltage = now_voltage;
            vTaskDelay(1000/60);
        }        
        //Update global variable
        adc_voltage = (uint16_t) (k2 - k1 * min_voltage);        
        if (debug_voltage) {
            memcpy(p_min_voltage, &min_voltage, 2);
            memcpy(p_max_voltage, &max_voltage, 2);
            memcpy(p_voltage, &adc_voltage, 2);
            send_voltage[1] = p_min_voltage[1];
            send_voltage[2] = p_min_voltage[0];
            send_voltage[4] = p_max_voltage[1];
            send_voltage[5] = p_max_voltage[0];
            send_voltage[7] = p_voltage[1];
            send_voltage[8] = p_voltage[0];            
            SendData(USART1, send_voltage, 9);
        }
        vTaskDelay(50);        
    }
}

// Current Task
void Current_Function(void *pvParameters)
{	
    uint16_t min_current = 4096;
    uint16_t max_current = 0;
    uint16_t now_current = 0;
    uint8_t send_current[9] = {0xDD, 0x00, 0x00, 0xEE, 0x00, 0x00, 0xFF, 0x00, 0x00};
    uint8_t p_max_current[2];
    uint8_t p_min_current[2];
    uint8_t p_current[2];
    int32_t i;    
    float k1 = 0.058743f;
    float k2 = -36.82923f;
    float temp_current;
    
    while(1)
    {
        min_current = 4096;
        max_current = 0;        

        for (i=0; i<120; i++) {
            now_current = ADCConvertedValue[1];	//ADCConvertedValue[1] : Current
            if (now_current >= max_current)
                max_current = now_current;
            if (now_current <= min_current)
                min_current = now_current;            
            vTaskDelay(1000/60);
        }
        //adc_current = (uint16_t) ((max_current - min_current) * 0.707);
        //adc_current = (uint16_t) k1 * max_current + k2;
        temp_current =  (k1 * max_current + k2);
        adc_current = (uint16_t) (temp_current);
        if (debug_current) {
            memcpy(p_min_current, &min_current, 2);
            memcpy(p_max_current, &max_current, 2);
            memcpy(p_current, &adc_current, 2);
            send_current[1] = p_min_current[1];
            send_current[2] = p_min_current[0];
            send_current[4] = p_max_current[1];
            send_current[5] = p_max_current[0];
            send_current[7] = p_current[1];
            send_current[8] = p_current[0];
            SendData(USART1, send_current, 9);            
        }
        vTaskDelay(50);
    }
}

int main(void)
{
    // Configurations
	RCC_Configuration();
	GPIO_Configuration();
	AT32_Board_Init();
	UART_Configuration();
	ADC_Configuration();
    
    // Led Init
    GPIOF->BRE = GPIO_Pins_6; // LED1 (on)	
	GPIOB->BSRE = GPIO_Pins_5;// LED2 (off)
	GPIOB->BSRE = GPIO_Pins_6;// LED3 (off)
	GPIOB->BSRE = GPIO_Pins_7;// LED4 (off)
    
    //Tasks
	taskENTER_CRITICAL(); //Enter critical
	xTaskCreate(RS485RxTask_Function, "RS485RxTask", 128, NULL, 1, NULL);
	xTaskCreate(RS232RxTask_Function, "RS232RxTask", 128, NULL, 2, NULL);

    xTaskCreate(RS232Task_Function, "RS232Task", 128, NULL, 3, NULL);
    xTaskCreate(RS485Task_Function, "RS485Task", 128, NULL, 4, NULL);

    xTaskCreate(Status_Function, "StatusTask", 128, NULL, 5, NULL);

    xTaskCreate(Voltage_Function, "VoltageTask", 128, NULL, 6, NULL);
    xTaskCreate(Current_Function, "CurrentTask", 128, NULL, 7, NULL);

	taskEXIT_CRITICAL();  //Exit critical    
	vTaskStartScheduler(); //Start Scheduler
}


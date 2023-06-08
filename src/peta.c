#include "at32f4xx.h"

#define MAX_PACKET	        22

void SendData(USART_Type* USARTx, uint8_t TxMsg[], int32_t len)
{
    uint8_t idx = 0;
    
    while (idx <= len)
	{
        USART_SendData(USARTx, TxMsg[idx++]);

        while(USART_GetFlagStatus(USARTx, USART_FLAG_TDE) == RESET)
        {
        }
    }
}

void RCC_Configuration(void)
{ 
	/* GPIOA (USART1, RS485=USART2) GPIOB (LED) GPIOC (ADC, RS232(USART3orUART4), PB12*/    
    RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA | RCC_APB2PERIPH_GPIOC | RCC_APB2PERIPH_AFIO |  RCC_APB2PERIPH_GPIOB, ENABLE);
  
    //RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA | RCC_APB2PERIPH_GPIOC | RCC_APB2PERIPH_AFIO, ENABLE);
	/* Enable DMA1 & ADC1 clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_APB2PERIPH_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_ADC1, ENABLE);
	
    RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_USART2, ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_USART3, ENABLE);  
}

void GPIO_Configuration(void)
{
    GPIO_InitType GPIO_InitStructure;
  
	
    /* ADC1 11 PC1 220VS  */            /* ADC1 12 PC2 CTS2 */
    GPIO_InitStructure.GPIO_Pins = (GPIO_Pins_1 | GPIO_Pins_2) ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_ANALOG;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
    // USART1 Rx
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // USART1 Tx
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_9;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Enable the USART3 Pins Software Remapping */
    GPIO_PinsRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

    /* Configure USART2 Rx as input floating */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  

    /* Configure USART2 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_2;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART3 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_10;
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    
    /*  Configure GPIO pin : PB12  */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_12;
		GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
    /*  Configure GPIO pin : (LED PF6)  */
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_6 ;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    /*  Configure GPIO pin : (LED PB5 6 7 )  */
    GPIO_InitStructure.GPIO_Pins = (GPIO_Pins_5 | GPIO_Pins_6 | GPIO_Pins_7);
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
}

void UART_Configuration(void)
{
    USART_InitType USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Init(USART2, &USART_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    USART_INTConfig(USART2, USART_INT_RDNE, ENABLE);
    USART_INTConfig(USART2, USART_INT_TDE, ENABLE);
    USART_INTConfig(USART3, USART_INT_RDNE, ENABLE);
    USART_INTConfig(USART3, USART_INT_TDE, ENABLE);

    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

volatile uint16_t ADCConvertedValue[2];

void ADC_Configuration(void)
{
	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_Reset(DMA1_Channel1);
	DMA_DefaultInitParaConfig(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr    = (uint32_t)&ADC1->RDOR;	//adc1 data register
	DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)&ADCConvertedValue;	//ADC1 value
	DMA_InitStructure.DMA_Direction             = DMA_DIR_PERIPHERALSRC;
	DMA_InitStructure.DMA_BufferSize            = 2;
	DMA_InitStructure.DMA_PeripheralInc         = DMA_PERIPHERALINC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc             = DMA_MEMORYINC_ENABLE;		//multi
	DMA_InitStructure.DMA_PeripheralDataWidth   = DMA_PERIPHERALDATAWIDTH_HALFWORD;	//read from 16bit (use 12bit)
	DMA_InitStructure.DMA_MemoryDataWidth       = DMA_MEMORYDATAWIDTH_HALFWORD; // save to 16bit (use 12bit)
	DMA_InitStructure.DMA_Mode                  = DMA_MODE_CIRCULAR;
	DMA_InitStructure.DMA_Priority              = DMA_PRIORITY_HIGH;
	DMA_InitStructure.DMA_MTOM                  = DMA_MEMTOMEM_DISABLE;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  
	/* Enable DMA1 channel1 */
	DMA_ChannelEnable(DMA1_Channel1, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode              = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanMode          = ENABLE;
	ADC_InitStructure.ADC_ContinuousMode    = ENABLE;
	ADC_InitStructure.ADC_ExternalTrig      = ADC_ExternalTrig_None;
	ADC_InitStructure.ADC_DataAlign         = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NumOfChannel      = 2;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channels configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239_5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239_5);  
	/* Enable ADC1 DMA */
	ADC_DMACtrl(ADC1, ENABLE);		
	
	/* Enable ADC1 */
	ADC_Ctrl(ADC1, ENABLE);

	/* Enable ADC1 reset calibration register */   
	ADC_RstCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCtrl(ADC1, ENABLE);
}

void NVIC_Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
   /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

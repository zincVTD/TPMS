#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_i2c.h"              // Keil::Device:StdPeriph Drivers:I2C
#include "stm32f10x_can.h"              // Keil::Device:StdPeriph Drivers:CAN
#include "OLED_LCD_SSD1306.h"
#include "timer.h"

#define TPMS_DATA_ID 0x1AD
uint8_t TPMSData[8] = {0};
uint8_t pressureData[4] = {0};
uint8_t batteryData[4] = {0};

void intToStr(int value, char* str) {
	char temp[12];
	int i = 0, j = 0;
	if (value < 0) {
		str[j++] = '-';
		value = -value;
	}
	do {
		temp[i++] = (value % 10) + '0';
		value /= 10;
	} while (value > 0);
	while (i > 0) {
		str[j++] = temp[--i];
	}
	str[j] = '\0';
}

void RCC_Config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_I2C1, ENABLE);
}

void GPIO_Config(){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

uint8_t OLED_Config(SSD1306_Name* mySSD1306, I2C_TypeDef* I2C) {
	I2C_InitTypeDef I2C_InitStructure;

	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(I2C, &I2C_InitStructure);
	I2C_Cmd(I2C, ENABLE);
	
	return SSD1306_Init(mySSD1306, I2C);
}

void CAN_Config(){
	CAN_InitTypeDef CAN_InitStructure;

	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = ENABLE;
	CAN_InitStructure.CAN_AWUM = ENABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 6;
	
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	while(CAN_Init(CAN1, &CAN_InitStructure) == CAN_InitStatus_Failed);
}

void CAN_FilterConfig() {
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x1AD << 5;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFE0;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

	CAN_FilterInit(&CAN_FilterInitStructure);
}

void CAN_ReceiveData(uint32_t id, uint8_t* data) {
	CanRxMsg RxMessage;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	if(RxMessage.StdId == id){
		for (int i = 0; i < RxMessage.DLC; i++) {
			data[i] = RxMessage.Data[i];
		}
	}
}

void USB_LP_CAN1_RX0_IRQHandler(){
	if(CAN_GetITStatus(CAN1, CAN_IT_FMP0)){
		CAN_ReceiveData(TPMS_DATA_ID, TPMSData);
		for(int i = 0; i < 4; i++){
			pressureData[i] = TPMSData[i + 1];
		}
		batteryData[0] = (TPMSData[5] & 0xF0) >> 4;
		batteryData[1] = TPMSData[5] & 0x0F;
		batteryData[2] = (TPMSData[6] & 0xF0) >> 4;
		batteryData[3] = TPMSData[6] & 0x0F;
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
	}
}

void OLED_DisplayTPMS(SSD1306_Name* SSD1306, uint8_t pressures[4], uint8_t batteryLevels[4]) {
	// Clear the display
	SSD1306_Clear(SSD1306);

	// Display each tire data
	for (int i = 0; i < 4; i++) {
		int x = (i % 2 == 0) ? 0 : 64;  // Left or Right
		int y = (i < 2) ? 0 : 32;       // Top or Bottom

		// Display tire pressure
		SSD1306_GotoXY(SSD1306, x, y);
		const char* label = (i == 0) ? "FL" : (i == 1) ? "FR" : (i == 2) ? "RL" : "RR";
		char buffer[32];
		int len = 0;

		// Add label to buffer
		while (label[len] != '\0') {
			buffer[len] = label[len];
			len++;
		}

		// Add ": "
		buffer[len++] = ':';
		buffer[len++] = ' ';

		// Add pressure value in psi
		char temp[12];
		intToStr(pressures[i], temp);
		int j = 0;
		while (temp[j] != '\0') {
			buffer[len++] = temp[j++];
		}
		buffer[len] = '\0';

		SSD1306_Puts(SSD1306, buffer, &Font_7x10, SSD1306_COLOR_WHITE);

		// Display battery level
		SSD1306_GotoXY(SSD1306, x, y + 16);
		len = 0;
		int batteryPercent = (batteryLevels[i] * 100) / 16; // Convert to percentage
		intToStr(batteryPercent, buffer);
		while (buffer[len] != '\0') {
			len++;
		}
		buffer[len++] = '%';
		buffer[len] = '\0';

		SSD1306_Puts(SSD1306, buffer, &Font_7x10, SSD1306_COLOR_WHITE);
	}

	// Update the display
	SSD1306_UpdateScreen(SSD1306);
}

int main(){
	SSD1306_Name mySSD;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	RCC_Config();
	GPIO_Config();
	TIM2_Config();
	while(OLED_Config(&mySSD, I2C1) == 0);
	CAN_Config();
	CAN_FilterConfig();
	
	while(1){
		OLED_DisplayTPMS(&mySSD, pressureData, batteryData);
		delay(10);
	}
}

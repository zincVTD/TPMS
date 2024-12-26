#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_spi.h"              // Keil::Device:StdPeriph Drivers:SPI
#include "stm32f10x_can.h"              // Keil::Device:StdPeriph Drivers:CAN
#include "timer.h"

#define MIN_PRESSURE 5
#define MAX_PRESSURE 40

uint8_t sensorData[6] = {0};
uint8_t CANTransmitData[8] = {0};

void ConvertSensorDataToTPMSData(uint8_t* sensorData, uint8_t* TPMSData) {
	uint8_t count = 0;
	for (int i = 0; i < 8; i++) {
		TPMSData[i] = 0x00;
	}

	for(int i = 0; i < 6; i++){
		if(sensorData[i] != 0x00){
			count++;
		}
	}
	
	if(count != 0){
		TPMSData[0] |= (1 << 7);
		for (int i = 0; i < 4; i++) {
			if (sensorData[i] == 0xFF) {
				TPMSData[0] &= ~(1 << i);
			} 
			else {
				TPMSData[0] |= (1 << i);
			}
		}
	}
	else{
		TPMSData[0] = 0b01111111;
		for(int i = 1; i < 9; i++){
			TPMSData[i] = 0xFF;
			return;
		}
	}
	
	for (int i = 0; i < 4; i++) {
		TPMSData[i + 1] = (sensorData[i] == 0xFF) ? 0xFF : sensorData[i];
	}

	TPMSData[5] = sensorData[4];
//	TPMSData[6] = sensorData[5];
	TPMSData[6] = 0x00;

	for (int i = 0; i < 4; i++) {
		if (sensorData[i] != 0xFF) {
			if (sensorData[i] > 36) {
				TPMSData[7] |= (1 << (4 + i));
			}
			if (sensorData[i] < 28) {
				TPMSData[7] |= (1 << i);
			}
		}
	}
}

void RCC_Config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
}

void GPIO_Config(){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
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

	while(CAN_Init(CAN1, &CAN_InitStructure) == CAN_InitStatus_Failed);
}

void SPI_Config(){
	SPI_InitTypeDef SPI_InitStruct;
	
	SPI_InitStruct.SPI_Mode = SPI_Mode_Slave;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	
	SPI_Init(SPI1, &SPI_InitStruct);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}

void CAN_TransmitData(uint8_t* data, uint8_t length){
	CanTxMsg TxMessage;

	TxMessage.StdId = 0x1AD;
	TxMessage.RTR = CAN_RTR_Data;
	TxMessage.IDE = CAN_Id_Standard;
	TxMessage.DLC = length;

	for (int i = 0; i < length; i++) {
		TxMessage.Data[i] = data[i];
	}
	
	uint8_t mailbox = CAN_Transmit(CAN1, &TxMessage);
	while (CAN_TransmitStatus(CAN1, mailbox) != CAN_TxStatus_Ok);
}

void SPI1_IRQHandler(void){
	if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE) == SET){
		static int i = 0;
		sensorData[i++] = (uint8_t)SPI_I2S_ReceiveData(SPI1);
		if(i == 6){
			ConvertSensorDataToTPMSData(sensorData, CANTransmitData);
			for(int j = 0; j < 7; j++){
				sensorData[j] = 0x00;
			}
			i = 0;
		}
		SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_RXNE);
	}
}

int main(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	RCC_Config();
	GPIO_Config();
	TIM2_Config();
	SPI_Config();
	CAN_Config();
	while(1){
		CAN_TransmitData(CANTransmitData, 8);
		delay(10);
	}
}

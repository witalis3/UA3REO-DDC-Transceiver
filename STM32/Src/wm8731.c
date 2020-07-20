#include "stm32h7xx_hal.h"
#include "wm8731.h"
#include "trx_manager.h"
#include "wire.h"
#include "lcd.h"
#include "agc.h"
#include "usbd_audio_if.h"

//Public variables
uint32_t WM8731_DMA_samples = 0;									//считаем количество семплов, переданных аудио-кодеку
bool WM8731_DMA_state = true;										//с какой частью буфера сейчас работаем, true - compleate; false - half
bool WM8731_Buffer_underrun = false;								//недостаток данных в буфере из аудио-процессора
IRAM2 int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE] = {0}; //кольцевые буфферы аудио-кодека
IRAM2 int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE] = {0};

//Private variables
static bool WM8731_Beeping = false; //в данный момент идёт сигнал бипера

//Prototypes
static uint8_t WM8731_SendI2CCommand(uint8_t reg, uint8_t value);																		  //отправить I2C команду в кодек
static HAL_StatusTypeDef HAL_I2S_TXRX_DMA(I2S_HandleTypeDef *hi2s, uint16_t *txData, uint16_t *rxData, uint16_t txSize, uint16_t rxSize); //Full-duplex реализация запуска I2S
static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma);																						  //RX Буффер полностью отправлен в кодек
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma);																					  //RX Буффер на половину отправлен в кодек
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma);																						  //TX Буффер полностью принят из кодека
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma);																					  //TX Буффер на половину принят из кодека
static void I2S_DMAError(DMA_HandleTypeDef *hdma);																						  //Ошибка DMA I2S

//запуск шины I2S
void WM8731_start_i2s_and_dma(void)
{
	WM8731_CleanBuffer();
	if (HAL_I2S_GetState(&hi2s3) == HAL_I2S_STATE_READY)
		HAL_I2S_TXRX_DMA(&hi2s3, (uint16_t *)&CODEC_Audio_Buffer_RX[0], (uint16_t *)&CODEC_Audio_Buffer_TX[0], CODEC_AUDIO_BUFFER_SIZE * 2, CODEC_AUDIO_BUFFER_SIZE); // 32bit rx spi, 16bit tx spi
}

//очистка буффера аудио-кодека и USB аудио
void WM8731_CleanBuffer(void)
{
	memset(CODEC_Audio_Buffer_RX, 0x00, sizeof CODEC_Audio_Buffer_RX);
	memset(CODEC_Audio_Buffer_TX, 0x00, sizeof CODEC_Audio_Buffer_TX);
	SCB_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_RX[0], sizeof(CODEC_Audio_Buffer_RX));
	SCB_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_TX[0], sizeof(CODEC_Audio_Buffer_TX));
	memset(USB_AUDIO_rx_buffer_a, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	memset(USB_AUDIO_rx_buffer_b, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	memset(USB_AUDIO_tx_buffer, 0x00, sizeof USB_AUDIO_tx_buffer);
	ResetAGC();
}

//сигнал бипера
void WM8731_Beep(void)
{
	WM8731_Beeping = true;
	for (uint16_t i = 0; i < CODEC_AUDIO_BUFFER_SIZE; i++)
		CODEC_Audio_Buffer_RX[i] = (int32_t)(((float32_t)TRX_Volume / 100.0f) * 2000.0f * arm_sin_f32(((float32_t)i / (float32_t)TRX_SAMPLERATE) * PI * 2.0f * 500.0f));
	HAL_Delay(50);
	WM8731_Beeping = false;
}

//отправить I2C команду в кодек
static uint8_t WM8731_SendI2CCommand(uint8_t reg, uint8_t value)
{
	uint8_t st = 2;
	uint8_t repeats = 0;
	while (st != 0 && repeats < 3)
	{
		i2c_begin();
		i2c_beginTransmission_u8(B8(0011010)); //I2C_ADDRESS_WM8731 00110100
		i2c_write_u8(reg);					   // MSB
		i2c_write_u8(value);				   // MSB
		st = i2c_endTransmission();
		if (st != 0)
			repeats++;
		HAL_Delay(1);
	}
	return st;
}

//переход в режим TX (глушим динамик и пр.)
void WM8731_TX_mode(void)
{
	WM8731_SendI2CCommand(B8(00000100), B8(00000000)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000110), B8(00000000)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001010), B8(00011110)); //R5 Digital Audio Path Control
	if (TRX.InputType_LINE)							   //line
	{
		WM8731_SendI2CCommand(B8(00000000), B8(00010111)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000010), B8(00010111)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00000010)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01101010)); //R6 Power Down Control
	}
	if (TRX.InputType_MIC) //mic
	{
		WM8731_SendI2CCommand(B8(00000001), B8(10000000)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000011), B8(10000000)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00000101)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01101001)); //R6 Power Down Control
	}
}

//переход в режим RX (глушим микрофон и пр.)
void WM8731_RX_mode(void)
{
	WM8731_SendI2CCommand(B8(00000000), B8(10000000)); //R0 Left Line In
	WM8731_SendI2CCommand(B8(00000010), B8(10000000)); //R1 Right Line In
	WM8731_SendI2CCommand(B8(00000100), B8(01111111)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000110), B8(01111111)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001000), B8(00010110)); //R4 Analogue Audio Path Control
	WM8731_SendI2CCommand(B8(00001010), B8(00010110)); //R5 Digital Audio Path Control
	WM8731_SendI2CCommand(B8(00001100), B8(01100111)); //R6 Power Down Control
}

//переход в смешанный режим RX-TX (для LOOP)
void WM8731_TXRX_mode(void) //loopback
{
	WM8731_SendI2CCommand(B8(00000100), B8(01111111)); //R2 Left Headphone Out
	WM8731_SendI2CCommand(B8(00000110), B8(01111111)); //R3 Right Headphone Out
	WM8731_SendI2CCommand(B8(00001010), B8(00010110)); //R5 Digital Audio Path Control
	if (TRX.InputType_LINE)							   //line
	{
		WM8731_SendI2CCommand(B8(00000000), B8(00010111)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000010), B8(00010111)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00010010)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01100010)); //R6 Power Down Control, internal crystal
	}
	if (TRX.InputType_MIC) //mic
	{
		WM8731_SendI2CCommand(B8(00000001), B8(10000000)); //R0 Left Line In
		WM8731_SendI2CCommand(B8(00000011), B8(10000000)); //R1 Right Line In
		WM8731_SendI2CCommand(B8(00001000), B8(00010101)); //R4 Analogue Audio Path Control
		WM8731_SendI2CCommand(B8(00001100), B8(01100001)); //R6 Power Down Control, internal crystal
	}
}

//инициализация аудио-кодека по I2C
void WM8731_Init(void)
{
	if (WM8731_SendI2CCommand(B8(00011110), B8(00000000)) != 0) //R15 Reset Chip
	{
		sendToDebug_strln("[ERR] Audio codec not found");
		LCD_showError("Audio codec init error", true);
	}
	WM8731_SendI2CCommand(B8(00001110), B8(00001110)); //R7 Digital Audio Interface Format, Codec Slave, 32bits, I2S Format, MSB-First left-1 justified
	WM8731_SendI2CCommand(B8(00010000), B8(00000000)); //R8 Sampling Control normal mode, 256fs, SR=0 (MCLK@12.288Mhz, fs=48kHz))
	WM8731_SendI2CCommand(B8(00010010), B8(00000001)); //R9 reactivate digital audio interface
	WM8731_RX_mode();
}

//RX Буффер полностью отправлен в кодек
static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma)
{
	if (((I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent)->Instance == SPI3)
	{
		if (WM8731_Beeping)
			return;
		if (Processor_NeedRXBuffer) //если аудио-кодек не предоставил данные в буфер - поднимаем флаг ошибки
			WM8731_Buffer_underrun = true;
		WM8731_DMA_state = true;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO()->Mode == TRX_MODE_LOOPBACK)
			Processor_NeedTXBuffer = true;
		WM8731_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

//RX Буффер на половину отправлен в кодек
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
	if (((I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent)->Instance == SPI3)
	{
		if (WM8731_Beeping)
			return;
		if (Processor_NeedRXBuffer) //если аудио-кодек не предоставил данные в буфер - поднимаем флаг ошибки
			WM8731_Buffer_underrun = true;
		WM8731_DMA_state = false;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO()->Mode == TRX_MODE_LOOPBACK)
			Processor_NeedTXBuffer = true;
		WM8731_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

//TX Буффер полностью принят из кодека
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma)
{
	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
	HAL_I2S_RxCpltCallback(hi2s);
}

//TX Буффер на половину принят из кодека
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
	HAL_I2S_RxHalfCpltCallback(hi2s);
}

//Ошибка DMA I2S
static void I2S_DMAError(DMA_HandleTypeDef *hdma)
{
	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent; /* Derogation MISRAC2012-Rule-11.5 */

	/* Disable Rx and Tx DMA Request */
	CLEAR_BIT(hi2s->Instance->CFG1, (SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN));
	hi2s->TxXferCount = (uint16_t)0UL;
	hi2s->RxXferCount = (uint16_t)0UL;

	hi2s->State = HAL_I2S_STATE_READY;

	/* Set the error code and execute error callback*/
	SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);

	/* Call user error callback */
	HAL_I2S_ErrorCallback(hi2s);
}

//Full-duplex реализация запуска I2S
static HAL_StatusTypeDef HAL_I2S_TXRX_DMA(I2S_HandleTypeDef *hi2s, uint16_t *txData, uint16_t *rxData, uint16_t txSize, uint16_t rxSize)
{
	if ((rxData == NULL) || (txData == NULL) || (rxSize == 0UL) || (txSize == 0UL))
	{
		return HAL_ERROR;
	}

	/* Process Locked */
	__HAL_LOCK(hi2s);

	if (hi2s->State != HAL_I2S_STATE_READY)
	{
		__HAL_UNLOCK(hi2s);
		return HAL_BUSY;
	}

	/* Set state and reset error code */
	hi2s->State = HAL_I2S_STATE_BUSY;
	hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
	hi2s->pRxBuffPtr = rxData;
	hi2s->RxXferSize = rxSize;
	hi2s->RxXferCount = rxSize;
	hi2s->pTxBuffPtr = txData;
	hi2s->TxXferSize = txSize;
	hi2s->TxXferCount = txSize;

	hi2s->hdmarx->XferHalfCpltCallback = I2S_DMARxHalfCplt;
	hi2s->hdmarx->XferCpltCallback = I2S_DMARxCplt;
	hi2s->hdmarx->XferErrorCallback = I2S_DMAError;
	hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;
	hi2s->hdmatx->XferCpltCallback = I2S_DMATxCplt;
	hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

	/* Enable the Rx DMA Stream/Channel */
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&hi2s->Instance->RXDR, (uint32_t)hi2s->pRxBuffPtr, hi2s->RxXferSize))
	{
		// Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmatx, (uint32_t)hi2s->pTxBuffPtr, (uint32_t)&hi2s->Instance->TXDR, hi2s->TxXferSize))
	{
		//Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}

	/* Check if the I2S Rx request is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_RXDMAEN))
	{
		// Enable Rx DMA Request
		SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_RXDMAEN);
	}
	/* Check if the I2S Tx request is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN))
	{
		/* Enable Tx DMA Request */
		SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN);
	}

	/* Check if the I2S is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CR1, SPI_CR1_SPE))
	{
		/* Enable I2S peripheral */
		__HAL_I2S_ENABLE(hi2s);
	}

	/* Start the transfer */
	SET_BIT(hi2s->Instance->CR1, SPI_CR1_CSTART);

	__HAL_UNLOCK(hi2s);
	return HAL_OK;
}

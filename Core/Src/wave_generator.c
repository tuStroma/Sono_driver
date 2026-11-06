#include "wave_generator.h"
#include "pins.h"

#define CLOCK_FREQUENCY 24000000 // [Hz]
#define BIT_SHL_24 16777216		 // 1 << 24

// AD1920 registers
#define SPICONFIG 0x00
#define POWERCONFIG 0x01
#define WAV_CONFIG 0x2E
#define PAT_TYPE 0x1F
#define DDS_TW_HI 0x3F
#define DDS_TW_LO 0x3E
#define RAMUPDATE 0x1D


extern void Error_Handler(void);

// Initialization
void initialize_SPI()
{
	hspi1.Instance = SPI1;										// Choosing the SPI hardware block
	hspi1.Init.Mode = SPI_MODE_MASTER;							// STM will be the master in relation to the AD1920
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;				// Full duplex SPI bus
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;					// 8-bit data block

	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;					// CPOL - rest position at 0
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;						// CPHA - read data on rising edge

	hspi1.Init.NSS = SPI_NSS_SOFT;								// SPI control (NSS) set to "software"
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;	// Setting the communication speed
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;						// Bit ordering - MSB first

	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
	    Error_Handler();
	}
}

// AD1920 programming
void ad1920_reset()
{
	HAL_GPIO_WritePin(GPIOB, RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOB, RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
}
void ad1920_program_register(uint16_t instruction, uint16_t data)
{
	uint8_t tx_buffer[4];

	tx_buffer[0] = (instruction >> 8) & 0xFF;
	tx_buffer[1] = instruction & 0xFF;
	tx_buffer[2] = (data >> 8) & 0xFF;
	tx_buffer[3] = data & 0xFF;

	HAL_GPIO_WritePin(GPIOB, CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, tx_buffer, 4, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOB, CS_PIN, GPIO_PIN_SET);
}

void ad1920_program_wave(double frequency)
{
	ad1920_reset();

	uint32_t f_bits = calculate_frequency(frequency);
	uint16_t f_bits_hi = (f_bits >> 16) & 0xFFFF;
	uint16_t f_bits_lo = f_bits & 0xFFFF;

	// Registers
	ad1920_program_register(SPICONFIG, 0x0);
	ad1920_program_register(POWERCONFIG, 0x0);
	ad1920_program_register(WAV_CONFIG, 0x2);
	ad1920_program_register(PAT_TYPE, 0x0);
	ad1920_program_register(DDS_TW_HI, f_bits_hi);
	ad1920_program_register(DDS_TW_LO, f_bits_lo);

	// Commit configuration
	ad1920_program_register(RAMUPDATE, 0x1);
}

void ad1920_trigger()
{
	HAL_GPIO_WritePin(GPIOA, TRIGGER_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOA, TRIGGER_PIN, GPIO_PIN_RESET);
}

// Frequency
uint32_t calculate_frequency(double frequency)
{
	double f_out = (frequency / CLOCK_FREQUENCY) * BIT_SHL_24;
	return (uint32_t) f_out;
}


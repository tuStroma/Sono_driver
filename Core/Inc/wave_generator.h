#ifndef WAVE_GENERATOR_H
#define WAVE_GENERATOR_H

#include <stdint.h>

// Initialization
SPI_HandleTypeDef hspi1;
void initialize_SPI();

// AD1920 programming
void ad1920_reset();
void ad1920_program_register(uint16_t instruction, uint16_t data);
void ad1920_program_wave(double frequency);
void ad1920_trigger();

// Frequency
uint32_t calculate_frequency(double frequency);

#endif /* WAVE_GENERATOR_H */

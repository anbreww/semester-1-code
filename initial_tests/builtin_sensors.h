#ifndef BUILTIN_SENSORS_H
#define BUILTIN_SENSORS_H

uint16_t get_ambient_light(void);
uint16_t get_chip_temperature(void);
uint16_t get_scaled_vcc(void);
void adc_init(void);
void adc_temp_init(void);

#endif

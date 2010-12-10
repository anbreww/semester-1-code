#include "config.h"
#include "libs/adc_driver.h"

#include "builtin_sensors.h"



void adc_init(void)
{
	ADC_CalibrationValues_Load(&ADCA);
	ADC_ConvMode_and_Resolution_Config(&ADCA, 
		ADC_ConvMode_Unsigned, ADC_RESOLUTION_12BIT_gc);

	ADC_Prescaler_Config(&ADCA, ADC_PRESCALER_DIV16_gc);

	ADC_Reference_Config(&ADCA, ADC_REFSEL_VCC_gc);

	ADC_Ch_InputMode_and_Gain_Config(&ADCA.CH0,
	                                 ADC_CH_INPUTMODE_SINGLEENDED_gc,
                                     ADC_CH_GAIN_1X_gc);

	ADC_Ch_InputMux_Config(&ADCA.CH0, ADC_CH_MUXPOS_PIN7_gc, ADC_CH_MUXNEG_PIN0_gc);

	ADC_Enable(&ADCA);

	ADC_Wait_8MHz(&ADCA);
}

void adc_temp_init(void)
{
	// enable 1V reference and temperature modules
	ADC_BandgapReference_Enable(&ADCB);
	ADC_TempReference_Enable(&ADCB);

	// load calibration from signature bytes
	ADC_CalibrationValues_Load(&ADCB);

	// Conversion mode and resolution (12 bit right-aligned)
	ADC_ConvMode_and_Resolution_Config(&ADCB, 
		ADC_ConvMode_Unsigned, ADC_RESOLUTION_12BIT_gc);

	// prescaler from system clock (fastest)
	ADC_Prescaler_Config(&ADCB, ADC_PRESCALER_DIV4_gc);

	// internal 1V reference
	ADC_Reference_Config(&ADCB, ADC_REFSEL_INT1V_gc);

	// channel 0 for temperature
	ADC_Ch_InputMode_and_Gain_Config(&ADCB.CH0,
	                                 ADC_CH_INPUTMODE_INTERNAL_gc,
                                     ADC_CH_GAIN_1X_gc);

	ADC_Ch_InputMux_Config(&ADCB.CH0, ADC_CH_MUXINT_TEMP_gc, ADC_CH_MUXNEG_PIN0_gc);

	// channel 1 for VCC/10
	ADC_Ch_InputMode_and_Gain_Config(&ADCB.CH1,
	                                 ADC_CH_INPUTMODE_INTERNAL_gc,
                                     ADC_CH_GAIN_1X_gc);

	ADC_Ch_InputMux_Config(&ADCB.CH1, ADC_CH_MUXINT_SCALEDVCC_gc, ADC_CH_MUXNEG_PIN0_gc);

	ADC_Enable(&ADCB);

	ADC_Wait_8MHz(&ADCB);
}

uint16_t get_ambient_light(void)
{
	uint16_t adc_result = 0;
	int8_t offset = 0;
	offset = ADC_Offset_Get_Unsigned(&ADCA, &(ADCA.CH0), true);

	// enable ambient light sensor
	AMBIENT_PORT.DIRSET = AMBIENT_ENABLE_bm;
	AMBIENT_PORT.OUTSET = AMBIENT_ENABLE_bm;
	_delay_us(100);


	ADC_Ch_Conversion_Start(&ADCA.CH0);
	while(!ADC_Ch_Conversion_Complete(&ADCA.CH0));

	adc_result = ADC_ResultCh_GetWord_Unsigned(&ADCA.CH0, offset);

	// disable ambient light sensor (to save battery)
	AMBIENT_PORT.OUTCLR = AMBIENT_ENABLE_bm;

	return adc_result;
}

uint16_t get_chip_temperature(void)
{
	uint16_t adc_result = 0;
	int8_t offset = 0;
	offset = ADC_Offset_Get_Unsigned(&ADCB, &(ADCB.CH0), true);

	ADC_Ch_Conversion_Start(&ADCB.CH0);
	while(!ADC_Ch_Conversion_Complete(&ADCB.CH0));

	adc_result = ADC_ResultCh_GetWord_Unsigned(&ADCB.CH0, offset);

	return adc_result;
}

uint16_t get_scaled_vcc(void)
{
	uint16_t adc_result = 0;
	int8_t offset = 0;
	offset = ADC_Offset_Get_Unsigned(&ADCB, &(ADCB.CH1), true);

	ADC_Wait_8MHz(&ADCB);
	ADC_Ch_Conversion_Start(&ADCB.CH1);
	while(!ADC_Ch_Conversion_Complete(&ADCB.CH1));

	adc_result = ADC_ResultCh_GetWord_Unsigned(&ADCB.CH1, offset);

	return adc_result;
}

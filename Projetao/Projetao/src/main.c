#include <asf.h>
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"

#define CONF_UART              UART0
#define CONF_UART_BAUDRATE     9600
#define CONF_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
#define CONF_UART_PARITY       US_MR_PAR_NO
#define CONF_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

#define	PINO_LED_AZUL	PIO_PA19
#define PINO_LED_VERDE	PIO_PA20

#define TC			TC0
#define CHANNEL		0
#define ID_TC		ID_TC0
#define TC_Handler  TC0_Handler
#define TC_IRQn     TC0_IRQn

#define VOLT_REF        (3300)
#define TRACKING_TIME    15
#define TRANSFER_PERIOD  2
#define STARTUP_TIME	ADC_STARTUP_TIME_4
#define MAX_DIGITAL     (4095)
#define ADC_CHANNEL_LDR 5	//LDR no pino PB1
#define ADC_CHANNEL_UMIDADE	0	//Sensor de umidade do solo

#define PWM_FREQUENCY	1000	/** PWM frequency in Hz */
#define PERIOD_VALUE	4096	/** Period value of PWM output waveform */
#define INIT_DUTY_VALUE	4000	/** Initial duty cycle value */
#define PWM_CHANNEL_BOMBA	2	//Canal PWM da bomba
#define PWM_CHANNEL_BOMBA_EN	PWM_ENA_CHID2	//Enable do canal. Ultimo digito = canal
#define PIN_BOMBA	PIO_PDR_P13	//Pino da bomba
#define PIN_BOMBA_ABCDSR	PIO_ABCDSR_P13
#define PIO_BOMBA	PIOA

#define PIO_HUM	PIOA
#define PINO_HUM	PIO_PA14

#define TEMPO_MAX_ESCURO	10

uint32_t	escuro_max = TEMPO_MAX_ESCURO;
uint32_t	escuro = TEMPO_MAX_ESCURO;
uint32_t	luz_min = 50;	//Luz minima em %
uint32_t	duty_cycle = INIT_DUTY_VALUE;
uint32_t	max_aceso = 10;	//Tempo maximo com a luz acesa

void inicializacao_UART (){
	
	static usart_serial_options_t usart_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS
	};
	usart_serial_init(CONF_UART, &usart_options);
	stdio_serial_init((Usart *)CONF_UART, &usart_options);
}

static void tc_config(uint32_t freq_desejada)
{
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t counts;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	pmc_enable_periph_clk(ID_TC);
	
	tc_find_mck_divisor( freq_desejada, ul_sysclk, &ul_div, &ul_tcclks,	BOARD_MCK);
	tc_init(TC, CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	counts = ((ul_sysclk/ul_div)/freq_desejada);
	tc_write_rc(TC, CHANNEL, counts);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 4);
	NVIC_EnableIRQ(TC_IRQn);
	tc_enable_interrupt(TC,	CHANNEL, TC_IER_CPCS);
	tc_start(TC, CHANNEL);
}

void TC_Handler(void)
{
	tc_get_status(TC,CHANNEL);
	LED_Toggle(LED0_GPIO);
	adc_start(ADC);
	//if (duty_cycle == INIT_DUTY_VALUE)
		//duty_cycle = 1000;
	//else
		//duty_cycle = INIT_DUTY_VALUE;
	//PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CDTYUPD = duty_cycle;
}

void ADC_Handler(void)
{
	uint16_t result;

	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY)
	{
		result = adc_get_channel_value(ADC, ADC_CHANNEL_LDR);
		
		//exibição do último valor
		char buffer[10];
		sprintf (buffer, "%d", result);
		puts(buffer);
		puts("\r");
		
		//lógica de tempo para acender lâmpada
		if (result <= (4095*luz_min/100))
			escuro--;
		else
			escuro = escuro_max;
		sprintf (buffer, "%d", escuro);
		puts(buffer);
		puts("\r");
		if (escuro <= -max_aceso)
			pio_set(PIOA, PINO_LED_VERDE);
		else if (escuro <= 0)
			pio_clear(PIOA, PINO_LED_VERDE);
		else
			pio_set(PIOA, PINO_LED_VERDE);
	}
}

void configuracoes_gerais()
{
	char buffer[255];
	puts("Insira tempo maximo no escuro em horas:\r");
	escuro_max = getchar();
	escuro_max *= 3600;
	puts("Insira a luminosidade minima em \%:\r");
	luz_min = getchar();
	luz_min *= 10;
	puts("Configuracao completa!\r");
	sprintf(buffer,"Tempo: %i\tLuz: %i\%\n\r", escuro, luz_min);
	puts(buffer);
}

void configure_adc(void)
{
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);
	
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, STARTUP_TIME);
	adc_configure_timing(ADC, TRACKING_TIME	, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);
	adc_enable_channel(ADC, ADC_CHANNEL_LDR);
	adc_enable_channel(ADC, ADC_CHANNEL_UMIDADE);
	NVIC_SetPriority(ADC_IRQn, 5);
	NVIC_EnableIRQ(ADC_IRQn);
	adc_enable_interrupt(ADC, ADC_IER_DRDY);
}

void configure_pwm(void)
{
	// disable the PIO (peripheral controls the pin)
	PIO_BOMBA->PIO_PDR = PIN_BOMBA;
	// select alternate function B (PWML0) for pin PA19
	PIO_BOMBA->PIO_ABCDSR[0] |= PIN_BOMBA_ABCDSR;
	PIO_BOMBA->PIO_ABCDSR[1] &= ~PIN_BOMBA_ABCDSR;
	// Enable the PWM peripheral from the Power Manger
	PMC->PMC_PCER0 = (1 << ID_PWM);
	// Select the Clock to run at the MCK (4MHz)
	PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CMR = PWM_CMR_CPRE_MCK;
	// select the period 10msec
	PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CPRD = PERIOD_VALUE;// freq em khz
	// select the duty cycle
	PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CDTY = INIT_DUTY_VALUE;
	// enable the channel
	PWM->PWM_ENA = PWM_CHANNEL_BOMBA_EN;
}

int main (void)
{
	sysclk_init();
	board_init();
	inicializacao_UART();
	configure_adc();
	configure_pwm();
	tc_config(1);
	
	pio_set_output(PIOA, PINO_LED_AZUL, HIGH, DISABLE, ENABLE);
	pio_set_output(PIOA, PINO_LED_VERDE, HIGH, DISABLE, ENABLE);

//	configuracoes_gerais();

	while(1)
	{
		
	}
}

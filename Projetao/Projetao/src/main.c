#include <asf.h>

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
#define ADC_CHANNEL 5

#define PIO_HUM	PIOA
#define PINO_HUM	PIO_PA14

#define TEMPO_MAX_ESCURO	10

int	escuro_max = TEMPO_MAX_ESCURO;
int	escuro = TEMPO_MAX_ESCURO;
int	luz_min = 50;

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

void menu()
{
	printf("\n\r");
	puts("a: Acender LED azul\r");
	puts("s: Apagar LED azul\r");
	puts("v: Acender LED verde\r");
	puts("b: Apagar LED verde\r");
}

void TC_Handler(void)
{
	tc_get_status(TC,CHANNEL);
	LED_Toggle(LED0_GPIO);
	adc_start(ADC);
}

void ADC_Handler(void)
{
	uint16_t result;

	if ((adc_get_status(ADC) & ADC_ISR_DRDY) == ADC_ISR_DRDY)
	{
		result = adc_get_latest_value(ADC);
		
		//exibição do último valor
		char buffer[10];
		sprintf (buffer, "%d", result);
		puts(buffer);
		puts("\r");
		
		//lógica de tempo para acender lâmpada
		if (result <= (4095*100/luz_min))
			escuro--;
		else
			escuro = escuro_max;
		if (escuro <= 0)
			pio_clear(PIOA, PINO_LED_VERDE);
		else
			pio_set(PIOA, PINO_LED_VERDE);
	}
}

void configuracoes_gerais()
{
	char buffer[255];
	puts("Insira tempo maximo no escuro em horas:\r");
	scanf("%i", &escuro);
	escuro *= 3600;
	puts("Insira a luminosidade minima em \%:\r");
	scanf("%i", &luz_min);
	luz_min /= 100.0;
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
	adc_enable_channel(ADC, ADC_CHANNEL);
	NVIC_SetPriority(ADC_IRQn, 5);
	NVIC_EnableIRQ(ADC_IRQn);
	adc_enable_interrupt(ADC, ADC_IER_DRDY);
}

static void hum_handle(uint32_t id, uint32_t mask)
{
	//iniciar bomba
	pio_clear(PIOA, PINO_LED_VERDE);
}

void configure_botao(void)
{
	pmc_enable_periph_clk(ID_PIOB);
	
	pio_set_input(PIO_HUM, PINO_HUM, PIN_PUSHBUTTON_1_ATTR);
	pio_set_debounce_filter(PIO_HUM, PINO_HUM, 10);
	pio_handler_set(PIO_HUM, ID_PIOA, PINO_HUM, PIN_PUSHBUTTON_1_ATTR ,hum_handle);
	pio_enable_interrupt(PIO_HUM, PINO_HUM);
	NVIC_SetPriority(PIOA_IRQn, 5);
	NVIC_EnableIRQ(PIOA_IRQn);
}

int main (void)
{
	sysclk_init();
	board_init();
	inicializacao_UART();
	configure_adc();
	tc_config(1);
	
	pio_set_output(PIOA, PINO_LED_AZUL, HIGH, DISABLE, ENABLE);
	pio_set_output(PIOA, PINO_LED_VERDE, HIGH, DISABLE, ENABLE);

	menu();
//	configuracoes_gerais();

	while(1)
	{
		
	}
}

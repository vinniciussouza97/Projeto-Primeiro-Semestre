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
}

int main (void)
{
	sysclk_init();
	board_init();
	inicializacao_UART();
	tc_config(10);
	
	pio_set_output(PIOA, PINO_LED_AZUL, HIGH, DISABLE, ENABLE);
	pio_set_output(PIOA, PINO_LED_VERDE, HIGH, DISABLE, ENABLE);

	menu();

	while(1)
	{
		
	}
}

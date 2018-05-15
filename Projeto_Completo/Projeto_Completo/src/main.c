#include <asf.h>

#define TC			TC0
#define CHANNEL		0
#define ID_TC		ID_TC0
#define TC_Handler  TC0_Handler
#define TC_IRQn     TC0_IRQn

#define CONF_UART              UART0
#define CONF_UART_BAUDRATE     9600
#define CONF_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
#define CONF_UART_PARITY       US_MR_PAR_NO
#define CONF_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

int		escuro;
float	luz_min;

static void tc_config(uint32_t freq_desejada)
{
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t counts;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	pmc_enable_periph_clk(ID_TC);
	
	tc_find_mck_divisor( freq_desejada, ul_sysclk, &ul_div, &ul_tcclks,	BOARD_MCK);
	tc_init(TC, CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	counts = (ul_sysclk/ul_div)/freq_desejada;
	tc_write_rc(TC, CHANNEL, counts);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 4);
	NVIC_EnableIRQ(TC_IRQn);
	tc_enable_interrupt(TC,	CHANNEL, TC_IER_CPCS);
	tc_start(TC, CHANNEL);
}

void inicializacao_UART ()
{
	static usart_serial_options_t usart_options = 
	{
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS
	};
	usart_serial_init(CONF_UART, &usart_options);
	stdio_serial_init((Usart *)CONF_UART, &usart_options);
}

void TC_Handler(void)
{
	tc_get_status(TC,CHANNEL);
	LED_Toggle(LED0_GPIO);
}

void configuracoes_gerais()
{
	puts("Insira tempo maximo no escuro em horas:\r");
	scanf("%i", &escuro);
	escuro *= 3600;
	puts("Insira a luminosidade minima em %:\r");
	scanf("%f", &luz_min);
	luz_min /= 100.0;
	puts("Configuracao completa!\r");
}

int main (void)
{
	sysclk_init();
	board_init();
	tc_config(1);
	inicializacao_UART();
	configuracoes_gerais();
}

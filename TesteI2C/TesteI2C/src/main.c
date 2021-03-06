#include <asf.h>

#define TWI_CLK	200000
#define ENDERECO_SENSOR	0x68
#define COMANDO_LEITURA_1	0x3B

#define ILI93XX_LCD_CS      1

twi_options_t opt;
struct ili93xx_opt_t g_ili93xx_display_opt;

#define CONF_UART              UART0
#define CONF_UART_BAUDRATE     9600
#define CONF_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
#define CONF_UART_PARITY       US_MR_PAR_NO
#define CONF_UART_STOP_BITS    US_MR_NBSTOP_1_BIT
#define	PINO_LED_AZUL	PIO_PA19
#define PINO_LED_VERDE	PIO_PA20

void inicializacao_UART ()
{
	static usart_serial_options_t usart_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS
	};
	usart_serial_init(CONF_UART, &usart_options);
	stdio_serial_init((Usart *)CONF_UART, &usart_options);
	puts("Inicializado\r");
}

void configure_lcd()
{
	/** Enable peripheral clock */
	pmc_enable_periph_clk(ID_SMC);

	/** Configure SMC interface for Lcd */
	smc_set_setup_timing(SMC, ILI93XX_LCD_CS, SMC_SETUP_NWE_SETUP(2)
	| SMC_SETUP_NCS_WR_SETUP(2)
	| SMC_SETUP_NRD_SETUP(2)
	| SMC_SETUP_NCS_RD_SETUP(2));
	
	smc_set_pulse_timing(SMC, ILI93XX_LCD_CS, SMC_PULSE_NWE_PULSE(4)
	| SMC_PULSE_NCS_WR_PULSE(4)
	| SMC_PULSE_NRD_PULSE(10)
	| SMC_PULSE_NCS_RD_PULSE(10));
	
	smc_set_cycle_timing(SMC, ILI93XX_LCD_CS, SMC_CYCLE_NWE_CYCLE(10)
	| SMC_CYCLE_NRD_CYCLE(22));
	
	smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
	| SMC_MODE_WRITE_MODE);

	/** Initialize display parameter */
	g_ili93xx_display_opt.ul_width = ILI93XX_LCD_WIDTH;
	g_ili93xx_display_opt.ul_height = ILI93XX_LCD_HEIGHT;
	g_ili93xx_display_opt.foreground_color = COLOR_BLACK;
	g_ili93xx_display_opt.background_color = COLOR_WHITE;

	/** Switch off backlight */
	aat31xx_disable_backlight();

	/** Initialize LCD */
	ili93xx_init(&g_ili93xx_display_opt);

	/** Set backlight level */
	aat31xx_set_backlight(AAT31XX_AVG_BACKLIGHT_LEVEL);

	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(0, 0, ILI93XX_LCD_WIDTH,
	ILI93XX_LCD_HEIGHT);
	/** Turn on LCD */
	ili93xx_display_on();
	ili93xx_set_cursor_position(0, 0);
}

int main (void)
{
	sysclk_init();
	board_init();
	configure_lcd();
	inicializacao_UART();

//	pio_configure(PIOB, PIO_PERIPH_B, (PIO_PB5A_TWCK1 | PIO_PB4A_TWD1), PIO_OPENDRAIN);
	
	pmc_enable_periph_clk(ID_TWI1);

	opt.master_clk = sysclk_get_peripheral_hz();
	opt.speed =	TWI_CLK;
	twi_enable_master_mode(TWI1);
	twi_master_init(TWI1, &opt);
	
	twi_packet_t	pacote;
	uint8_t	resposta[20];

	resposta[0] = 0;
	
	pacote.addr[0] = 0x6b;
	pacote.addr_length = 1;
	pacote.chip = ENDERECO_SENSOR;
	pacote.buffer = &resposta;
	pacote.length = 1;
	
	//puts("Escrevendo...");
	//if(twi_master_write(TWI1, &pacote) == TWI_SUCCESS)
	//{
		//ili93xx_set_foreground_color(COLOR_WHITE);
		//ili93xx_draw_filled_rectangle(0, 0, 200, 200);
		//ili93xx_set_foreground_color(COLOR_BLACK);
		//ili93xx_draw_string(0, 0, "Enviou");
		//puts("Mensagem enviada\r");
	//}
	//else
		//puts("Falha ao escrever");

	while(1)
	{
		pacote.addr[0] = COMANDO_LEITURA_1;
		pacote.addr_length = 1;
		pacote.buffer = &resposta;		
		pacote.chip = ENDERECO_SENSOR;
		pacote.length = 14;

		twi_master_write(TWI1, &pacote);
		twi_master_read(TWI1, &pacote);
		
		char	a[20];
		int16_t	b;
		b = (resposta[0] << 8 | resposta[1]);
		sprintf(a,"%i",b);
		puts(a);
		ili93xx_set_foreground_color(COLOR_WHITE);
		ili93xx_draw_filled_rectangle(95, 175, 240, 200);
		ili93xx_set_foreground_color(COLOR_BLACK);
		ili93xx_draw_string(100, 180, a);
		delay_ms(100);
	}
}
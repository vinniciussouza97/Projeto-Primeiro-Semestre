#include <asf.h>
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include <string.h>

#define ILI93XX_LCD_CS      1

//Configurações da UART
#define CONF_UART              UART0
#define CONF_UART_BAUDRATE     9600
#define CONF_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
#define CONF_UART_PARITY       US_MR_PAR_NO
#define CONF_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

//Pinos dos LEDs
#define	PINO_LED_AZUL	PIO_PA19
#define PINO_LED_VERDE	PIO_PA20

//Configurações do Timer
#define TC			TC0
#define CHANNEL		0
#define ID_TC		ID_TC0
#define TC_Handler  TC0_Handler
#define TC_IRQn     TC0_IRQn

//Configurações do ADC
#define VOLT_REF        (3300)
#define TRACKING_TIME    15
#define TRANSFER_PERIOD  2
#define STARTUP_TIME	ADC_STARTUP_TIME_4
#define MAX_DIGITAL     (4095)
#define ADC_CHANNEL_LDR 5	//LDR no pino PB1
#define ADC_ISR_LDR	ADC_ISR_EOC5
#define ADC_CHANNEL_UMIDADE	0	//Sensor de umidade do solo PA17
#define ADC_ISR_UMIDADE	ADC_ISR_EOC0

#define UMIDADE_MINIMA	3200	//Umidade mínima do solo. Quanto maior, mais seco

#define PWM_FREQUENCY	1000	/** PWM frequency in Hz */
#define PERIOD_VALUE	4096	/** Period value of PWM output waveform */
#define INIT_DUTY_VALUE	1024	/** Initial duty cycle value */
#define PWM_CHANNEL_BOMBA	2	//Canal PWM da bomba
#define PWM_CHANNEL_BOMBA_EN	PWM_ENA_CHID2	//Enable do canal. Ultimo digito = canal
#define PIN_BOMBA	PIO_PDR_P13	//Pino da bomba
#define PIN_BOMBA_ABCDSR	PIO_ABCDSR_P13
#define PIO_BOMBA	PIOA

#define TEMPO_MAX_ESCURO	30	//Tempo máximo na escuridão
#define INTERVALO_MEDICAO	10	//Intervalo entre medições com a bomba desligada
#define INTERVALO_ATIVO	1	//Intervalo entre medições com a bomba ligada

struct ili93xx_opt_t g_ili93xx_display_opt;	//Display LCD

//Algumas variáveis globais para uso em múltiplas funções
uint32_t	escuro_max = TEMPO_MAX_ESCURO;
int32_t	escuro = TEMPO_MAX_ESCURO;	//Quando "escuro" chega em 0, o LED é aceso
uint32_t	luz_min = 50;	//Luz minima em %
uint32_t	duty_cycle = INIT_DUTY_VALUE;
int32_t	max_aceso = 8*3600;	//Tempo maximo com a luz acesa
uint32_t	tempo_entre_medicoes = INTERVALO_MEDICAO;
uint32_t	tempo_prox_medicao = INTERVALO_MEDICAO;

//Configurações do TC
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

//ISR do TC
void TC_Handler(void)
{
	tc_get_status(TC,CHANNEL);
	LED_Toggle(LED0_GPIO);
	tempo_prox_medicao--;	//Contagens ocorrem apenas após um intervalo
	
	//Reiniciar contagem, iniciar ADC
	if (tempo_prox_medicao <= 0)
	{
		adc_start(ADC);
		tempo_prox_medicao = tempo_entre_medicoes;
	}
	
	//Print de tempos de medição
	char buffer[100];
	sprintf(buffer, "Entre medicoes: %d\nProx medicao: %d\n", tempo_entre_medicoes, tempo_prox_medicao);

	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(0, 0, ILI93XX_LCD_WIDTH, 100);

	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_string(5, 5, (uint8_t*) buffer);
}

void ADC_Handler(void)
{
	uint16_t result;

	//Tratamento do LDR
	if (adc_get_status(ADC) & ADC_ISR_LDR)
	{
		result = adc_get_channel_value(ADC, ADC_CHANNEL_LDR);
		
		//exibição do último valor
		char buffer[30];
		sprintf (buffer, "LDR: %d", result);
		puts(buffer);
		
		//Print no LCD
		ili93xx_set_foreground_color(COLOR_WHITE);
		ili93xx_draw_filled_rectangle(0, 150, ILI93XX_LCD_WIDTH, ILI93XX_LCD_HEIGHT);
		ili93xx_set_foreground_color(COLOR_BLACK);
		ili93xx_draw_string(5, 155, (uint8_t*) buffer);
		
		//lógica de tempo para acender lâmpada
		if (result <= (4095*luz_min/100))
			escuro -= tempo_entre_medicoes;
		else
			escuro = escuro_max;
		sprintf (buffer, "Escuro: %d", escuro);
		puts(buffer);
		ili93xx_set_foreground_color(COLOR_WHITE);
		ili93xx_draw_filled_rectangle(0, 171, ILI93XX_LCD_WIDTH, ILI93XX_LCD_HEIGHT);
		ili93xx_set_foreground_color(COLOR_BLACK);
		ili93xx_draw_string(5, 176, (uint8_t*) buffer);
		
		if (escuro <= -max_aceso)
		{
			pio_set(PIOA, PINO_LED_VERDE);
			escuro = escuro_max;
		}
		else if (escuro <= 0)
			pio_clear(PIOA, PINO_LED_VERDE);
		else
			pio_set(PIOA, PINO_LED_VERDE);

	}
	//Tratamento do sensor de umidade do solo
	else if (adc_get_status(ADC) & ADC_ISR_UMIDADE)
	{
		result = adc_get_channel_value(ADC, ADC_CHANNEL_UMIDADE);
		
		char buffer[30];
		sprintf (buffer, "Umidade: %d", result);
		puts(buffer);
		ili93xx_set_foreground_color(COLOR_WHITE);
		ili93xx_draw_filled_rectangle(0, 192, ILI93XX_LCD_WIDTH, ILI93XX_LCD_HEIGHT);
		ili93xx_set_foreground_color(COLOR_BLACK);
		ili93xx_draw_string(5, 197, (uint8_t*) buffer);
		
		// Ligar a bomba baseado na umidade
		if (result >= UMIDADE_MINIMA)
		{
			duty_cycle = 0;
			PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CDTYUPD = duty_cycle;
			if (tempo_entre_medicoes != INTERVALO_ATIVO)
			{
				tempo_entre_medicoes = INTERVALO_ATIVO;
				tempo_prox_medicao = tempo_entre_medicoes;
			}
		}
		//Desligar bomba
		else
		{
			duty_cycle = 4095;
			PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CDTYUPD = duty_cycle;
			if (tempo_entre_medicoes != INTERVALO_MEDICAO)
			{
				tempo_entre_medicoes = INTERVALO_MEDICAO;
				tempo_prox_medicao = tempo_entre_medicoes;
			}
		}
	}
	//Tratamento do sensor de temperatura embutido
	else if (adc_get_status(ADC) & ADC_ISR_EOC15)
	{
		result = adc_get_channel_value(ADC, ADC_TEMPERATURE_SENSOR);
		
		//Cálculo da temperatura atual, vide Datasheet
		float	temp = (float)(result*VOLT_REF/MAX_DIGITAL - 1440) * 0.21276 + 27.0;
		char buffer[30];
		sprintf(buffer, "Temperatura: %.1f", temp);
		puts(buffer);
		ili93xx_set_foreground_color(COLOR_WHITE);
		ili93xx_draw_filled_rectangle(0, 213, ILI93XX_LCD_WIDTH, ILI93XX_LCD_HEIGHT);
		ili93xx_set_foreground_color(COLOR_BLACK);
		ili93xx_draw_string(5, 218, (uint8_t*) buffer);
	}
}

//Função de configuração para a temporização e medição do LDR
void configuracoes_gerais()
{
	char buffer[255], a;
	puts("Insira tempo maximo no escuro em horas:\r");
	escuro_max = getchar();
	escuro_max -= 48;
	escuro_max *= 3600;
	escuro = escuro_max;
	puts("Insira a luminosidade minima em \%:\r");
	luz_min = getchar();
	luz_min -= 48;
	luz_min *= 10;
	puts("Insira o tempo maximo com a luz acesa em horas\r");
	max_aceso = getchar();
	max_aceso -= 48;
	max_aceso *= 3600;
	puts("Configuracao completa!\r");
	sprintf(buffer,"Tempo: %i\tLuz: %i\%\tAceso: %i\n\r", escuro_max, luz_min, max_aceso);
	puts(buffer);
}

//Configurações do ADC
void configure_adc(void)
{
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);
	
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, STARTUP_TIME);
	adc_configure_timing(ADC, TRACKING_TIME	, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);
	adc_enable_channel(ADC, ADC_CHANNEL_LDR);
	adc_enable_channel(ADC, ADC_CHANNEL_UMIDADE);
	adc_enable_channel(ADC, ADC_TEMPERATURE_SENSOR);
	NVIC_SetPriority(ADC_IRQn, 5);
	NVIC_EnableIRQ(ADC_IRQn);
	adc_enable_interrupt(ADC, ADC_ISR_LDR);
	adc_enable_interrupt(ADC, ADC_ISR_UMIDADE);
	adc_enable_interrupt(ADC, ADC_ISR_EOC15);
	ADC->ADC_ACR |= ADC_ACR_TSON;	//Ativação do sensor de temperatura embutido
}

//Configuração do PWM
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

//Configuração do LCD
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

//Configuração da UART
void inicializacao_UART (){
	static usart_serial_options_t usart_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS
	};
	usart_serial_init(CONF_UART, &usart_options);
	stdio_serial_init((Usart *)CONF_UART, &usart_options);
	configuracoes_gerais();
	NVIC_SetPriority( UART0_IRQn, 5);
	NVIC_EnableIRQ( UART0_IRQn);
	uart_enable_interrupt(UART0, UART_IER_RXRDY);
}

//Comandos UART
void UART0_Handler()
{
	/* Get UART status and check if PDC receive buffer is full */
	if ((uart_get_status(UART0) & UART_IER_RXRDY) == UART_IER_RXRDY)
	{
		char key;
		uart_read(UART0, &key);
		switch (key)
		{
			case 'c':	//Com a letra 'c', abre-se o menu de configurações gerais
				configuracoes_gerais();
				break;
			case 'b':	//Com a letra 'b', aciona-se a bomba manualmente
				duty_cycle = 0;
				PWM->PWM_CH_NUM[PWM_CHANNEL_BOMBA].PWM_CDTYUPD = duty_cycle;
				if (tempo_entre_medicoes != INTERVALO_ATIVO)
				{
					tempo_entre_medicoes = INTERVALO_ATIVO;
					tempo_prox_medicao = tempo_entre_medicoes;
				}
				puts("Bomba iniciada\r");
				break;
			case 'm':	//Com a letra 'm', as medições atuais são enviadas para o Bluetooth
				adc_start(ADC);
				break;
		}
	}
}

int main (void)
{
	sysclk_init();
	board_init();
	inicializacao_UART();
	configure_adc();
	configure_pwm();
	configure_lcd();
	tc_config(1);

	pio_set_output(PIOA, PINO_LED_AZUL, HIGH, DISABLE, ENABLE);
	pio_set_output(PIOA, PINO_LED_VERDE, HIGH, DISABLE, ENABLE);

	while(1)
	{
		
	}
}

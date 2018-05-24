#include <asf.h>

/* Rotina de inicialização do PWM */
void PWM_init(void)
{
	// disable the PIO (peripheral controls the pin)
	PIOA->PIO_PDR = PIO_PDR_P20;
	// select alternate function B (PWML0) for pin PA19
	PIOA->PIO_ABCDSR[0] |= PIO_ABCDSR_P20;
	PIOA->PIO_ABCDSR[1] &= ~PIO_ABCDSR_P20;
	// Enable the PWM peripheral from the Power Manger
	PMC->PMC_PCER0 = (1 << ID_PWM);
	// Select the Clock to run at the MCK (4MHz)
	PWM->PWM_CH_NUM[1].PWM_CMR = PWM_CMR_CPRE_MCK;
	// select the period 10msec
	PWM->PWM_CH_NUM[1].PWM_CPRD = 4096;// freq em khz
	// select the duty cycle
	PWM->PWM_CH_NUM[1].PWM_CDTY = 3500;
	// enable the channel
	PWM->PWM_ENA = PWM_ENA_CHID1;
}

int main (void)
{
	sysclk_init();
	board_init();
	PWM_init();
	while(1){}
}

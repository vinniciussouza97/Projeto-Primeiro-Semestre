#include <asf.h>

#define TWI_CLK	400000

twi_options_t opt;

int main (void)
{
	sysclk_init();
	board_init();
	
	pmc_enable_periph_clk(TWI0);
	opt.master_clk = sysclk_get_peripheral_hz();
	opt.speed      = TWI_CLK;
	twi_enable_master_mode(TWI0);
	twi_master_init(TWI0, &opt);
}

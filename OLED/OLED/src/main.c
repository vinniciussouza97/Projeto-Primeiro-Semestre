#include <asf.h>
#include <string.h>

// Master
#define SPI_MASTER_BASE	SPI
#define SPI_CHIP_SEL	0
#define SPI_CHIP_PCS	spi_get_pcs(SPI_CHIP_SEL)
#define SPI_CLK_POLARITY	0
#define SPI_CLK_PHASE	0
/* Delay before SPCK. */
#define SPI_DLYBS	0x40
/* Delay between consecutive transfers. */
#define SPI_DLYBCT	0x10
/* SPI clock setting (Hz). */
static uint32_t gs_ul_spi_clock = 500000;

static void spi_master_initialize(void)
{
	puts("-I- Initialize SPI as master\r");

	/* Configure an SPI peripheral. */
	spi_enable_clock(SPI_MASTER_BASE);
	spi_disable(SPI_MASTER_BASE);
	spi_reset(SPI_MASTER_BASE);
	spi_set_lastxfer(SPI_MASTER_BASE);
	spi_set_master_mode(SPI_MASTER_BASE);
	spi_disable_mode_fault_detect(SPI_MASTER_BASE);
	spi_set_peripheral_chip_select_value(SPI_MASTER_BASE, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_MASTER_BASE, SPI_CHIP_SEL,
	SPI_CSR_BITS_8_BIT);
	spi_set_baudrate_div(SPI_MASTER_BASE, SPI_CHIP_SEL,	(sysclk_get_peripheral_hz()	/ gs_ul_spi_clock));
	spi_set_transfer_delay(SPI_MASTER_BASE, SPI_CHIP_SEL, SPI_DLYBS, SPI_DLYBCT);
	spi_enable(SPI_MASTER_BASE);
}

int main(void)
{
	sysclk_init();
	board_init();
	spi_master_initialize();
}
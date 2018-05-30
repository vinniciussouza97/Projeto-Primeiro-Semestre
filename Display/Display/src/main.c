/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <board.h>
#include <sysclk.h>
#include <ssd1306.h>

int main(void)
{
	//! the page address to write to
	uint8_t page_address;
	//! the column address, or the X pixel.
	uint8_t column_address;
	//! store the LCD controller start draw line
	uint8_t start_line_address = 0;

	board_init();
	sysclk_init();

	// Initialize SPI and SSD1306 controller
	ssd1306_init();

	// set addresses at beginning of display
	ssd1306_set_page_address(0);
	ssd1306_set_column_address(0);

	// fill display with lines
	for (page_address = 0; page_address <= 7; page_address++) {
		ssd1306_set_page_address(page_address);
		for (column_address = 0; column_address < 128; column_address++) {
			ssd1306_set_column_address(column_address);
			/* fill every other pixel in the display. This will produce
			horizontal lines on the display. */
			ssd1306_write_data(0x6F);
		}
	}

	// scroll the display using hardware support in the LCD controller
	while (true) {
		ssd1306_set_display_start_line_address(start_line_address++);
		delay_ms(250);
	}
}

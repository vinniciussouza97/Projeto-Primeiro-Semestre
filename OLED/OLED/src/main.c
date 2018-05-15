#include <asf.h>
#include <string.h>

int main(void)
{

	sysclk_init();
	board_init();

	// Initialize SPI and SSD1306 controller.
	ssd1306_init();

	
	while(1){
		// Clear screen.
		ssd1306_clear();
		
		//Set line and column to 0
		ssd1306_set_page_address(0);
		ssd1306_set_column_address(0);
		
		/// -------- First Screen --------
		ssd1306_write_text("Coffee consumption improves");
		
		delay_ms(1500);
		
		ssd1306_set_page_address(1);
		ssd1306_set_column_address(8);
		
		ssd1306_write_text("programming performance");
		
		delay_ms(1500);
		
		ssd1306_set_page_address(2);
		ssd1306_set_column_address(20);
		
		ssd1306_write_text("when coding in C.");
		
		delay_ms(1500);
		


		/// -------- Second Screen --------
		ssd1306_clear();
		ssd1306_set_page_address(0);
		ssd1306_set_column_address(0);
		
		uint8_t text[65];
		uint8_t* pText = text;
		uint8_t *char_ptr;
		uint8_t i=0, column=0, page=0;
		
		//use sprintf to create strings from numbers, variables and other strings
		sprintf(text, "When coding in Java, however, performance decreases in %f %%", 73.37);

		//print text character by character
		while(*pText){
			//write a single character
			char_ptr = font_table[*pText++ - 32];
			for (i = 1; i <= char_ptr[0]; i++) {
				ssd1306_write_data(char_ptr[i]);
			}
			
			//newline
			if(column++ == 35){
				column = 0;
				page++;
				ssd1306_set_column_address(column);
				ssd1306_set_page_address(page);
			}
			
			//wait
			delay_ms(100);
		}
	}
}
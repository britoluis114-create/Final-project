#ifndef LBRIT005_SPIAVR_H
#define LBRIT005_SPIAVR_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
//B5 should always be SCK(spi clock) and B3 should always be MOSI. If you are using an
//SPI peripheral that sends data back to the arduino, you will need to use B4 as the MISO pin.
//The SS pin can be any digital pin on the arduino. Right before sending an 8 bit value with
//the SPI_SEND() funtion, you will need to set your SS pin to low. If you have multiple SPI
//devices, they will share the SCK, MOSI and MISO pins but should have different SS pins.
//To send a value to a specific device, set it's SS pin to low and all other SS pins to high.
// Outputs, pin definitions



#define PIN_SCK PORTB5//SHOULD ALWAYS BE B5 ON THE ARDUINO
#define PIN_MOSI PORTB3//SHOULD ALWAYS BE B3 ON THE ARDUINO
#define PIN_SS PORTB2
#define PIN_DC PORTB0
#define PIN_RST PORTB1

#define SWRESET 0x01   /* Software Reset */
#define SLPOUT  0x11   /* Sleep Out */
#define COLMOD  0x3A   /* Color Mode Set */
#define DISPON  0x29   /* Display ON */
#define CASET   0x2A   /* Column Address Set */
#define RASET   0x2B   /* Row Address Set */
#define RAMWR   0x2C   /* Memory Write */

#define BLACK       0x000000
#define WHITE       0xFFFFFF
#define RED         0xFF0000
#define RED_LIGHT   0xFF6B6B
#define GREEN       0x00FF00
#define LIGHT_GREEN 0x90EE90
#define BRIGHT_GREEM 0x00FF00
#define DARK_GREEN  0x008000
#define BLUE        0x0000FF
#define YELLOW      0xFFFF00
#define CYAN        0x00FFFF
#define MAGENTA     0xFF00FF
#define ORANGE      0xFF8000
#define PURPLE      0x800080
#define GRAY        0x808080
#define DARK_GREY   0x3A3A3A
#define DARKER_GREY 0x2C2C2C
#define DARK_RED    0x800000
#define DARK_BLUE   0x000080
#define DARK_PURPLE 0x4B0082
#define BROWN 0xA52A2A
#define TEAL 0x2C5282
#define BLUE_GRAY 0x4A7BA7

extern volatile unsigned char grid[3][7];
extern volatile unsigned char sun;
extern volatile unsigned char gamestate;
extern volatile unsigned char plant_health[3][7];

typedef struct PEASHOOTING{ // when the peashooters start shooting
	
	uint16_t x;
	uint16_t y;
	uint16_t peashooters;
	uint16_t row;

} SHOOTS;

typedef struct ZOMBIES{ // when the zombies start moving
	uint16_t x;
	uint16_t y;
	uint16_t zombies;
	uint16_t row;
	uint16_t health;

} ZOMBIE;

#define SHOTS 15
#define ZOMBIES 5

extern SHOOTS PEA[SHOTS];
extern ZOMBIE ZOMBIE_ARRAY[ZOMBIES];

//If SS is on a different port, make sure to change the init to take that into account.
void SPI_INIT(){
DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_SS);//initialize your pins.
SPCR |= (1 << SPE) | (1 << MSTR); //initialize SPI coomunication

DDRB |= (1 << PIN_DC) | (1 << PIN_RST);//initialize DC and RST pins as outputs
PORTB |= (1 << PIN_SS) | (1 << PIN_DC) | (1 << PIN_RST);//set DC and RST pins high, set SS pin high to disable device
SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); //SPI enable, master mode, clock rate fck/16
SPSR &= ~(1 << SPI2X); //normal speed

}
void SPI_SEND(char data)
{
SPDR = data;//set data that you want to transmit
while (!(SPSR & (1 << SPIF)));// wait until done transmitting
}

void Delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        _delay_ms(1);
    }
}

void Send_Command(uint8_t cmd) {
    PORTB &= ~(1 << PIN_SS);        /* SS LOW - enable device */
    PORTB &= ~(1 << PIN_DC);        /* DC LOW - command mode */
    SPI_SEND(cmd);                  /* Send command via SPI */
    PORTB |= (1 << PIN_SS);         /* SS HIGH - disable device */
}

void Send_Data(uint8_t data) {
    PORTB &= ~(1 << PIN_SS);        /* SS LOW - enable device */
    PORTB |= (1 << PIN_DC);         /* DC HIGH - data mode */
    SPI_SEND(data);                  /* Send data via SPI */
    PORTB |= (1 << PIN_SS);         /* SS HIGH - disable device */
}

void HardwareReset(void) {

    PORTB &= ~(1 << PIN_RST);       /* RST LOW */
    Delay_ms(200);
    PORTB |= (1 << PIN_RST);        /* RST HIGH */
    Delay_ms(200);
}

void tft_init(void) {
    // Example initialization sequence for an LCD display
    HardwareReset();
    Send_Command(SWRESET);
    Delay_ms(150);
    Send_Command(SLPOUT);
    Delay_ms(200);
    Send_Command(COLMOD);
    Send_Data(0x06); // 18-bit color
    Delay_ms(10);
    Send_Command(DISPON);
    Delay_ms(200);
}



void fillScreen(uint32_t color) {
    // Example function to fill the screen with a single color
    // This will depend on the specific LCD you are using
    Send_Command(CASET);
    Send_Data(0);      
    Send_Data(0);       
    Send_Data(0);       
    Send_Data(159);     
    
    Send_Command(RASET);
    Send_Data(0);       
    Send_Data(0);       
    Send_Data(0);     
    Send_Data(159);     
    
    Send_Command(RAMWR);
    
    for (uint32_t i = 0; i < 25600; i++) {
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        Send_Data(r & 0xFC);
        Send_Data(g & 0xFC);
        Send_Data(b & 0xFC);
    }
}

void display_init(void){
	SPI_INIT();
	tft_init();
}

void fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
	// to draw
	if (x + width > 160) width = 160 - x;
    if (y + height > 160) height = 160 - y;
    
    Send_Command(CASET);
    Send_Data(x >> 8);
    Send_Data(x & 0xFF);
    Send_Data((x + width - 1) >> 8);
    Send_Data((x + width - 1) & 0xFF);
    
    Send_Command(RASET);
    Send_Data(y >> 8);
    Send_Data(y & 0xFF);
    Send_Data((y + height - 1) >> 8);
    Send_Data((y + height - 1) & 0xFF);
    
    Send_Command(RAMWR);


    uint32_t count = (uint32_t)width * height;
    
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        Send_Data(r & 0xFC);
        Send_Data(g & 0xFC);
        Send_Data(b & 0xFC);
    }
}


void Draw_Pixel(uint16_t x, uint16_t y, uint32_t color) {
	// to draw bits
    if (x >= 160 || y >= 160) return;
    
    Send_Command(CASET);
    Send_Data(x >> 8);
    Send_Data(x & 0xFF);
    Send_Data(x >> 8);
    Send_Data(x & 0xFF);
    
    Send_Command(RASET);
    Send_Data(y >> 8);
    Send_Data(y & 0xFF);
    Send_Data(y >> 8);
    Send_Data(y & 0xFF);
    
    Send_Command(RAMWR);
    
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    Send_Data(r & 0xFC);
    Send_Data(g & 0xFC);
    Send_Data(b & 0xFC);
}

void drawRectBorder(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){
// to draw borders
	fillRectangle(x, y, width, 1, color);

	fillRectangle(x, y + height - 1, width, 1, color);

	fillRectangle(x, y, 1, height, color);

	fillRectangle(x + width - 1, y, 1, height, color);

}

void drawNumber(uint16_t x, uint16_t y, unsigned char c, uint32_t color){

if(c == 0){
    fillRectangle(x+4*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+2*4, y+0*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);

    fillRectangle(x+4*4, y+1*4, 4,4, color);
    fillRectangle(x+1*4, y+1*4, 4,4, color);

    fillRectangle(x+4*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);

    fillRectangle(x+4*4, y+3*4, 4,4, color);
    fillRectangle(x+1*4, y+3*4, 4,4, color);

    fillRectangle(x+4*4, y+4*4, 4,4, color);
    fillRectangle(x+1*4, y+4*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);
    fillRectangle(x+2*4, y+4*4, 4,4, color);
	
	}

	else if(c == 1){
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+1*4, 4,4, color);
    fillRectangle(x+3*4, y+2*4, 4,4, color);
    fillRectangle(x+3*4, y+3*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);

	}

	else if(c == 2){
    fillRectangle(x+4*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+2*4, y+0*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);

    fillRectangle(x+1*4, y+1*4, 4,4, color);

    fillRectangle(x+4*4, y+2*4, 4,4, color);
    fillRectangle(x+3*4, y+2*4, 4,4, color);
    fillRectangle(x+2*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);

    fillRectangle(x+1*4, y+3*4, 4,4, color);

    fillRectangle(x+4*4, y+4*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);
    fillRectangle(x+2*4, y+4*4, 4,4, color);
    fillRectangle(x+1*4, y+4*4, 4,4, color);

	}
	else if(c == 3){
    	fillRectangle(x+4*4, y+0*4, 4,4, color);
    	fillRectangle(x+3*4, y+0*4, 4,4, color);
    	fillRectangle(x+2*4, y+0*4, 4,4, color);
    	fillRectangle(x+1*4, y+0*4, 4,4, color);

 	   fillRectangle(x+1*4, y+1*4, 4,4, color);

    	fillRectangle(x+4*4, y+2*4, 4,4, color);
  	  fillRectangle(x+3*4, y+2*4, 4,4, color);
  	  fillRectangle(x+2*4, y+2*4, 4,4, color);
   	 fillRectangle(x+1*4, y+2*4, 4,4, color);

    	fillRectangle(x+1*4, y+3*4, 4,4, color);

  	  fillRectangle(x+4*4, y+4*4, 4,4, color);
 	   fillRectangle(x+3*4, y+4*4, 4,4, color);
  	  fillRectangle(x+2*4, y+4*4, 4,4, color);
  	  fillRectangle(x+1*4, y+4*4, 4,4, color);
	}
		else if(c == 4){
    		fillRectangle(x+4*4, y+4*4, 4,4, color);
    		fillRectangle(x+1*4, y+4*4, 4,4, color);

    		fillRectangle(x+4*4, y+3*4, 4,4, color);
    		fillRectangle(x+1*4, y+3*4, 4,4, color);

    	fillRectangle(x+4*4, y+2*4, 4,4, color);
    	fillRectangle(x+3*4, y+2*4, 4,4, color);
    	fillRectangle(x+2*4, y+2*4, 4,4, color);
    	fillRectangle(x+1*4, y+2*4, 4,4, color);

	    fillRectangle(x+1*4, y+1*4, 4,4, color);
    	fillRectangle(x+1*4, y+0*4, 4,4, color);
	}
		else if(c == 5){
    	fillRectangle(x+4*4, y+4*4, 4,4, color);
  	  fillRectangle(x+3*4, y+4*4, 4,4, color);
   	 fillRectangle(x+2*4, y+4*4, 4,4, color);
   		 fillRectangle(x+1*4, y+4*4, 4,4, color);

	    fillRectangle(x+4*4, y+3*4, 4,4, color);

    	fillRectangle(x+4*4, y+2*4, 4,4, color);
  	  fillRectangle(x+3*4, y+2*4, 4,4, color);
   	 fillRectangle(x+2*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);

    fillRectangle(x+1*4, y+1*4, 4,4, color);

    fillRectangle(x+4*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+2*4, y+0*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);
	}

	else if(c == 6){
    fillRectangle(x+4*4, y+4*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);
    fillRectangle(x+2*4, y+4*4, 4,4, color);
    fillRectangle(x+1*4, y+4*4, 4,4, color);

    fillRectangle(x+4*4, y+3*4, 4,4, color);

    fillRectangle(x+4*4, y+2*4, 4,4, color);
    fillRectangle(x+3*4, y+2*4, 4,4, color);
    fillRectangle(x+2*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);

    fillRectangle(x+4*4, y+1*4, 4,4, color);
    fillRectangle(x+1*4, y+1*4, 4,4, color);

    fillRectangle(x+4*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+2*4, y+0*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);
	}
		else if(c == 7){
    fillRectangle(x+4*4, y+4*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);
    fillRectangle(x+2*4, y+4*4, 4,4, color);
    fillRectangle(x+1*4, y+4*4, 4,4, color);

    fillRectangle(x+1*4, y+3*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+1*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);

	}
	else if(c == 8){
    fillRectangle(x+4*4, y+0*4, 4,4, color);
    fillRectangle(x+3*4, y+0*4, 4,4, color);
    fillRectangle(x+2*4, y+0*4, 4,4, color);
    fillRectangle(x+1*4, y+0*4, 4,4, color);

    fillRectangle(x+4*4, y+1*4, 4,4, color);
    fillRectangle(x+1*4, y+1*4, 4,4, color);

    fillRectangle(x+4*4, y+2*4, 4,4, color);
    fillRectangle(x+3*4, y+2*4, 4,4, color);
    fillRectangle(x+2*4, y+2*4, 4,4, color);
    fillRectangle(x+1*4, y+2*4, 4,4, color);

    fillRectangle(x+4*4, y+3*4, 4,4, color);
    fillRectangle(x+1*4, y+3*4, 4,4, color);

    fillRectangle(x+4*4, y+4*4, 4,4, color);
    fillRectangle(x+3*4, y+4*4, 4,4, color);
    fillRectangle(x+2*4, y+4*4, 4,4, color);
    fillRectangle(x+1*4, y+4*4, 4,4, color);
	
	}
		else if(c == 9){
    	fillRectangle(x+4*4, y+4*4, 4,4, color);
    	fillRectangle(x+3*4, y+4*4, 4,4, color);
    	fillRectangle(x+2*4, y+4*4, 4,4, color);
   	 fillRectangle(x+1*4, y+4*4, 4,4, color);

    	fillRectangle(x+4*4, y+3*4, 4,4, color);
    	fillRectangle(x+1*4, y+3*4, 4,4, color);

    	fillRectangle(x+4*4, y+2*4, 4,4, color);
    	fillRectangle(x+3*4, y+2*4, 4,4, color);
    	fillRectangle(x+2*4, y+2*4, 4,4, color);
    	fillRectangle(x+1*4, y+2*4, 4,4, color);

	    fillRectangle(x+1*4, y+1*4, 4,4, color);

	    fillRectangle(x+4*4, y+0*4, 4,4, color);
	    fillRectangle(x+3*4, y+0*4, 4,4, color);
 	   fillRectangle(x+2*4, y+0*4, 4,4, color);
  	  fillRectangle(x+1*4, y+0*4, 4,4, color);
	}

}


void fillrect(uint16_t x, uint16_t y, uint16_t wid, uint16_t height, uint32_t color){
	//this filled a rectangle 
	for(uint16_t row = y; row < y + height; row++){
		for(uint16_t col = x; col < x +wid; col++){
		
		}
	}
}
//theses are the zombies
void zombie(uint16_t x, uint16_t y){
	Draw_Pixel(x+10,y+0, BLACK);
	Draw_Pixel(x+9,y+0, BLACK);

	Draw_Pixel(x+5,y+0, BLACK);
	Draw_Pixel(x+4,y+0, BLACK);

	Draw_Pixel(x+11,y+1, BLACK);
	Draw_Pixel(x+10,y+1, GREEN);
	Draw_Pixel(x+9,y+1, BLACK);
	Draw_Pixel(x+6,y+1, BLACK);
	Draw_Pixel(x+5, y+1, GREEN);
	Draw_Pixel(x+4,y+1, BLACK);

	Draw_Pixel(x+13,y+2, BLACK);
	Draw_Pixel(x+12,y+2, BLACK);
	Draw_Pixel(x+11,y+2, BLACK);
	Draw_Pixel(x+10,y+2, DARKER_GREY);
	Draw_Pixel(x+9,y+2,DARKER_GREY);
	Draw_Pixel(x+8,y+2, DARKER_GREY);
	Draw_Pixel(x+7,y+2,DARKER_GREY);
	Draw_Pixel(x+10,y+2, DARKER_GREY);
	Draw_Pixel(x+7,y+2,BLACK);
	Draw_Pixel(x+5, y+2, BLACK);
	Draw_Pixel(x+4,y+2, BLACK);

	Draw_Pixel(x+13,y+3, BLACK);
	Draw_Pixel(x+12,y+3, GREEN);
	Draw_Pixel(x+11,y+3, BLACK);
	Draw_Pixel(x+10,y+3, DARKER_GREY);
	Draw_Pixel(x+9,y+3,DARKER_GREY);
	Draw_Pixel(x+8,y+3, DARKER_GREY);
	Draw_Pixel(x+7,y+3,DARKER_GREY);
	Draw_Pixel(x+6,y+3, DARKER_GREY);
	Draw_Pixel(x+5,y+3,BLACK);
	Draw_Pixel(x+4, y+3, GREEN);
	Draw_Pixel(x+3,y+3, BLACK);


	Draw_Pixel(x+12,y+4, BLACK);
	Draw_Pixel(x+11,y+4, BLACK);
	Draw_Pixel(x+10,y+4, DARKER_GREY);
	Draw_Pixel(x+9,y+4,DARKER_GREY);
	Draw_Pixel(x+8,y+4, DARKER_GREY);
	Draw_Pixel(x+8,y+4,DARKER_GREY);
	Draw_Pixel(x+7,y+4, DARKER_GREY);
	Draw_Pixel(x+6,y+4, DARKER_GREY);
	Draw_Pixel(x+5,y+4,GREEN);
	Draw_Pixel(x+4, y+4, GREEN);
	Draw_Pixel(x+3,y+4, BLACK);


	Draw_Pixel(x+11,y+5, BLACK);
	Draw_Pixel(x+10,y+5, BLACK);
	Draw_Pixel(x+9,y+5,BLACK);
	Draw_Pixel(x+8,y+5, BLACK);
	Draw_Pixel(x+8,y+5,BLACK);
	Draw_Pixel(x+7,y+5, BLACK);
	Draw_Pixel(x+6,y+5,BLACK);
	Draw_Pixel(x+5, y+5, DARKER_GREY);
	Draw_Pixel(x+4,y+5, BLACK);

	Draw_Pixel(x+12,y+6, BLACK);
	Draw_Pixel(x+11,y+6, GREEN);
	Draw_Pixel(x+10,y+6,GREEN);
	Draw_Pixel(x+9,y+6, GREEN);
	Draw_Pixel(x+8,y+6,GREEN);
	Draw_Pixel(x+7,y+6, GREEN);
	Draw_Pixel(x+6,y+6,GREEN);
	Draw_Pixel(x+5, y+6, GREEN);
	Draw_Pixel(x+4,y+6, BLACK);

	Draw_Pixel(x+13,y+7, BLACK);
	Draw_Pixel(x+12,y+7, GREEN);
	Draw_Pixel(x+11,y+7, GREEN);
	Draw_Pixel(x+10,y+7,GREEN);
	Draw_Pixel(x+9,y+7, GREEN);
	Draw_Pixel(x+8,y+7,GREEN);
	Draw_Pixel(x+7,y+7, GREEN);
	Draw_Pixel(x+6,y+7,GREEN);
	Draw_Pixel(x+5, y+7, GREEN);
	Draw_Pixel(x+4,y+7, GREEN);
	Draw_Pixel(x+3,y+7, BLACK);

	Draw_Pixel(x+13,y+8, BLACK);
	Draw_Pixel(x+12,y+8, WHITE);
	Draw_Pixel(x+11,y+8, GREEN);
	Draw_Pixel(x+10,y+8,GREEN);
	Draw_Pixel(x+9,y+8, GREEN);
	Draw_Pixel(x+8,y+8,GREEN);
	Draw_Pixel(x+7,y+8, WHITE);
	Draw_Pixel(x+6,y+8,WHITE);
	Draw_Pixel(x+5, y+8, GREEN);
	Draw_Pixel(x+4,y+8, RED);
	Draw_Pixel(x+3, y+8, RED);
	Draw_Pixel(x+2,y+8, BLACK);

	Draw_Pixel(x+13,y+9, BLACK);
	Draw_Pixel(x+12,y+9, WHITE);
	Draw_Pixel(x+11,y+9, GREEN);
	Draw_Pixel(x+10,y+9,GREEN);
	Draw_Pixel(x+9,y+9, GREEN);
	Draw_Pixel(x+8,y+9, GREEN);
	Draw_Pixel(x+7,y+9,WHITE);
	Draw_Pixel(x+6,y+9, WHITE);
	Draw_Pixel(x+5,y+9,RED);
	Draw_Pixel(x+4, y+9, BROWN);
	Draw_Pixel(x+3,y+9, RED);
	Draw_Pixel(x+2, y+9, BLACK);

	Draw_Pixel(x+13,y+10, BLACK);
	Draw_Pixel(x+12,y+10, GREEN);
	Draw_Pixel(x+11,y+10, GREEN);
	Draw_Pixel(x+10,y+10,GREEN);
	Draw_Pixel(x+9,y+10, GREEN);
	Draw_Pixel(x+8,y+10, GREEN);
	Draw_Pixel(x+7,y+10,GREEN);
	Draw_Pixel(x+6,y+10, RED);
	Draw_Pixel(x+5,y+10,BROWN);
	Draw_Pixel(x+4, y+10, RED);
	Draw_Pixel(x+3,y+10, BLACK);

	Draw_Pixel(x+13, y+11, BLACK);
	Draw_Pixel(x+12,y+11, GREEN);
	Draw_Pixel(x+11,y+11, GREEN);
	Draw_Pixel(x+10,y+11,GREEN);
	Draw_Pixel(x+9,y+11, GREEN);
	Draw_Pixel(x+8,y+11, GREEN);
	Draw_Pixel(x+7,y+11,RED);
	Draw_Pixel(x+6,y+11, RED);
	Draw_Pixel(x+5,y+11,RED);
	Draw_Pixel(x+4, y+11, RED);
	Draw_Pixel(x+3,y+11, BLACK);

	
	Draw_Pixel(x+12,y+12, BLACK);
	Draw_Pixel(x+11,y+12, GREEN);
	Draw_Pixel(x+10,y+12,GREEN);
	Draw_Pixel(x+9,y+12, GREEN);
	Draw_Pixel(x+8,y+12, RED);
	Draw_Pixel(x+7,y+12,RED);
	Draw_Pixel(x+6,y+12, BROWN);
	Draw_Pixel(x+5,y+12,RED);
	Draw_Pixel(x+4, y+12, BLACK);

	Draw_Pixel(x+11,y+13, BLACK);
	Draw_Pixel(x+10,y+13,BLACK);
	Draw_Pixel(x+9,y+13, BLACK);
	Draw_Pixel(x+8,y+13, BLACK);
	Draw_Pixel(x+7,y+13,BLACK);
	Draw_Pixel(x+6,y+13, BLACK);
	Draw_Pixel(x+5,y+13,BLACK);
	
	
	




}

void secVariant(uint16_t x, uint16_t y){

	Draw_Pixel(x+10,y+0, BLACK);
	Draw_Pixel(x+9,y+0, BLACK);
	Draw_Pixel(x+5,y+0, BLACK);
	Draw_Pixel(x+4,y+0, BLACK);

	Draw_Pixel(x+11,y+1, BLACK);
	Draw_Pixel(x+10,y+1, GRAY);
	Draw_Pixel(x+9,y+1, BLACK);
	Draw_Pixel(x+6,y+1, BLACK);
	Draw_Pixel(x+5, y+1, GRAY);
	Draw_Pixel(x+4,y+1, BLACK);

	Draw_Pixel(x+13,y+2, BLACK);
	Draw_Pixel(x+12,y+2, BLACK);
	Draw_Pixel(x+11,y+2, BLACK);
	Draw_Pixel(x+10,y+2, GRAY);
	Draw_Pixel(x+9,y+2,GRAY);
	Draw_Pixel(x+8,y+2, GRAY);
	Draw_Pixel(x+7,y+2,GRAY);
	Draw_Pixel(x+10,y+2, GRAY);
	Draw_Pixel(x+7,y+2,BLACK);
	Draw_Pixel(x+5, y+2, BLACK);
	Draw_Pixel(x+4,y+2, BLACK);

	Draw_Pixel(x+13,y+3, BLACK);
	Draw_Pixel(x+12,y+3, GREEN);
	Draw_Pixel(x+11,y+3, BLACK);
	Draw_Pixel(x+10,y+3, GRAY);
	Draw_Pixel(x+9,y+3,GRAY);
	Draw_Pixel(x+8,y+3, GRAY);
	Draw_Pixel(x+7,y+3,GRAY);
	Draw_Pixel(x+6,y+3, GRAY);
	Draw_Pixel(x+5,y+3,BLACK);
	Draw_Pixel(x+4, y+3, GREEN);
	Draw_Pixel(x+3,y+3, BLACK);


	Draw_Pixel(x+12,y+4, BLACK);
	Draw_Pixel(x+11,y+4, BLACK);
	Draw_Pixel(x+10,y+4, GRAY);
	Draw_Pixel(x+9,y+4,GRAY);
	Draw_Pixel(x+8,y+4, GRAY);
	Draw_Pixel(x+8,y+4,GRAY);
	Draw_Pixel(x+7,y+4, GRAY);
	Draw_Pixel(x+6,y+4, GRAY);
	Draw_Pixel(x+5,y+4,GREEN);
	Draw_Pixel(x+4, y+4, GREEN);
	Draw_Pixel(x+3,y+4, BLACK);


	Draw_Pixel(x+11,y+5, BLACK);
	Draw_Pixel(x+10,y+5, BLACK);
	Draw_Pixel(x+9,y+5,BLACK);
	Draw_Pixel(x+8,y+5, BLACK);
	Draw_Pixel(x+8,y+5,BLACK);
	Draw_Pixel(x+7,y+5, BLACK);
	Draw_Pixel(x+6,y+5,BLACK);
	Draw_Pixel(x+5, y+5, DARKER_GREY);
	Draw_Pixel(x+4,y+5, BLACK);

	Draw_Pixel(x+12,y+6, BLACK);
	Draw_Pixel(x+11,y+6, GREEN);
	Draw_Pixel(x+10,y+6,GREEN);
	Draw_Pixel(x+9,y+6, GRAY);
	Draw_Pixel(x+8,y+6,GRAY);
	Draw_Pixel(x+7,y+6, GRAY);
	Draw_Pixel(x+6,y+6,GRAY);
	Draw_Pixel(x+5, y+6, GRAY);
	Draw_Pixel(x+4,y+6, BLACK);

	Draw_Pixel(x+13,y+7, BLACK);
	Draw_Pixel(x+12,y+7, GREEN);
	Draw_Pixel(x+11,y+7, GREEN);
	Draw_Pixel(x+10,y+7,GREEN);
	Draw_Pixel(x+9,y+7, GRAY);
	Draw_Pixel(x+8,y+7,GRAY);
	Draw_Pixel(x+7,y+7, GRAY);
	Draw_Pixel(x+6,y+7,GRAY);
	Draw_Pixel(x+5, y+7, GRAY);
	Draw_Pixel(x+4,y+7, GRAY);
	Draw_Pixel(x+3,y+7, BLACK);

	Draw_Pixel(x+13,y+8, BLACK);
	Draw_Pixel(x+12,y+8, WHITE);
	Draw_Pixel(x+11,y+8, GREEN);
	Draw_Pixel(x+10,y+8,GREEN);
	Draw_Pixel(x+9,y+8, GRAY);
	Draw_Pixel(x+8,y+8,GRAY);
	Draw_Pixel(x+7,y+8, WHITE);
	Draw_Pixel(x+6,y+8,WHITE);
	Draw_Pixel(x+5, y+8, GRAY);
	Draw_Pixel(x+4,y+8, RED);
	Draw_Pixel(x+3, y+8, RED);
	Draw_Pixel(x+2,y+8, BLACK);

	Draw_Pixel(x+13,y+9, BLACK);
	Draw_Pixel(x+12,y+9, GREEN);
	Draw_Pixel(x+11,y+9, GREEN);
	Draw_Pixel(x+10,y+9,GREEN);
	Draw_Pixel(x+9,y+9, GRAY);
	Draw_Pixel(x+8,y+9, GRAY);
	Draw_Pixel(x+7,y+9,WHITE);
	Draw_Pixel(x+6,y+9, WHITE);
	Draw_Pixel(x+5,y+9,GRAY);
	Draw_Pixel(x+4, y+9, BROWN);
	Draw_Pixel(x+3,y+9, RED);
	Draw_Pixel(x+2, y+9, BLACK);

	Draw_Pixel(x+13,y+10, BLACK);
	Draw_Pixel(x+12,y+10, GREEN);
	Draw_Pixel(x+11,y+10, GREEN);
	Draw_Pixel(x+10,y+10,GRAY);
	Draw_Pixel(x+9,y+10, GREEN);
	Draw_Pixel(x+8,y+10, GREEN);
	Draw_Pixel(x+7,y+10,GREEN);
	Draw_Pixel(x+6,y+10, RED);
	Draw_Pixel(x+5,y+10,BROWN);
	Draw_Pixel(x+4, y+10, RED);
	Draw_Pixel(x+3,y+10, BLACK);

	Draw_Pixel(x+13, y+11, BLACK);
	Draw_Pixel(x+12,y+11, GREEN);
	Draw_Pixel(x+11,y+11, GREEN);
	Draw_Pixel(x+10,y+11,GRAY);
	Draw_Pixel(x+9,y+11, GRAY);
	Draw_Pixel(x+8,y+11, GRAY);
	Draw_Pixel(x+7,y+11,GRAY);
	Draw_Pixel(x+6,y+11, RED);
	Draw_Pixel(x+5,y+11,RED);
	Draw_Pixel(x+4, y+11, RED);
	Draw_Pixel(x+3,y+11, BLACK);

	
	Draw_Pixel(x+12,y+12, BLACK);
	Draw_Pixel(x+11,y+12, GREEN);
	Draw_Pixel(x+10,y+12,GREEN);
	Draw_Pixel(x+9,y+12, GRAY);
	Draw_Pixel(x+8,y+12, GRAY);
	Draw_Pixel(x+7,y+12,GRAY);
	Draw_Pixel(x+6,y+12, BROWN);
	Draw_Pixel(x+5,y+12,RED);
	Draw_Pixel(x+4, y+12, BLACK);

	Draw_Pixel(x+11,y+13, BLACK);
	Draw_Pixel(x+10,y+13,BLACK);
	Draw_Pixel(x+9,y+13, BLACK);
	Draw_Pixel(x+8,y+13, BLACK);
	Draw_Pixel(x+7,y+13,BLACK);
	Draw_Pixel(x+6,y+13, BLACK);
	Draw_Pixel(x+5,y+13,BLACK);
	

}

void fastVar(uint16_t x, uint16_t y){

	
	Draw_Pixel(x+10,y+0, BLACK);
	Draw_Pixel(x+9,y+0, BLACK);
	Draw_Pixel(x+5,y+0, BLACK);
	Draw_Pixel(x+4,y+0, BLACK);

	Draw_Pixel(x+11,y+1, BLACK);
	Draw_Pixel(x+10,y+1, TEAL);
	Draw_Pixel(x+9,y+1, BLACK);
	Draw_Pixel(x+6,y+1, BLACK);
	Draw_Pixel(x+5, y+1, TEAL);
	Draw_Pixel(x+4,y+1, BLACK);

	Draw_Pixel(x+13,y+2, BLACK);
	Draw_Pixel(x+12,y+2, BLACK);
	Draw_Pixel(x+11,y+2, BLACK);
	Draw_Pixel(x+10,y+2, BLUE_GRAY);
	Draw_Pixel(x+9,y+2,BLUE_GRAY);
	Draw_Pixel(x+8,y+2, BLUE_GRAY);
	Draw_Pixel(x+7,y+2,BLUE_GRAY);
	Draw_Pixel(x+10,y+2, BLUE_GRAY);
	Draw_Pixel(x+7,y+2,BLACK);
	Draw_Pixel(x+5, y+2, BLACK);
	Draw_Pixel(x+4,y+2, BLACK);

	Draw_Pixel(x+13,y+3, BLACK);
	Draw_Pixel(x+12,y+3, WHITE);
	Draw_Pixel(x+11,y+3, BLACK);
	Draw_Pixel(x+10,y+3, BLUE_GRAY);
	Draw_Pixel(x+9,y+3,BLUE_GRAY);
	Draw_Pixel(x+8,y+3, BLUE_GRAY);
	Draw_Pixel(x+7,y+3,BLUE_GRAY);
	Draw_Pixel(x+6,y+3, BLUE_GRAY);
	Draw_Pixel(x+5,y+3,BLUE_GRAY);
	Draw_Pixel(x+4, y+3, WHITE);
	Draw_Pixel(x+3,y+3, BLACK);


	Draw_Pixel(x+12,y+4, BLACK);
	Draw_Pixel(x+11,y+4, BLACK);
	Draw_Pixel(x+10,y+4, BLUE_GRAY);
	Draw_Pixel(x+9,y+4,BLUE_GRAY);
	Draw_Pixel(x+8,y+4, BLUE_GRAY);
	Draw_Pixel(x+8,y+4,BLUE_GRAY);
	Draw_Pixel(x+7,y+4, BLUE_GRAY);
	Draw_Pixel(x+6,y+4, BLUE_GRAY);
	Draw_Pixel(x+5,y+4,GREEN);
	Draw_Pixel(x+4, y+4, GREEN);
	Draw_Pixel(x+3,y+4, BLACK);


	Draw_Pixel(x+11,y+5, BLACK);
	Draw_Pixel(x+10,y+5, BLACK);
	Draw_Pixel(x+9,y+5,BLACK);
	Draw_Pixel(x+8,y+5, BLACK);
	Draw_Pixel(x+8,y+5,BLACK);
	Draw_Pixel(x+7,y+5, BLACK);
	Draw_Pixel(x+6,y+5,BLACK);
	Draw_Pixel(x+5, y+5, DARKER_GREY);
	Draw_Pixel(x+4,y+5, BLACK);

	Draw_Pixel(x+12,y+6, BLACK);
	Draw_Pixel(x+11,y+6, GREEN);
	Draw_Pixel(x+10,y+6,GREEN);
	Draw_Pixel(x+9,y+6, GREEN);
	Draw_Pixel(x+8,y+6,GREEN);
	Draw_Pixel(x+7,y+6, GREEN);
	Draw_Pixel(x+6,y+6,GREEN);
	Draw_Pixel(x+5, y+6, GREEN);
	Draw_Pixel(x+4,y+6, BLACK);

	Draw_Pixel(x+13,y+7, BLACK);
	Draw_Pixel(x+12,y+7, GREEN);
	Draw_Pixel(x+11,y+7, GREEN);
	Draw_Pixel(x+10,y+7,GREEN);
	Draw_Pixel(x+9,y+7, GREEN);
	Draw_Pixel(x+8,y+7,GREEN);
	Draw_Pixel(x+7,y+7, GREEN);
	Draw_Pixel(x+6,y+7,GREEN);
	Draw_Pixel(x+5, y+7, GREEN);
	Draw_Pixel(x+4,y+7, GREEN);
	Draw_Pixel(x+3,y+7, BLACK);

	Draw_Pixel(x+13,y+8, BLACK);
	Draw_Pixel(x+12,y+8, WHITE);
	Draw_Pixel(x+11,y+8, GREEN);
	Draw_Pixel(x+10,y+8,GREEN);
	Draw_Pixel(x+9,y+8, GREEN);
	Draw_Pixel(x+8,y+8,GREEN);
	Draw_Pixel(x+7,y+8, WHITE);
	Draw_Pixel(x+6,y+8,WHITE);
	Draw_Pixel(x+5, y+8, BLUE_GRAY);
	Draw_Pixel(x+4,y+8, BLUE_GRAY);
	Draw_Pixel(x+3, y+8, BLUE_GRAY);
	Draw_Pixel(x+2,y+8, BLUE_GRAY);
	Draw_Pixel(x+1,y+8, BLUE_GRAY);

	Draw_Pixel(x+13,y+9, BLACK);
	Draw_Pixel(x+12,y+9, GREEN);
	Draw_Pixel(x+11,y+9, GREEN);
	Draw_Pixel(x+10,y+9,GREEN);
	Draw_Pixel(x+9,y+9, GREEN);
	Draw_Pixel(x+8,y+9, GRAY);
	Draw_Pixel(x+7,y+9,WHITE);
	Draw_Pixel(x+6,y+9, WHITE);
	Draw_Pixel(x+5,y+9,BLUE_GRAY);
	Draw_Pixel(x+4, y+9, BLUE_GRAY);
	Draw_Pixel(x+3,y+9, BLUE_GRAY);
	Draw_Pixel(x+2, y+9, BLUE_GRAY);
	Draw_Pixel(x+1,y+9,BLUE_GRAY);

	Draw_Pixel(x+13,y+10, BLACK);
	Draw_Pixel(x+12,y+10, GREEN);
	Draw_Pixel(x+11,y+10, GREEN);
	Draw_Pixel(x+10,y+10,GREEN);
	Draw_Pixel(x+9,y+10, GREEN);
	Draw_Pixel(x+8,y+10, GREEN);
	Draw_Pixel(x+7,y+10,BLUE_GRAY);
	Draw_Pixel(x+6,y+10, BLUE_GRAY);
	Draw_Pixel(x+5,y+10,BLUE_GRAY);
	Draw_Pixel(x+4, y+10, BLUE_GRAY);
	Draw_Pixel(x+3,y+10, BLUE_GRAY);
	Draw_Pixel(x+2,y+10, BLUE_GRAY);
	Draw_Pixel(x+1,y+10, BLUE_GRAY);

	Draw_Pixel(x+14, y+11,BLUE_GRAY);
	Draw_Pixel(x+13, y+11, BLUE_GRAY);
	Draw_Pixel(x+12,y+11, BLUE_GRAY);
	Draw_Pixel(x+11,y+11, BLUE_GRAY);
	Draw_Pixel(x+10,y+11,BLUE_GRAY);
	Draw_Pixel(x+9,y+11, BLUE_GRAY);
	Draw_Pixel(x+8,y+11, BLUE_GRAY);
	Draw_Pixel(x+7,y+11,BLUE_GRAY);
	Draw_Pixel(x+6,y+11, BLUE_GRAY);
	Draw_Pixel(x+5,y+11,BLUE_GRAY);
	Draw_Pixel(x+4, y+11, BLUE_GRAY);
	Draw_Pixel(x+3,y+11, BLUE_GRAY);
	Draw_Pixel(x+2, y+11,BLUE_GRAY);
	Draw_Pixel(x+1, y+11,BLUE_GRAY);


	Draw_Pixel(x+14,y+12, BLUE_GRAY);
	Draw_Pixel(x+13,y+12, BLUE_GRAY);	
	Draw_Pixel(x+12,y+12, BLUE_GRAY);
	Draw_Pixel(x+11,y+12, BLUE_GRAY);
	Draw_Pixel(x+10,y+12,BLUE_GRAY);
	Draw_Pixel(x+9,y+12, BLUE_GRAY);
	Draw_Pixel(x+8,y+12, BLUE_GRAY);
	Draw_Pixel(x+7,y+12,BLUE_GRAY);
	Draw_Pixel(x+6,y+12, BLUE_GRAY);
	Draw_Pixel(x+5,y+12,BLUE_GRAY);
	Draw_Pixel(x+4, y+12, BLUE_GRAY);
	Draw_Pixel(x+3,y+12, WHITE);
	Draw_Pixel(x+2, y+12,BLUE_GRAY);
	

	Draw_Pixel(x+14,y+13, BLUE_GRAY);
	Draw_Pixel(x+13,y+13, BLUE_GRAY);	
	Draw_Pixel(x+12,y+13, BLUE_GRAY);
	Draw_Pixel(x+11,y+13, BLUE_GRAY);
	Draw_Pixel(x+10,y+13,BLUE_GRAY);
	Draw_Pixel(x+9,y+13, BLUE_GRAY);
	Draw_Pixel(x+8,y+13, BLUE_GRAY);
	Draw_Pixel(x+7,y+13,BLUE_GRAY);
	Draw_Pixel(x+6,y+13, BLUE_GRAY);
	Draw_Pixel(x+5,y+13,BLUE_GRAY);
	Draw_Pixel(x+4,y+13,WHITE);
	Draw_Pixel(x+3,y+13, WHITE);
	Draw_Pixel(x+2,y+13,BLUE_GRAY);

	Draw_Pixel(x+14,y+14, BLUE_GRAY);
	Draw_Pixel(x+13,y+14, BLUE_GRAY);	
	Draw_Pixel(x+12,y+14, BLUE_GRAY);
	Draw_Pixel(x+11,y+14, BLUE_GRAY);
	Draw_Pixel(x+10,y+14,BLUE_GRAY);
	Draw_Pixel(x+9,y+14, BLUE_GRAY);
	Draw_Pixel(x+8,y+14, BLUE_GRAY);
	Draw_Pixel(x+7,y+14,BLUE_GRAY);
	Draw_Pixel(x+6,y+14, BLUE_GRAY);
	Draw_Pixel(x+5,y+14,WHITE);
	Draw_Pixel(x+4,y+14,WHITE);
	Draw_Pixel(x+3,y+14, BLUE_GRAY);
	Draw_Pixel(x+2,y+14,BLUE_GRAY);

	
	Draw_Pixel(x+12,y+15, BLUE_GRAY);
	Draw_Pixel(x+11,y+15, BLUE_GRAY);
	Draw_Pixel(x+10,y+15,BLUE_GRAY);
	Draw_Pixel(x+9,y+15, BLUE_GRAY);
	Draw_Pixel(x+8,y+15, BLUE_GRAY);
	Draw_Pixel(x+7,y+15,BLUE_GRAY);
	Draw_Pixel(x+6,y+15, BLUE_GRAY);
	Draw_Pixel(x+5,y+15,BLUE_GRAY);
	Draw_Pixel(x+4,y+15,BLUE_GRAY);
	
	
	


}
//
void sunFlower(uint16_t x, uint16_t y){

	Draw_Pixel(x+11, y+0, DARK_GREEN);
	Draw_Pixel(x+10, y+0, DARK_GREEN);
	Draw_Pixel(x+9, y+0, DARK_GREEN);
	Draw_Pixel(x+8, y+0, DARK_GREEN);
	Draw_Pixel(x+7, y+0, DARK_GREEN);
	Draw_Pixel(x+6, y+0, DARK_GREEN);

	Draw_Pixel(x+10, y+1, DARK_GREEN);
	Draw_Pixel(x+9, y+1, DARK_GREEN);
	Draw_Pixel(x+8, y+1, DARK_GREEN);
	Draw_Pixel(x+7, y+1, DARK_GREEN);

	Draw_Pixel(x+13, y+2, BLACK);
	Draw_Pixel(x+12, y+2, BLACK);
	Draw_Pixel(x+9, y+2, DARK_GREEN);
	Draw_Pixel(x+8, y+2, DARK_GREEN);
	Draw_Pixel(x+5, y+2, BLACK);
	Draw_Pixel(x+4, y+2, BLACK);

	Draw_Pixel(x+14, y+3, BLACK);
	Draw_Pixel(x+13, y+3,YELLOW);
	Draw_Pixel(x+12, y+3,YELLOW);
	Draw_Pixel(x+11, y+3, BLACK);
	Draw_Pixel(x+9, y+3, DARK_GREEN);
	Draw_Pixel(x+8, y+3, DARK_GREEN);
	Draw_Pixel(x+6, y+3, BLACK);
	Draw_Pixel(x+5, y+3,YELLOW);
	Draw_Pixel(x+4, y+3,YELLOW);
	Draw_Pixel(x+3, y+3, BLACK);

	Draw_Pixel(x+13, y+4, BLACK);
	Draw_Pixel(x+12, y+4,YELLOW);
	Draw_Pixel(x+11, y+4,YELLOW);
	Draw_Pixel(x+10, y+4, BLACK);
	Draw_Pixel(x+10, y+4, DARK_GREEN);
	Draw_Pixel(x+9, y+4, DARK_GREEN);
	Draw_Pixel(x+7, y+4, BLACK);
	Draw_Pixel(x+6, y+4,YELLOW);
	Draw_Pixel(x+5, y+4,YELLOW);
	Draw_Pixel(x+4, y+4, BLACK);

	Draw_Pixel(x+15, y+5, BLACK);
	Draw_Pixel(x+14, y+5, BLACK);
	Draw_Pixel(x+12, y+5, BLACK);
	Draw_Pixel(x+11, y+5, BLACK);
	Draw_Pixel(x+10, y+5, BLACK);
	Draw_Pixel(x+9, y+5, BLACK);
	Draw_Pixel(x+8, y+5, BLACK);
	Draw_Pixel(x+7, y+5, BLACK);
	Draw_Pixel(x+6, y+5, BLACK);
	Draw_Pixel(x+5, y+5, BLACK);
	Draw_Pixel(x+3, y+5, BLACK);
	Draw_Pixel(x+2, y+5, BLACK);

	Draw_Pixel(x+16, y+6, BLACK);
	Draw_Pixel(x+15, y+6, YELLOW);
	Draw_Pixel(x+14, y+6, YELLOW);
	Draw_Pixel(x+13, y+6, BLACK);
	Draw_Pixel(x+12, y+6, ORANGE);
	Draw_Pixel(x+11, y+6, ORANGE);
	Draw_Pixel(x+10, y+6, ORANGE);
	Draw_Pixel(x+9, y+6, ORANGE);
	Draw_Pixel(x+8, y+6, ORANGE);
	Draw_Pixel(x+7, y+6, ORANGE);
	Draw_Pixel(x+6, y+6, ORANGE);
	Draw_Pixel(x+5, y+6, ORANGE);
	Draw_Pixel(x+4, y+6, BLACK);
	Draw_Pixel(x+3, y+6, YELLOW);
	Draw_Pixel(x+2, y+6, YELLOW);
	Draw_Pixel(x+1, y+6, BLACK);


	Draw_Pixel(x+15, y+7, BLACK);
	Draw_Pixel(x+14, y+7, BLACK);
	Draw_Pixel(x+13, y+7, ORANGE);
	Draw_Pixel(x+12, y+7, ORANGE);
	Draw_Pixel(x+11, y+7, ORANGE);
	Draw_Pixel(x+10, y+7, ORANGE);
	Draw_Pixel(x+9, y+7, BLACK);
	Draw_Pixel(x+8, y+7, BLACK);
	Draw_Pixel(x+7, y+7, ORANGE);
	Draw_Pixel(x+6, y+7, ORANGE);
	Draw_Pixel(x+5, y+7, ORANGE);
	Draw_Pixel(x+4, y+7, ORANGE);
	Draw_Pixel(x+3, y+7, ORANGE);
	Draw_Pixel(x+2, y+7, BLACK);
	Draw_Pixel(x+1, y+7, BLACK);

	Draw_Pixel(x+16, y+8, BLACK);
	Draw_Pixel(x+15, y+8, YELLOW);
	Draw_Pixel(x+14, y+8, BLACK);
	Draw_Pixel(x+13, y+8, ORANGE);
	Draw_Pixel(x+12, y+8, ORANGE);
	Draw_Pixel(x+11, y+8, ORANGE);
	Draw_Pixel(x+10, y+8, ORANGE);
	Draw_Pixel(x+9, y+8, ORANGE);
	Draw_Pixel(x+8, y+8, ORANGE);
	Draw_Pixel(x+7, y+8, ORANGE);
	Draw_Pixel(x+6, y+8, ORANGE);
	Draw_Pixel(x+5, y+8, ORANGE);
	Draw_Pixel(x+4, y+8, ORANGE);
	Draw_Pixel(x+3, y+8, BLACK);
	Draw_Pixel(x+2, y+8, YELLOW);
	Draw_Pixel(x+1, y+8, BLACK);

	Draw_Pixel(x+16, y+9, BLACK);
	Draw_Pixel(x+15, y+9, YELLOW);
	Draw_Pixel(x+14, y+9, BLACK);
	Draw_Pixel(x+13, y+9, ORANGE);
	Draw_Pixel(x+12, y+9, ORANGE);
	Draw_Pixel(x+11, y+9, BLACK);
	Draw_Pixel(x+10, y+9, ORANGE);
	Draw_Pixel(x+9, y+9, ORANGE);
	Draw_Pixel(x+8, y+9, ORANGE);
	Draw_Pixel(x+7, y+9, ORANGE);
	Draw_Pixel(x+6, y+9, BLACK);
	Draw_Pixel(x+5, y+9, ORANGE);
	Draw_Pixel(x+4, y+9, ORANGE);
	Draw_Pixel(x+3, y+9, BLACK);
	Draw_Pixel(x+2, y+9, YELLOW);
	Draw_Pixel(x+1, y+9, BLACK);

	
	Draw_Pixel(x+16, y+10, BLACK);
	Draw_Pixel(x+15, y+10, YELLOW);
	Draw_Pixel(x+14, y+10, BLACK);
	Draw_Pixel(x+13, y+10, ORANGE);
	Draw_Pixel(x+12, y+10, ORANGE);
	Draw_Pixel(x+11, y+10, ORANGE);
	Draw_Pixel(x+10, y+10, ORANGE);
	Draw_Pixel(x+9, y+10, ORANGE);
	Draw_Pixel(x+8, y+10, ORANGE);
	Draw_Pixel(x+7, y+10, ORANGE);
	Draw_Pixel(x+6, y+10, ORANGE);
	Draw_Pixel(x+5, y+10, ORANGE);
	Draw_Pixel(x+4, y+10, ORANGE);
	Draw_Pixel(x+3, y+10, BLACK);
	Draw_Pixel(x+2, y+10, YELLOW);
	Draw_Pixel(x+1, y+10, BLACK);

	Draw_Pixel(x+15, y+11, BLACK);
	Draw_Pixel(x+14, y+11, YELLOW);
	Draw_Pixel(x+12, y+11, BLACK);
	Draw_Pixel(x+11, y+11, ORANGE);
	Draw_Pixel(x+10, y+11, ORANGE);
	Draw_Pixel(x+9, y+11, ORANGE);
	Draw_Pixel(x+8, y+11, ORANGE);
	Draw_Pixel(x+7, y+11, ORANGE);
	Draw_Pixel(x+6, y+11, ORANGE);
	Draw_Pixel(x+5, y+11, BLACK);
	Draw_Pixel(x+3, y+11, YELLOW);
	Draw_Pixel(x+2, y+11, BLACK);

	Draw_Pixel(x+15, y+12, BLACK);
	Draw_Pixel(x+14, y+12, YELLOW);
	Draw_Pixel(x+12, y+12, YELLOW);
	Draw_Pixel(x+11, y+12, BLACK);
	Draw_Pixel(x+10, y+12, BLACK);
	Draw_Pixel(x+9, y+12, BLACK);
	Draw_Pixel(x+8, y+12, BLACK);
	Draw_Pixel(x+7, y+12, BLACK);
	Draw_Pixel(x+6, y+12, BLACK);
	Draw_Pixel(x+5, y+12, YELLOW);
	Draw_Pixel(x+3, y+12, YELLOW);
	Draw_Pixel(x+2, y+12, BLACK);

	Draw_Pixel(x+15, y+13, BLACK);
	Draw_Pixel(x+14, y+13, YELLOW);
	Draw_Pixel(x+13, y+13, BLACK);
	Draw_Pixel(x+12, y+13, YELLOW);
	Draw_Pixel(x+11, y+13, YELLOW);
	Draw_Pixel(x+10, y+13, YELLOW);
	Draw_Pixel(x+9, y+13, YELLOW);
	Draw_Pixel(x+8, y+13, BLACK);
	Draw_Pixel(x+7, y+13, YELLOW);
	Draw_Pixel(x+6, y+13, YELLOW);
	Draw_Pixel(x+5, y+13, YELLOW);
	Draw_Pixel(x+4, y+13, BLACK);
	Draw_Pixel(x+3, y+13, YELLOW);
	Draw_Pixel(x+2, y+13, BLACK);

	
	Draw_Pixel(x+14, y+14, BLACK);
	Draw_Pixel(x+12, y+14, BLACK);
	Draw_Pixel(x+11, y+14, BLACK);
	Draw_Pixel(x+10, y+14, YELLOW);
	Draw_Pixel(x+9, y+14, BLACK);
	Draw_Pixel(x+7, y+14, BLACK);
	Draw_Pixel(x+6, y+14, YELLOW);
	Draw_Pixel(x+5, y+14, BLACK);
	Draw_Pixel(x+3, y+14, BLACK);

	Draw_Pixel(x+10, y+15, BLACK);
	Draw_Pixel(x+6, y+15, BLACK);
	
	
	





	







}

void peashooter(uint16_t x, uint16_t y){
Draw_Pixel(x+16, y, 0x000000);
Draw_Pixel(x+5, y, 0x000000);

Draw_Pixel(x+15, 1+y, 0x000000);
Draw_Pixel(x+6, 1+y, 0x000000);
Draw_Pixel(x+7, 1+y, 0x000000);

Draw_Pixel(x+8, 2+y, 0x000000);
Draw_Pixel(x+14, 2+y, 0x000000);

Draw_Pixel(x+8, 3+y, 0x000000);
Draw_Pixel(x+9, 3+y, 0x000000);
Draw_Pixel(x+10, 3+y, 0x000000);
Draw_Pixel(x+11, 3+y, 0x000000);
Draw_Pixel(x+12, 3+y, 0x000000);

Draw_Pixel(x+12, 4+y, 0x000000);
Draw_Pixel(x+12, 5+y, 0x000000);

Draw_Pixel(x+13, 6+y, 0x000000);
Draw_Pixel(x+11, 6+y, 0x000000);
Draw_Pixel(x+10, 6+y, 0x000000);
Draw_Pixel(x+8, 6+y, 0x008000);
Draw_Pixel(x+7, 6+y, 0x008000);
Draw_Pixel(x+6, 6+y, 0x008000);

Draw_Pixel(x+14, 7+y, 0x000000);
Draw_Pixel(x+12, 7+y, 0x000000);
Draw_Pixel(x+8, 7+y, 0x008000);
Draw_Pixel(x+7, 7+y, 0x90EE90);
Draw_Pixel(x+6, 7+y, 0x90EE90);
Draw_Pixel(x+5, 7+y, 0x90EE90);
Draw_Pixel(x+4, 7+y, 0x008000);
Draw_Pixel(x+3, 7+y, 0x008000);
Draw_Pixel(x+2, 7+y, 0x008000);

Draw_Pixel(x+13, 8+y, 0x000000);
Draw_Pixel(x+9, 8+y, 0x008000);
Draw_Pixel(x+8, 8+y, 0x90EE90);
Draw_Pixel(x+7, 8+y, 0x90EE90);
Draw_Pixel(x+6, 8+y, 0x90EE90);
Draw_Pixel(x+5, 8+y, 0x008000);
Draw_Pixel(x+4, 8+y, 0x000000);
Draw_Pixel(x+3, 8+y, 0x000000);
Draw_Pixel(x+2, 8+y, 0x000000);
Draw_Pixel(x+1, 8+y, 0x008000);

Draw_Pixel(x+14, 9+y, 0x000000);
Draw_Pixel(x+12, 9+y, 0x000000);
Draw_Pixel(x+10, 9+y, 0x008000);
Draw_Pixel(x+9, 9+y, 0x90EE90);
Draw_Pixel(x+8, 9+y, 0x90EE90);
Draw_Pixel(x+7, 9+y, 0x90EE90);
Draw_Pixel(x+6, 9+y, 0x90EE90);
Draw_Pixel(x+5, 9+y, 0x008000);
Draw_Pixel(x+4, 9+y, 0x000000);
Draw_Pixel(x+3, 9+y, 0x000000);
Draw_Pixel(x+2, 9+y, 0x000000);
Draw_Pixel(x+1, 9+y, 0x008000);

Draw_Pixel(x+14, 10+y, 0x000000);
Draw_Pixel(x+11, 10+y, 0x000000);
Draw_Pixel(x+10, 10+y, 0x008000);
Draw_Pixel(x+9, 10+y, 0x90EE90);
Draw_Pixel(x+8, 10+y, 0x90EE90);
Draw_Pixel(x+7, 10+y, 0x90EE90);
Draw_Pixel(x+6, 10+y, 0x90EE90);
Draw_Pixel(x+5, 10+y, 0x008000);
Draw_Pixel(x+4, 10+y, 0x000000);
Draw_Pixel(x+3, 10+y, 0x000000);
Draw_Pixel(x+2, 10+y, 0x000000);
Draw_Pixel(x+1, 10+y, 0x008000);


Draw_Pixel(x+14, 11+y, 0x000000);
Draw_Pixel(x+10, 11+y, 0x008000);
Draw_Pixel(x+10, 11+y, 0x008000);
Draw_Pixel(x+9, 11+y, 0x90EE90);
Draw_Pixel(x+8, 11+y, 0x90EE90);
Draw_Pixel(x+7, 11+y, 0x000000);
Draw_Pixel(x+6, 11+y, 0x90EE90);
Draw_Pixel(x+5, 11+y, 0x90EE90);
Draw_Pixel(x+4, 11+y, 0x008000);
Draw_Pixel(x+3, 11+y, 0x008000);
Draw_Pixel(x+2, 11+y, 0x008000);



Draw_Pixel(x+16, 12+y, 0x000000);
Draw_Pixel(x+9, 12+y, 0x008000);
Draw_Pixel(x+8, 12+y, 0x90EE90);
Draw_Pixel(x+7, 12+y, 0x90EE90);
Draw_Pixel(x+6, 12+y, 0x90EE90);

Draw_Pixel(x+8, 13+y, 0x008000);
Draw_Pixel(x+7, 13+y, 0x008000);
Draw_Pixel(x+6, 13+y, 0x008000);









}


void peaspawn(uint16_t row, uint16_t col){
	uint16_t i = 0;


	for(i = 0; i < SHOTS; i++){ //finds the first available peashooter slot
		if(!PEA[i].peashooters){
			PEA[i].x = (col * 18);
            PEA[i].y = 32 + (row * 32) + 8;
            PEA[i].row = row;
            PEA[i].peashooters = 1;
			break;
		}
	}
}
void pea(){
	uint16_t i = 0;
	uint16_t j = 0;

	for(i = 0; i < SHOTS; i++){
		if(PEA[i].peashooters){
			
			PEA[i].x-=2;
			Draw_Pixel(PEA[i].x, PEA[i].y, BLACK);
			if(PEA[i].x < 2){
				PEA[i].peashooters = 0;
				continue;
			}
			for(j = 0; j < ZOMBIES; j++){
				if(!ZOMBIE_ARRAY[j].zombies){
					continue;
				}				
				if(PEA[i].row == ZOMBIE_ARRAY[j].row && PEA[i].x <= 
					ZOMBIE_ARRAY[j].x + 30 &&  ZOMBIE_ARRAY[j].x <=  + PEA[i].x + 30)
					{
						
						PEA[i].peashooters = 0;
						if(ZOMBIE_ARRAY[j].health >= 20){
							ZOMBIE_ARRAY[j].health -= 20;
						}
						else{
							ZOMBIE_ARRAY[j].health = 0;
					}
					if(ZOMBIE_ARRAY[j].health == 0){
						ZOMBIE_ARRAY[j].zombies = 0;
						fillRectangle(ZOMBIE_ARRAY[j].x, ZOMBIE_ARRAY[j].y, 18, 30, BLACK);
					}
					break;
				}
			}

			
			if(PEA[i].peashooters){
			Draw_Pixel(PEA[i].x, PEA[i].y, DARK_GREEN);
			}
		}	
	}
}
void wallnut(uint16_t x, uint16_t y){

	Draw_Pixel(x+5, 0+y, 0x654321 );
	Draw_Pixel(x+6, 0+y, 0x654321 );
	Draw_Pixel(x+7, 0+y, 0x654321 );
	Draw_Pixel(x+8, 0+y, 0x654321 );
	Draw_Pixel(x+9, 0+y, 0x654321 );
	Draw_Pixel(x+10, 0+y, 0x654321 );
	Draw_Pixel(x+11, 0+y, 0x654321 );
	Draw_Pixel(x+12, 0+y, 0x654321 );
	Draw_Pixel(x+3, 1+y, 0x654321 );
	Draw_Pixel(x+4,1+y,0xA52A2A);
	Draw_Pixel(x+5,1+y,0xA52A2A);
	Draw_Pixel(x+6,1+y,0xA52A2A);
	Draw_Pixel(x+7,1+y,0xA52A2A);
	Draw_Pixel(x+8,1+y,0xA52A2A);
	Draw_Pixel(x+9,1+y,0xA52A2A);
	Draw_Pixel(x+10,1+y,0xA52A2A);
	Draw_Pixel(x+11,1+y,0xA52A2A);
	Draw_Pixel(x+12,1+y,0xA52A2A);
	Draw_Pixel(x+13, 1+y, 0x654321 );
	Draw_Pixel(x+3, 2+y, 0x654321 );
	Draw_Pixel(x+4, 2+y, 0xA52A2A );
	Draw_Pixel(x+5, 2+y, 0xA52A2A );
	Draw_Pixel(x+6, 2+y, 0xA52A2A );
	Draw_Pixel(x+7, 2+y, 0xA52A2A );
	Draw_Pixel(x+8, 2+y, 0xA52A2A );
	Draw_Pixel(x+9, 2+y, 0xA52A2A );
	Draw_Pixel(x+10, 2+y, 0xA52A2A );
	Draw_Pixel(x+11, 2+y, 0xA52A2A );
	Draw_Pixel(x+12, 2+y, 0xA52A2A );
	Draw_Pixel(x+13, 2+y, 0xA52A2A );
	Draw_Pixel(x+14, 2+y, 0x654321 );
	Draw_Pixel(x+2,3+y,0x654321);
	Draw_Pixel(x+3,3+y,0xA52A2A);
	Draw_Pixel(x+4,3+y,0xA52A2A);
	Draw_Pixel(x+5,3+y,0xA52A2A);
	Draw_Pixel(x+6,3+y,0xA52A2A);
	Draw_Pixel(x+7,3+y,0x000000);
	Draw_Pixel(x+8,3+y,0x000000);
	Draw_Pixel(x+9,3+y,0x000000);
	Draw_Pixel(x+10,3+y,0x000000);
	Draw_Pixel(x+11,3+y,0xA52A2A);
	Draw_Pixel(x+12,3+y,0xA52A2A);
	Draw_Pixel(x+13,3+y,0xA52A2A);
	Draw_Pixel(x+14,3+y,0xA52A2A);
	Draw_Pixel(x+15,3+y,0x654321);
	Draw_Pixel(x+2,4+y,0x654321);
	Draw_Pixel(x+3,4+y,0xA52A2A);
	Draw_Pixel(x+4,4+y,0xA52A2A);
	Draw_Pixel(x+5,4+y,0xA52A2A);
	Draw_Pixel(x+6,4+y,0xA52A2A);
	Draw_Pixel(x+7,4+y,0x000000);
	Draw_Pixel(x+8,4+y,0x000000);
	Draw_Pixel(x+9,4+y,0x000000);
	Draw_Pixel(x+10,4+y,0x000000);
	Draw_Pixel(x+11,4+y,0xA52A2A);
	Draw_Pixel(x+12,4+y,0xA52A2A);
	Draw_Pixel(x+13,4+y,0xA52A2A);
	Draw_Pixel(x+14,4+y,0xA52A2A);
	Draw_Pixel(x+15,4+y,0x654321);
	Draw_Pixel(x+2,5+y,0x654321);
	Draw_Pixel(x+3,5+y,0xA52A2A);
	Draw_Pixel(x+4,5+y,0xA52A2A);
	Draw_Pixel(x+5,5+y,0xA52A2A);
	Draw_Pixel(x+6,5+y,0xA52A2A);
	Draw_Pixel(x+7,5+y,0xA52A2A);
	Draw_Pixel(x+8,5+y,0xA52A2A);
	Draw_Pixel(x+9,5+y,0xA52A2A);
	Draw_Pixel(x+10,5+y,0xA52A2A);
	Draw_Pixel(x+11,5+y,0xA52A2A);
	Draw_Pixel(x+12,5+y,0xA52A2A);
	Draw_Pixel(x+13,5+y,0xA52A2A);
	Draw_Pixel(x+14,5+y,0xA52A2A);
	Draw_Pixel(x+15,5+y,0x654321);
	Draw_Pixel(x+2,6+y,0x654321);
	Draw_Pixel(x+3,6+y,0xA52A2A);
	Draw_Pixel(x+4,6+y,0xA52A2A);
	Draw_Pixel(x+5,6+y,0xA52A2A);
	Draw_Pixel(x+6,6+y,0xA52A2A);
	Draw_Pixel(x+7,6+y,0xA52A2A);
	Draw_Pixel(x+8,6+y,0xA52A2A);
	Draw_Pixel(x+9,6+y,0xA52A2A);
	Draw_Pixel(x+10,6+y,0xA52A2A);
	Draw_Pixel(x+11,6+y,0xA52A2A);
	Draw_Pixel(x+12,6+y,0xA52A2A);
	Draw_Pixel(x+13,6+y,0xA52A2A);
	Draw_Pixel(x+14,6+y,0xA52A2A);
	Draw_Pixel(x+15,6+y,0x654321);
	Draw_Pixel(x+1,7+y,0x654321);
	Draw_Pixel(x+2,7+y,0xA52A2A);
	Draw_Pixel(x+3,7+y,0xA52A2A);
	Draw_Pixel(x+4,7+y,0xA52A2A);
	Draw_Pixel(x+5,7+y,0xA52A2A);
	Draw_Pixel(x+6,7+y,0xA52A2A);
	Draw_Pixel(x+7,7+y,0xA52A2A);
	Draw_Pixel(x+8,7+y,0xA52A2A);
	Draw_Pixel(x+9,7+y,0xA52A2A);
	Draw_Pixel(x+10,7+y,0xA52A2A);
	Draw_Pixel(x+11,7+y,0xA52A2A);
	Draw_Pixel(x+12,7+y,0xA52A2A);
	Draw_Pixel(x+13,7+y,0xA52A2A);
	Draw_Pixel(x+14,7+y,0xA52A2A);
	Draw_Pixel(x+15,7+y,0xA52A2A);
	Draw_Pixel(x+16,7+y,0x654321);
	Draw_Pixel(x+2,8+y,0x654321);
	Draw_Pixel(x+3,8+y,0xA52A2A);
	Draw_Pixel(x+4,8+y,0xA52A2A);
	Draw_Pixel(x+5,8+y,0x000000);
	Draw_Pixel(x+6,8+y,0x000000);
	Draw_Pixel(x+7,8+y,0xA52A2A);
	Draw_Pixel(x+8,8+y,0xA52A2A);
	Draw_Pixel(x+9,8+y,0xA52A2A);
	Draw_Pixel(x+10,8+y,0xA52A2A);
	Draw_Pixel(x+11,8+y,0x000000);
	Draw_Pixel(x+12,8+y,0x000000);
	Draw_Pixel(x+13,8+y,0xA52A2A);
	Draw_Pixel(x+14,8+y,0xA52A2A);
	Draw_Pixel(x+15,8+y,0x654321);
	Draw_Pixel(x+3,9+y,0xA52A2A);
	Draw_Pixel(x+2,9+y,0x654321);
	Draw_Pixel(x+3,9+y,0xA52A2A);
	Draw_Pixel(x+4,9+y,0xA52A2A);
	Draw_Pixel(x+5,9+y,0x000000);
	Draw_Pixel(x+6,9+y,0x000000);
	Draw_Pixel(x+7,9+y,0xA52A2A);
	Draw_Pixel(x+8,9+y,0xA52A2A);
	Draw_Pixel(x+9,9+y,0xA52A2A);
	Draw_Pixel(x+10,9+y,0xA52A2A);
	Draw_Pixel(x+11,9+y,0x000000);
	Draw_Pixel(x+12,9+y,0x000000);
	Draw_Pixel(x+13,9+y,0xA52A2A);
	Draw_Pixel(x+14,9+y,0xA52A2A);
	Draw_Pixel(x+15,9+y,0x654321);
	Draw_Pixel(x+2,10+y,0x654321);
	Draw_Pixel(x+3,10+y,0xA52A2A);
	Draw_Pixel(x+4,10+y,0xA52A2A);
	Draw_Pixel(x+5,10+y,0xA52A2A);
	Draw_Pixel(x+6,10+y,0xA52A2A);
	Draw_Pixel(x+7,10+y,0xA52A2A);
	Draw_Pixel(x+8,10+y,0xA52A2A);
	Draw_Pixel(x+9,10+y,0xA52A2A);
	Draw_Pixel(x+10,10+y,0xA52A2A);
	Draw_Pixel(x+11,10+y,0xA52A2A);
	Draw_Pixel(x+12,10+y,0xA52A2A);
	Draw_Pixel(x+13,10+y,0xA52A2A);
	Draw_Pixel(x+14,10+y,0xA52A2A);
	Draw_Pixel(x+15,10+y,0x654321);
	Draw_Pixel(x+3, 11+y, 0x654321 );
	Draw_Pixel(x+4, 11+y, 0xA52A2A );
	Draw_Pixel(x+5, 11+y, 0xA52A2A );
	Draw_Pixel(x+6, 11+y, 0xA52A2A );
	Draw_Pixel(x+7, 11+y, 0xA52A2A );
	Draw_Pixel(x+8, 11+y, 0xA52A2A );
	Draw_Pixel(x+9, 11+y, 0xA52A2A );
	Draw_Pixel(x+10, 11+y, 0xA52A2A );
	Draw_Pixel(x+11, 11+y, 0xA52A2A );
	Draw_Pixel(x+12, 11+y, 0xA52A2A );
	Draw_Pixel(x+3, 11+y, 0xA52A2A );
	Draw_Pixel(x+14, 11+y, 0x654321 );
	Draw_Pixel(x+3, 12+y, 0x654321 );
	Draw_Pixel(x+4,12+y,0xA52A2A);
	Draw_Pixel(x+5,12+y,0xA52A2A);
	Draw_Pixel(x+6,12+y,0xA52A2A);
	Draw_Pixel(x+7,12+y,0xA52A2A);
	Draw_Pixel(x+8,12+y,0xA52A2A);
	Draw_Pixel(x+9,12+y,0xA52A2A);
	Draw_Pixel(x+10,12+y,0xA52A2A);
	Draw_Pixel(x+11,12+y,0xA52A2A);
	Draw_Pixel(x+12,12+y,0xA52A2A);
	Draw_Pixel(x+13, 12+y, 0x654321 );
	Draw_Pixel(x+5, 13+y, 0x654321 );
	Draw_Pixel(x+6, 13+y, 0x654321 );
	Draw_Pixel(x+7, 13+y, 0x654321 );
	Draw_Pixel(x+8, 13+y, 0x654321 );
	Draw_Pixel(x+9, 13+y, 0x654321 );
	Draw_Pixel(x+10, 13+y, 0x654321 );
	Draw_Pixel(x+11, 13+y, 0x654321 );
	Draw_Pixel(x+12, 13+y, 0x654321 );


}

void drawZombies(uint16_t x, uint16_t y, uint8_t level){
	if(level == 1){ // this is selecting which zombie to draw based on the level
		zombie(x,y);
	}
	else if(level == 2){
		secVariant(x,y);
	}
	else if(level == 3){
		fastVar(x,y);
	}
}

void spawn(uint16_t row, uint8_t level){
	uint16_t i = 0;

	for(i = 0; i < ZOMBIES; i++){ // the first available zombie slot is found and the zombie is spawned there
		if(!ZOMBIE_ARRAY[i].zombies){
			ZOMBIE_ARRAY[i].x = 0;
			ZOMBIE_ARRAY[i].y = 32 + (row * 32);
			ZOMBIE_ARRAY[i].row = row;

			ZOMBIE_ARRAY[i].zombies = 1;
		if(level == 1){
			ZOMBIE_ARRAY[i].health = 100;
		}
		else if(level == 2){
			ZOMBIE_ARRAY[i].health = 200;
		}
		else if(level == 3){
			ZOMBIE_ARRAY[i].health = 100;
		}
			break;
		}
	}
}

void moveZombie(uint8_t level, uint32_t color){
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;
	for(i = 0; i < ZOMBIES; i++){ // how the zombies move and interact with the wallnuts and the end of the screen
		if(!ZOMBIE_ARRAY[i].zombies)
		continue;

				fillRectangle(ZOMBIE_ARRAY[i].x, ZOMBIE_ARRAY[i].y, 18, 30, color);
			if(level == 1){
				ZOMBIE_ARRAY[i].x+=2;
			}
			else if(level == 2){
				ZOMBIE_ARRAY[i].x+=2;
			}
			else if(level == 3){
				ZOMBIE_ARRAY[i].x+=8;
			}
			if(ZOMBIE_ARRAY[i].x > 150){
				ZOMBIE_ARRAY[i].zombies = 0;
				gamestate = 4;
				continue;
			}
			j = ZOMBIE_ARRAY[i].x / 18;
			k = ZOMBIE_ARRAY[i].row;
			if(j < 7 && (grid[k][j] == 3 || grid[k][j] == 4 || grid[k][j] == 5)){
				ZOMBIE_ARRAY[i].x = j * 18 - 2; // pin it to the wallnt
				if(plant_health[k][j] >= 10){ // when the wallnut is attacked it will lose 10 health until it is destroyed
					plant_health[k][j] -= 10;
				}
				else{
					plant_health[k][j] = 0;
				}
				if(plant_health[k][j] == 0){
					grid[k][j] = 0;
					fillRectangle(j*18, 32 + (k*32), 18, 32, color);
				}
			}
			drawZombies(ZOMBIE_ARRAY[i].x, ZOMBIE_ARRAY[i].y, level);
			}
		}


void drawplants(unsigned char row, unsigned char column, unsigned char planttype){

	uint16_t x = column * 18;
	uint16_t y = 32 + (row *32);


	if(planttype == 3){
		wallnut(x,y);
	}
	else if(planttype == 4){
		peashooter(x,y);
	}
	else if(planttype ==5){
		sunFlower(x,y);
	}


}

void drawplantgrid(){
	static unsigned char last_grid[3][7] = {0};
	unsigned char r = 0;
	unsigned char c = 0;
	
        for(r = 0; r < 3; r++){
                for(c = 0; c < 7; c++){
                if(grid[r][c] != 0){
                        drawplants(r,c,grid[r][c]);
						last_grid[r][c] = grid[r][c];
                }
                }
        }


}


// sun count
void drawSunCount(unsigned char count){
	    static unsigned char last_count = 255;
    if(count != last_count){ 
        fillRectangle(0, 0, 128, 32, BLACK);
        drawNumber(98, 0 , count, YELLOW);
        last_count = count;
    }

}

//these are the colors
void lawn_grid(uint32_t color1, uint32_t color2, unsigned char cur_col, unsigned char cur_row, unsigned char redraw){
	static unsigned char last_col = 0xFF;
	static unsigned char last_row = 0xFF;
	
	if(redraw){
		fillRectangle(0,0,160,128, color1);
		fillRectangle(0,0, 160,32,color2);
		fillRectangle(0,128, 160, 32, WHITE);
		drawRectBorder(0, 128, 30, 32, BLACK);
		drawRectBorder(25, 128, 30, 32, BLACK);
		wallnut(57,128);
		drawRectBorder(50, 128, 30, 32, BLACK);
		peashooter(78,128);
		drawRectBorder(75, 128, 30, 32, BLACK);
		sunFlower(106,128);
		drawRectBorder(100, 128, 30, 32, BLACK);
		fillRectangle(0,0, 128,32, BLACK);
		
		for(uint8_t row = 0; row < 3; row++) {
			for(uint8_t col = 0; col < 7; col++) {
				uint16_t x = 0 + (col * 18);
				uint16_t y = 32 + (row * 32);
				drawRectBorder(x, y, 18, 32, color2);
				
			}
		}
		last_col = 0xFF;
		last_row = 0xFF;
	}

	// Cursor
	if((last_col != cur_col || last_row != cur_row) && last_col != 0xFF){
		uint16_t old_x = last_col * 18;
		uint16_t old_y = 32 + (last_row * 32);
		fillRectangle(old_x, old_y, 18, 32, color1);
		drawRectBorder(old_x, old_y, 18, 32, color2);
	}

	drawplantgrid();
	

	uint16_t x = cur_col * 18;
	uint16_t y = 32 + (cur_row * 32);
	drawRectBorder(x, y, 18, 32, WHITE);
	last_col = cur_col;
	last_row = cur_row;
	// when the joystick is not being movedd tha game is paused
	pea();
	if(gamestate == 1){
		moveZombie(gamestate, GREEN);
	}
	else if(gamestate == 2){
		moveZombie(gamestate, DARK_PURPLE);
	}
	else if(gamestate == 3){
		moveZombie(gamestate, BLUE);
	}
	drawSunCount(sun);
}

#endif /* SPIAVR_H */
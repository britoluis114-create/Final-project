/*         Luis & lbrit005@ucr.edu:
            Discussion: 023
 *         Assignment: Custom project part 1
 *         Exercise Description: Plants vs Zombies game with TFT display, joystick control, IR remote
 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.
 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "spiAVR.h"
#include "irAVR.h"
#include "LCD.h"

volatile unsigned int adcHor = 0;
volatile unsigned int adcVer = 0;
volatile unsigned char adcBut = 0;
volatile unsigned char gamestate = 0;
volatile unsigned long remote_input;
volatile unsigned char remote_received = 0;
volatile unsigned char joy_button_pressed = 0;
volatile unsigned char joy_position = 0;
volatile unsigned char mouse_input = 0;
volatile unsigned char zombie_type = 0;
volatile unsigned char selected_plant = 0;
volatile unsigned char lcd_state = 0;
volatile unsigned char audio_state = 0;
volatile unsigned char render_state = 0;
volatile unsigned char sun_count = 0;
volatile unsigned long last_remote_code = 0;
volatile unsigned long gametimer = 0;
volatile unsigned char grid[4][8] = {{0}};
volatile unsigned char cursor_column = 0;
volatile unsigned char cursor_rows = 0;
volatile unsigned char moved;
volatile unsigned reset1 = 0;
volatile unsigned reset2 = 0;
volatile unsigned reset3 = 0;
volatile unsigned char menu = 0;
volatile unsigned long health = 0;
volatile unsigned char damage = 0;

decode_results ir_results;

#define BUTTON0 0XFF6897
#define BUTTON1 0xFF30CF        
#define BUTTON2 0xFF18E7        
#define BUTTON3 0xFF7A85   
#define BUTTON4 0xFF10EF

#define NUM_TASKS 6

typedef struct _task{
	signed char state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};
int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};

const unsigned long JoyStick_PERIOD = 50;
const unsigned long mouse_PERIOD = 50;
const unsigned long LCD_PERIOD = 1000;
const unsigned long RENDER_PERIOD = 50;
const unsigned long PLANT_PERIOD = 50;
const unsigned long MUSIC_PERIOD = 100;
const unsigned char ZOMBIE_PERIOD = 100;
const unsigned long GCD_PERIOD = 10;

task tasks[NUM_TASKS];

enum JOYSTICK_STATES{JOYSTICK_IDLE};
int TickFct_JoyStick(int state){
        adcHor = ADC_read(1);
        adcVer = ADC_read(3);
        adcBut = GetBit(PINC, 2);
        unsigned char button = GetBit(PINC, 0);
        static unsigned char prev_but = 0;
        static unsigned char pos = 1;
        static unsigned int temp_Hor = 512;
        static unsigned int temp_ver = 512;

        if(gamestate == 0){
		if(adcHor < 350 && temp_Hor >= 350){
			pos = 0;
			temp_Hor = adcHor;
		}
		else if(adcHor > 700 && temp_Hor <= 700){
			pos = 2;
			temp_Hor = adcHor;
		}
		else if(adcHor >= 350 && adcHor <= 700 && (temp_Hor < 350 || temp_Hor > 700)){
			pos = 1;
			temp_Hor = adcHor;
		}
	}

        if(gamestate>=1&& gamestate <= 3){
		if(adcHor <350 && temp_Hor >= 350){
			if(cursor_column < 7){
				cursor_column++;  
			}
			temp_Hor = adcHor;
		}
		else if(adcHor > 700 && temp_Hor <= 700){
			if(cursor_column > 0){
				cursor_column--;
			}
			temp_Hor = adcHor;
		}
		else if(adcHor >= 350 && adcHor <= 700 && (temp_Hor < 350 || temp_Hor > 700)){
			temp_Hor = adcHor;
		}   

		if(adcVer < 350 && temp_ver >= 350){
			if(cursor_rows > 0){
				cursor_rows--;
			}
			temp_ver = adcVer;
		}
		else if(adcVer > 700 && temp_ver <= 700){
			if(cursor_rows< 3){
				cursor_rows++;
			}
			temp_ver = adcVer;
		}
		else if(adcVer >= 350 && adcVer <= 700 && (temp_ver < 350 || temp_ver > 700)){
			temp_ver = adcVer;
		} 
        }

        joy_position = pos;

        if(button && !prev_but &&gamestate == 0){
		if(joy_position == 0){
			gamestate = 1;
		}
		else if(joy_position == 1){
			gamestate = 2;
		}
		else if(joy_position == 2){
			gamestate = 3;
		}
	}
        prev_but = button;

        return state;
}

enum RENDER_STATES{MENU, LEVEL1, LEVEL2, LEVEL3, RENDER_GAMEOVER};
int TickFct_Render(int state){
        static unsigned char last_col = 0xFF;
        static unsigned last_row = 0xFF;

        switch(state){
		case MENU:
			if(gamestate == 1){
				state = LEVEL1;
			}
			else if(gamestate == 2){
				state = LEVEL2;
			}
			else if(gamestate == 3){
				state = LEVEL3;
			}
			break;
		case LEVEL1:
			if(gamestate == 0){
				state = MENU;
			}
			else if(gamestate == 4){
				state = RENDER_GAMEOVER;
			}
			break;
		case LEVEL2:
			if(gamestate == 0){
				state = MENU;
			}
			else if(gamestate == 4){
				state = RENDER_GAMEOVER;
			}
			break;
		case LEVEL3:
			if(gamestate == 0){
				state = MENU;
			}
			else if(gamestate == 4){
				state = RENDER_GAMEOVER;
			}
			break;
		case RENDER_GAMEOVER:
			if(gamestate == 0){
				state = MENU;
			}
			break;
		default:
			break;
	}

        switch(state){
		case MENU:
			if(gamestate == 0 &&menu == 0){
				fillScreen(PURPLE);
				drawSentence(8,70, "NWAL", BLACK);
				drawSentence(50,70, "THGIN", BLACK);
				drawSentence(93,70, "LOOP", BLACK);
				drawRectBorder(0,0, 42, 160, GREEN);
				drawRectBorder(43, 0,42, 160, YELLOW);
				drawRectBorder(86, 0, 42,160, RED);
				menu = 1;
			}
			fastVar(100,20);
			secVariant(60,20);
			zombie(20,20);
			sunFlower(20,120);
			peashooter(20, 90);

			wallnut(60, 139);
			break;

		case LEVEL1:
			static unsigned char level1 = 0;
			
			if(reset1){
				level1 = 0;
				reset1 = 0;
				last_col = 0xFF;
				last_row = 0xFF;
			}
			if(!level1){
				lawn_grid(GREEN, DARK_GREEN, cursor_column, cursor_rows, 1);
				last_col = cursor_column;
				last_row = cursor_rows;
				level1 = 1;
			}
			else if(cursor_column != last_col || cursor_rows!= last_row){
				lawn_grid(GREEN, DARK_GREEN, cursor_column, cursor_rows, 0);
				last_col = cursor_column;
				last_row = cursor_rows;
			}
			break;

		case LEVEL2:
			static unsigned char level2 = 0;
			if(reset2){
				level2 = 0;
				reset2 = 0;
			}
			if(!level2){
				last_col = cursor_column;
				last_row = cursor_rows;
				level2 = 1;
				lawn_grid(DARK_PURPLE, PURPLE, cursor_column, cursor_rows, 1);
			}
			else if(cursor_column != last_col || cursor_rows!= last_row){
				lawn_grid(DARK_PURPLE, PURPLE, cursor_column, cursor_rows, 0);
				last_col = cursor_column;
				last_row = cursor_rows;
			}
			break;

		case LEVEL3:
			static unsigned char level3 = 0;
			if(reset3){
				level3 = 0;
				reset3 = 0;
			}
			else if(!level3){
				last_col = cursor_column;
				last_row = cursor_rows;
				level3 = 1;
				lawn_grid(BLUE, DARK_BLUE, cursor_column, cursor_rows, 1);
			}
			if(cursor_column != last_col || cursor_rows!= last_row){
				lawn_grid(BLUE, DARK_BLUE, cursor_column, cursor_rows, 0);
				last_col = cursor_column;
				last_row = cursor_rows;
			}
			break;

		case RENDER_GAMEOVER:
			fillScreen(RED);
			break;
		default:
			break;
	}

      return state;
}

enum REMOTE_STATE{RESET_IDLE, RESET};
int TickFct_RESET(int state){
        static decode_results actions;
        switch(state){
		case RESET_IDLE:
			if(IRdecode(&actions)){
				remote_input = actions.value;
				
				// ONLY handle BUTTON0 for reset
				if(remote_input == BUTTON0 && remote_input != last_remote_code){
					state = RESET;
					last_remote_code = remote_input;
				}
				
				IRresume();
			}
			break;
		case RESET:
			state = RESET_IDLE;
			break;
	}

	switch(state){
		case RESET_IDLE:
			break;
		case RESET:
			reset1 = 1;
			reset2 = 1;
			reset3 = 1;
			gamestate = 0;
			last_remote_code = 0;
			menu = 0;
            	for(unsigned char i = 0; i < 4; i++){
				for(unsigned char j = 0; j < 8; j++){
					grid[i][j] = 0;
				}
			}
			break;
		default:
			break;
	}

        return state;
}

enum LCD_STATES{LCD_IDLE, LCD_GAME, LCD_GAMEOVER};
int TickFct_LCD(int state){
        switch(state){
		case LCD_IDLE:
			if(gamestate >= 1 && gamestate <= 3){
				state = LCD_GAME;
			}
			break;
		case LCD_GAME:
			if(gamestate == 4){
				state = LCD_GAMEOVER;
			}
			else if(gamestate == 0){
				state = LCD_IDLE;
			}
			break;
		case LCD_GAMEOVER:
			if(gamestate == 0){
				state = LCD_IDLE;
			}
			break;
	}

        switch(state){
		case LCD_IDLE:
			lcd_clear();
			_delay_ms(2);
			lcd_goto_xy(0,0);
			_delay_us(50);
			if(joy_position == 0){
				lcd_write_str("LAWN/EASY");
			}
			else if(joy_position == 1){
				lcd_write_str("POOL/MEDIUM");
			}
			else{
				lcd_write_str("NIGHT/HARD");
			}
			lcd_goto_xy(1,0);
			_delay_us(50);
			lcd_write_str("SELECT LEVEL");
			break;
		case LCD_GAME:
			lcd_clear();
			_delay_ms(2);
			lcd_goto_xy(0,0);
			lcd_write_str("ZOMBIES");
			lcd_goto_xy(1,0);
			lcd_write_str("SUN:");
			break;
		case LCD_GAMEOVER:
			lcd_clear();
			_delay_ms(2);
			lcd_goto_xy(0,0);
			lcd_write_str("TIMER");
			lcd_goto_xy(1,0);
			lcd_write_str("KILLED");
			break;
		default:
			break;
	}   

        return state;
}

enum PLANT_STATES{PLANT_IDLE};

int TickFct_Plant(int state){
    static unsigned char last_plant_col = 0xFF;
    static unsigned char last_plant_row = 0xFF;
    
    switch(state){
        case PLANT_IDLE:
            // Reset button detection when cursor moves to new cell
            if(cursor_column != last_plant_col || cursor_rows != last_plant_row){
                last_remote_code = 0;  // Reset so next button works
                last_plant_col = cursor_column;
                last_plant_row = cursor_rows;
            }
            
            // Check for button press
            if(remote_input != last_remote_code && gamestate >= 1 && gamestate <= 3){
                
                // Check which button and place plant
                if(remote_input == BUTTON1){
                    grid[cursor_rows][cursor_column] = 5;  // Sunflower
                    health = 200;
                    last_remote_code = remote_input;
                }
                else if(remote_input == BUTTON2){
                    grid[cursor_rows][cursor_column] = 4;  // Peashooter
                    damage = 20;
                    health = 200;
                    last_remote_code = remote_input;
                }
                else if(remote_input == BUTTON3){
                    grid[cursor_rows][cursor_column] = 3;  // Wallnut
                    health = 1000;
                    damage = 0;
                    last_remote_code = remote_input;
                }
                else if((remote_input == BUTTON4) && gamestate == 3){
                    grid[cursor_rows][cursor_column] = 1;  // Lilypad
                    health = 200;
                    last_remote_code = remote_input;
                }
            }
            break;
            
        default:
            break;
    }
    
    return PLANT_IDLE;
}
enum BUZZER_STATES{BUZZER_MENU, BUZZER_GAME, BUZZER_GAMEOVER};
int TickFct_Buzzer(int state){
	static unsigned long counter = 0;
	switch(state){
		case BUZZER_MENU:
			if(gamestate >= 1 && gamestate <= 3){
				state = BUZZER_GAME;
				counter = 0;
			}
			break;
		case BUZZER_GAME:
			if(gamestate == 0){
				state = BUZZER_MENU;
				counter = 0;
			}
			else if(gamestate == 4){
				state = BUZZER_GAMEOVER;
				counter = 0;
			}
			break;
		case BUZZER_GAMEOVER:
			if(gamestate == 0){
				counter = 0;
				state = BUZZER_MENU;
			}
			break;
	}

	switch(state){
		case BUZZER_MENU:
			if(counter < 4){
				NOTEPLAYING(CSHARP);
			}
			else if(counter < 6){
				NOTEPLAYING(OFF);
			}
			else if(counter < 10){
				NOTEPLAYING(D);
			}
			else if(counter < 12){
				NOTEPLAYING(OFF);
			}
			else if(counter < 16){
				NOTEPLAYING(CSHARP);
			}
			else if(counter < 18){
				NOTEPLAYING(OFF);
			}
			else if(counter < 22){
				NOTEPLAYING(D);
			}
			else if(counter < 24){
				NOTEPLAYING(OFF);
			}
			else if(counter < 30){
				NOTEPLAYING(ASHARP);
			}
			else if(counter < 32){
				NOTEPLAYING(OFF);
			}
			else if(counter < 36){
				NOTEPLAYING(G);
			}
			else{
				counter = 0;
			}
			counter++;
			break;   

		case BUZZER_GAME:
			if(counter == 8){
				NOTEPLAYING(D);
			}
			else if(counter == 10){
				NOTEPLAYING(OFF);
			}
			else if(counter == 16){
				NOTEPLAYING(D);
			}
			else if(counter == 18){
				NOTEPLAYING(OFF);
			}
			else if(counter == 24){
				NOTEPLAYING(D);
			}
			else if(counter == 26){
				NOTEPLAYING(OFF);
			}
			else if(counter == 28){
				NOTEPLAYING(D);
			}
			else if(counter == 30){
				NOTEPLAYING(OFF);
			}
			else if(counter == 36){
				NOTEPLAYING(D);
			}
			else if(counter == 38){
				NOTEPLAYING(OFF);
			}
			else if(counter == 40){
				NOTEPLAYING(D);
			}
			else if(counter == 46){
				NOTEPLAYING(OFF);
			}
			else if(counter == 50){
				NOTEPLAYING(D);
			}
			else if(counter == 52){
				NOTEPLAYING(OFF);
			}
			else if(counter == 58){
				NOTEPLAYING(D);
			}
			else if(counter == 66){
				NOTEPLAYING(OFF);
			}
			else if(counter == 70){
				NOTEPLAYING(D);
			}
			else if(counter == 73){
				NOTEPLAYING(OFF);
			}
			else if(counter == 76){
				NOTEPLAYING(D);
			}
			else{
				counter = 0;
			}
			counter++;
			break;
			
		case BUZZER_GAMEOVER:
			if(counter < 12){
				NOTEPLAYING(D);
			}
			else if(counter < 16){
				NOTEPLAYING(OFF);
			}
			else{
				NOTEPLAYING(OFF);
			}
			counter++;
			break;
	}

	return state;
}

void TimerISR() {
	for(unsigned int i = 0; i < NUM_TASKS; i++){
		if(tasks[i].elapsedTime >= tasks[i].period){
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += GCD_PERIOD;
	}
}

int main(void) {
    DDRC = 0x30;
    PORTC = 0x00;
    PORTC |= (1 << 0);
    DDRD = 0xFB;
    PORTD = 0x00;
    DDRB = 0xB7;
    PORTB = 0x00;

    ADC_init();
    display_init();
    lcd_init();
    TimerSet(GCD_PERIOD);
    IRinit(&DDRD, &PIND, 2);
    TimerOn();

    tasks[0].period = JoyStick_PERIOD;
    tasks[0].state = JOYSTICK_IDLE;
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &TickFct_JoyStick;

    tasks[1].period = RENDER_PERIOD;
    tasks[1].state = MENU;
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &TickFct_Render;

    tasks[2].period = mouse_PERIOD;
    tasks[2].state = RESET_IDLE;
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &TickFct_RESET;

    tasks[3].period = LCD_PERIOD;
    tasks[3].state = LCD_IDLE;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &TickFct_LCD;

    tasks[4].period = PLANT_PERIOD;
    tasks[4].state = PLANT_IDLE;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &TickFct_Plant;

    tasks[5].period = MUSIC_PERIOD;
    tasks[5].state = BUZZER_MENU;
    tasks[5].elapsedTime = 0;
    tasks[5].TickFct = &TickFct_Buzzer;
    
    while(1) {}

    return 0;
}


void cherrybomb(uint16_t x, uint16_t y){
	Draw_Pixel(x+14, y, 0xFF0000);
	Draw_Pixel(x+13, y, 0xFF0000);
	Draw_Pixel(x+12, y, 0xFF0000);
	Draw_Pixel(x+11, y, 0xFF0000);
	Draw_Pixel(x+5, y, 0xFF0000);
	Draw_Pixel(x+4, y, 0xFF0000);
	Draw_Pixel(x+3, y, 0xFF0000);
	Draw_Pixel(x+2, y, 0xFF0000);

	Draw_Pixel(x+15, y+1, 0xFF0000);
	Draw_Pixel(x+14, y+1, 0xFF6B6B);
	Draw_Pixel(x+13, y+1, RED_LIGHT);
	Draw_Pixel(x+12, y+1, 0xFF6B6B);
	Draw_Pixel(x+11, y+1, RED_LIGHT);
	Draw_Pixel(x+10, y+1, RED);
	Draw_Pixel(x+6, y+1, RED);
	Draw_Pixel(x+5, y+1, RED_LIGHT);
	Draw_Pixel(x+4, y+1, RED_LIGHT);
	Draw_Pixel(x+3, y+1, RED_LIGHT);
	Draw_Pixel(x+2, y+1, RED_LIGHT);
	Draw_Pixel(x+1, y+1, RED);

	Draw_Pixel(x+16, y+2,RED);
	Draw_Pixel(x+15,y+2,RED_LIGHT);
	Draw_Pixel(x+14, y+2, RED_LIGHT);
	Draw_Pixel(x+13, y+2, BLACK);
	Draw_Pixel(x+12, y+2, BLACK);
	Draw_Pixel(x+11,y+2,RED_LIGHT);
	Draw_Pixel(x+10, y+2, RED_LIGHT);
	Draw_Pixel(x+9, y+2,RED);
	Draw_Pixel(x+7, y+2,RED);
	Draw_Pixel(x+6, y+2, RED_LIGHT);
	Draw_Pixel(x+5, y+2, BLACK);
	Draw_Pixel(x+4, y+2, RED_LIGHT);
	Draw_Pixel(x+3, y+2, RED_LIGHT);
	Draw_Pixel(x+2, y+2, BLACK);
	Draw_Pixel(x+1, y+2,RED);

	Draw_Pixel(x+16, y+3,RED);
	Draw_Pixel(x+15, y+3, RED_LIGHT);
	Draw_Pixel(x+14, y+3, BLACK);
	Draw_Pixel(x+13, y+3, RED_LIGHT);
	Draw_Pixel(x+12, y+3, RED_LIGHT);
	Draw_Pixel(x+11, y+3, BLACK);
	Draw_Pixel(x+10, y+3, RED_LIGHT);
	Draw_Pixel(x+9, y+3, RED);
	Draw_Pixel(x+7, y+3,RED);
	Draw_Pixel(x+6, y+3, RED_LIGHT);
	Draw_Pixel(x+5, y+3, RED_LIGHT);
	Draw_Pixel(x+4, y+3, BLACK);
	Draw_Pixel(x+3, y+3, BLACK);
	Draw_Pixel(x+2, y+3, RED_LIGHT);
	Draw_Pixel(x+1, y+3, RED);

	Draw_Pixel(x+16, y+4,RED);
	Draw_Pixel(x+15, y+4, RED_LIGHT);
	Draw_Pixel(x+14, y+4, RED_LIGHT);
	Draw_Pixel(x+13, y+4, RED_LIGHT);
	Draw_Pixel(x+12, y+4, RED_LIGHT);
	Draw_Pixel(x+11, y+4, RED_LIGHT);
	Draw_Pixel(x+10, y+4, RED_LIGHT);
	Draw_Pixel(x+9, y+4, RED);
	Draw_Pixel(x+7, y+4,RED);
	Draw_Pixel(x+6, y+4, RED_LIGHT);
	Draw_Pixel(x+5, y+4, RED_LIGHT);
	Draw_Pixel(x+4, y+4, RED_LIGHT);
	Draw_Pixel(x+3, y+4, RED_LIGHT);
	Draw_Pixel(x+2, y+4, RED_LIGHT);
	Draw_Pixel(x+1, y+4, RED);

	Draw_Pixel(x+16, y+5,RED);
	Draw_Pixel(x+15, y+5, RED_LIGHT);
	Draw_Pixel(x+14, y+5, BLACK);
	Draw_Pixel(x+13, y+5, RED_LIGHT);
	Draw_Pixel(x+12, y+5, RED_LIGHT);
	Draw_Pixel(x+11, y+5, RED_LIGHT);
	Draw_Pixel(x+10, y+5, BLACK);
	Draw_Pixel(x+9, y+5, RED);
	Draw_Pixel(x+7, y+5,RED);
	Draw_Pixel(x+6, y+5, BLACK);
	Draw_Pixel(x+5, y+5, RED_LIGHT);
	Draw_Pixel(x+4, y+5, RED_LIGHT);
	Draw_Pixel(x+3, y+5, RED_LIGHT);
	Draw_Pixel(x+2, y+5, BLACK);
	Draw_Pixel(x+1, y+5, RED);

	Draw_Pixel(x+16, y+6,RED);
	Draw_Pixel(x+15, y+6, RED_LIGHT);
	Draw_Pixel(x+14, y+6, RED_LIGHT);
	Draw_Pixel(x+13, y+6, RED_LIGHT);
	Draw_Pixel(x+12, y+6, RED_LIGHT);
	Draw_Pixel(x+11, y+6, RED_LIGHT);
	Draw_Pixel(x+10, y+6, RED);
	Draw_Pixel(x+6, y+6, RED);
	Draw_Pixel(x+5, y+6, RED_LIGHT);
	Draw_Pixel(x+4, y+6, RED_LIGHT);
	Draw_Pixel(x+3, y+6, RED_LIGHT);
	Draw_Pixel(x+2, y+6, RED_LIGHT);
	Draw_Pixel(x+1, y+6, RED);

	Draw_Pixel(x+15, y+7, RED);
	Draw_Pixel(x+14, y+7, RED);
	Draw_Pixel(x+13, y+7, RED);
	Draw_Pixel(x+12, y+7, RED);
	Draw_Pixel(x+11, y+7, RED);
	Draw_Pixel(x+5, y+7, RED);
	Draw_Pixel(x+4, y+7, RED);
	Draw_Pixel(x+3, y+7, RED);
	Draw_Pixel(x+2, y+7, RED);

	Draw_Pixel(x+13, y+8, BROWN);
	Draw_Pixel(x+3,y+8,BROWN);
	
	Draw_Pixel(x+12, y+9, BROWN);
	Draw_Pixel(x+4,y+9,BROWN);
	Draw_Pixel(x+5,y+9,BROWN);

	Draw_Pixel(x+11, y+10, BROWN);
	Draw_Pixel(x+10,y+10,BROWN);
	Draw_Pixel(x+9,y+10,BROWN);
	Draw_Pixel(x+6,y+10,BROWN);

	Draw_Pixel(x+8,y+11,BROWN);
	Draw_Pixel(x+7,y+11,BROWN);

	Draw_Pixel(x+7,y+12,BROWN);


}

// void lily(uint16_t x, uint16_t y){

// 	Draw_Pixel(x+13,y, DARK_GREEN);
// 	Draw_Pixel(x+12,y, DARK_GREEN);
// 	Draw_Pixel(x+11,y, DARK_GREEN);
// 	Draw_Pixel(x+10,y, DARK_GREEN);
// 	Draw_Pixel(x+9,y, DARK_GREEN);
// 	Draw_Pixel(x+8,y, DARK_GREEN);
// 	Draw_Pixel(x+7,y, DARK_GREEN);
// 	Draw_Pixel(x+6,y, DARK_GREEN);
// 	Draw_Pixel(x+5,y, DARK_GREEN);
// 	Draw_Pixel(x+4,y, DARK_GREEN);

// 	Draw_Pixel(x+14,y+1, DARK_GREEN);
// 	Draw_Pixel(x+13,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+1, DARK_GREEN);
// 	Draw_Pixel(x+8,y+1, DARK_GREEN);
// 	Draw_Pixel(x+7,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+1, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+1, DARK_GREEN);

// 	Draw_Pixel(x+15,y+2, DARK_GREEN);
// 	Draw_Pixel(x+14,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+2, DARK_GREEN);
// 	Draw_Pixel(x+8,y+2, DARK_GREEN);
// 	Draw_Pixel(x+7,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+2, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+2, DARK_GREEN);

	
// 	Draw_Pixel(x+14,y+3, DARK_GREEN);
// 	Draw_Pixel(x+13,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+3, DARK_GREEN);
// 	Draw_Pixel(x+10,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+3, DARK_GREEN);
// 	Draw_Pixel(x+8,y+3, DARK_GREEN);
// 	Draw_Pixel(x+7,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+3, DARK_GREEN);
// 	Draw_Pixel(x+5,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+3, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+3, DARK_GREEN);


// 	Draw_Pixel(x+15,y+4, DARK_GREEN);
// 	Draw_Pixel(x+14,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+4, DARK_GREEN);
// 	Draw_Pixel(x+10,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+4, DARK_GREEN);
// 	Draw_Pixel(x+8,y+4, DARK_GREEN);
// 	Draw_Pixel(x+7,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+4, DARK_GREEN);
// 	Draw_Pixel(x+5,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+4, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+4, DARK_GREEN);


// 	Draw_Pixel(x+15,y+5, DARK_GREEN);
// 	Draw_Pixel(x+14,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+5, DARK_GREEN);
// 	Draw_Pixel(x+11,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+5, DARK_GREEN);
// 	Draw_Pixel(x+8,y+5, DARK_GREEN);
// 	Draw_Pixel(x+7,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+5, DARK_GREEN);
// 	Draw_Pixel(x+4,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+5, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+5, DARK_GREEN);
	
// 	Draw_Pixel(x+15,y+6, DARK_GREEN);
// 	Draw_Pixel(x+14,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+6, DARK_GREEN);
// 	Draw_Pixel(x+11,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+6, DARK_GREEN);
// 	Draw_Pixel(x+8,y+6, DARK_GREEN);
// 	Draw_Pixel(x+7,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+6, DARK_GREEN);
// 	Draw_Pixel(x+4,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+6, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+6, DARK_GREEN);

// 	Draw_Pixel(x+16,y+7, DARK_GREEN);
// 	Draw_Pixel(x+15,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+14,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+7, DARK_GREEN);
// 	Draw_Pixel(x+12,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+7, DARK_GREEN);
// 	Draw_Pixel(x+8,y+7, DARK_GREEN);
// 	Draw_Pixel(x+7,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+7, DARK_GREEN);
// 	Draw_Pixel(x+3,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+7, LIGHT_GREEN);
// 	Draw_Pixel(x+1,y+7, DARK_GREEN);

// 	Draw_Pixel(x+16,y+8, DARK_GREEN);
// 	Draw_Pixel(x+15,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+14,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+8, DARK_GREEN);
// 	Draw_Pixel(x+12,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+8, DARK_GREEN);
// 	Draw_Pixel(x+8,y+8, DARK_GREEN);
// 	Draw_Pixel(x+7,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+8, DARK_GREEN);
// 	Draw_Pixel(x+3,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+8, LIGHT_GREEN);
// 	Draw_Pixel(x+1,y+8, DARK_GREEN);

// 	Draw_Pixel(x+16,y+9, DARK_GREEN);
// 	Draw_Pixel(x+15,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+14,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+9, DARK_GREEN);
// 	Draw_Pixel(x+12,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+9, DARK_GREEN);
// 	Draw_Pixel(x+8,y+9, DARK_GREEN);
// 	Draw_Pixel(x+7,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+9, DARK_GREEN);
// 	Draw_Pixel(x+3,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+9, LIGHT_GREEN);
// 	Draw_Pixel(x+1,y+9, DARK_GREEN);

// 	Draw_Pixel(x+16,y+10, DARK_GREEN);
// 	Draw_Pixel(x+15,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+14,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+9,y+10, DARK_GREEN);
// 	Draw_Pixel(x+8,y+10, DARK_GREEN);
// 	Draw_Pixel(x+7,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+6,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+10, LIGHT_GREEN);
// 	Draw_Pixel(x+1,y+10, DARK_GREEN);

// 	Draw_Pixel(x+15,y+11, DARK_GREEN);
// 	Draw_Pixel(x+14,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+10,y+11, DARK_GREEN);
// 	Draw_Pixel(x+9,y+11, DARK_GREEN);
// 	Draw_Pixel(x+8,y+11, DARK_GREEN);
// 	Draw_Pixel(x+7,y+11, DARK_GREEN);
// 	Draw_Pixel(x+6,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+5,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+11, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+11, DARK_GREEN);

// 	Draw_Pixel(x+15,y+12, DARK_GREEN);
// 	Draw_Pixel(x+14,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+13,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+11,y+12, DARK_GREEN);
// 	Draw_Pixel(x+6,y+12, DARK_GREEN);
// 	Draw_Pixel(x+5,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+4,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+12, LIGHT_GREEN);
// 	Draw_Pixel(x+2,y+12, DARK_GREEN);

// 	Draw_Pixel(x+14,y+13, DARK_GREEN);
// 	Draw_Pixel(x+13,y+13, LIGHT_GREEN);
// 	Draw_Pixel(x+12,y+13, DARK_GREEN);
// 	Draw_Pixel(x+5,y+13, DARK_GREEN);
// 	Draw_Pixel(x+4,y+13, LIGHT_GREEN);
// 	Draw_Pixel(x+3,y+13, DARK_GREEN);

// 	Draw_Pixel(x+13,y+14, DARK_GREEN);
// 	Draw_Pixel(x+4,y+14, DARK_GREEN);

// }

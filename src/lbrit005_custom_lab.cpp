/*         Luis & lbrit005@ucr.edu:

            Discussion: 023

 *         Assignment: Custom project part 2

 *         Exercise Description:In the final part of my project, 
                I implemented the game logic for the game. 
                This includes the spawning of zombies, the shooting of peas, 
                and the health of the plants. I also implemented the reset 
                functionality for the game using the remote.

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://youtu.be/zFHlvV_--JM?si=6YVCk0KMFsjEI8lP 
                https://youtu.be/GS2iN3kHR5o?si=jHzdR77wKjB44PKn 
                https://youtu.be/hfDb_sEYCB8?si=Ufre8EGdH06aY17d  >
 */

#include "lbrit005_timerISR.h"
#include "lbrit005_periph.h"
#include "lbrit005_spiAVR.h"
#include "lbrit005_irAVR.h"


volatile unsigned int adcHor = 0;                // detects horizontal position of joystick
volatile unsigned int adcVer = 0;               // detects vertical position of joystick
volatile unsigned char adcBut = 0;              // detects button press
volatile unsigned char gamestate = 0;                   // 0 for menu, 1 for level 1, 2 for level 2, 3 for level 3, and 4 for game over
volatile unsigned long remote_input;                    // detects input from remote
volatile unsigned char remote_received = 0;             // detects if remote input has been received
volatile unsigned char joy_button_pressed = 0;
volatile unsigned char joy_position = 0;
volatile unsigned char mouse_input = 0;                         // detects input from mouse
volatile unsigned char selected_plant = 0;                      // detects type of plant
volatile unsigned long last_remote_code = 0;
volatile unsigned char grid[3][7] = {{0}};
volatile unsigned char plant_health[3][7] = {{0}};              // grid to keep track of plants
volatile unsigned char cursor_column = 0;
volatile unsigned char cursor_rows = 0;                         // where the mpuse are at
volatile unsigned char moved;
volatile unsigned reset1 = 0;
volatile unsigned reset2 = 0;                                   // resets for each level 
volatile unsigned reset3 = 0;
volatile unsigned char menu = 0;
volatile unsigned char sun = 3;
volatile unsigned long gametimer = 0;                           // timer for the game


decode_results ir_results;


#define BUTTON1 0xFF30CF        
#define BUTTON2 0xFF18E7        
#define BUTTON3 0xFF7A85   
#define BUTTON4 0xFF10EF   
#define BUTTON0 0XFF6897       



#define SHOTS 15
#define ZOMBIES 5

SHOOTS PEA[SHOTS] = {0};
ZOMBIE ZOMBIE_ARRAY[ZOMBIES] = {0};



#define NUM_TASKS 8 //TODO: Change to the number of tasks being used



//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};//Stepper motor phases
int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //8 phases servo

const unsigned long JoyStick_PERIOD = 50;

const unsigned long mouse_PERIOD = 50;

const unsigned long LCD_PERIOD = 1000;

const unsigned long RENDER_PERIOD = 50;

const unsigned long PLANT_PERIOD = 50;

const unsigned long MUSIC_PERIOD = 100;

const unsigned long ZOMBIE_PERIOD = 100;

const unsigned long SHOOT_PERIOD = 500;

const unsigned long SUN_PERIOD = 50;

const unsigned long GCD_PERIOD = 10;//TODO:Set the GCD Period




task tasks[NUM_TASKS]; // declared task array with 8 tasks



enum JOYSTICK_STATES{JOYSTICK_IDLE, JOYSTICK_GAME};
int TickFct_JoyStick(int state){
        adcHor = ADC_read(1);
        adcVer = ADC_read(3);
        adcBut = GetBit(PINC, 2);
        unsigned char button = GetBit(PINC, 0);
        static unsigned char prev_but = 0;
        static unsigned char pos = 1;
        static unsigned int temp_Hor = 512;
        static unsigned int temp_ver = 512;

        switch(state){ // transitions for the joystick
                case JOYSTICK_IDLE:
                        if(gamestate >= 1 && gamestate <= 3){
                                state = JOYSTICK_GAME;
                        }
                break;
                case JOYSTICK_GAME:
                        if(gamestate == 0){
                                state = JOYSTICK_IDLE;
                        }
                break;
                default:
                break;
        }
        //actions for the joystick
        switch(state){
                case JOYSTICK_IDLE:
                                   //horizontal
        if(adcHor < 350 && temp_Hor >= 350){ // this to move the joystick and get the readings for the cursor
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
        joy_position = pos;

        if(button && !prev_but){ // this is to detect the button press of the joystick
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
                break;

                case JOYSTICK_GAME:
        if(adcHor < 350 && temp_Hor >= 350){
                if(cursor_column > 0){
                        cursor_column--;
                }
                temp_Hor = adcHor;
        }
        else if(adcHor > 700 && temp_Hor <= 700){
                if(cursor_column < 6){
                        cursor_column++;
                }
                temp_Hor = adcHor;
        }
        else if(adcHor >= 350 && adcHor <= 700 && (temp_Hor < 350 || temp_Hor > 700)){

                temp_Hor = adcHor;
        }
        else if(adcHor >= 350 && adcHor <= 700 && (temp_Hor < 350 || temp_Hor > 700)){
                temp_Hor = adcHor;
        }

        //vertical
        if(adcVer < 350 && temp_ver >= 350){
                if(cursor_rows > 0){
                        cursor_rows--;
                }
                temp_ver = adcVer;
        }
        else if(adcVer > 700 && temp_ver <= 700){
                if(cursor_rows < 2){
                        cursor_rows++;
                }
                temp_ver = adcVer;
        }
        else if(adcVer >= 350 && adcVer <= 700 && (temp_ver < 350 || temp_ver > 700)){

                temp_ver = adcVer;
        }
             break;   
        
             default:
                break;
              
        }
        prev_but = button;
         return state;   
}

        

enum RENDER_STATES{MENU, LEVEL1, LEVEL2, LEVEL3, RENDER_GAMEOVER};
int TickFct_Render(int state){
        static unsigned char last_col = 0xFF;
        static unsigned last_row = 0xFF;

        switch(state){ // transition for each state
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
                //actions for each state
        switch(state){
                case MENU:
                // this is the tft main menu screen
                        if(gamestate == 0 &&menu == 0){
                        fillScreen(PURPLE);


                        drawRectBorder(0,0, 42, 160, GREEN);//easy

                        drawRectBorder(43, 0,42, 160, YELLOW); // medium

                        drawRectBorder(86, 0, 42,160, RED); // hard pool

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
                static unsigned char level1 = 0; // to help with the reset
                        
                        if(reset1){
                                level1 = 0;
                                reset1 = 0;
                                last_col = 0xFF;
                                last_row = 0xFF;
                        }
                        if(!level1){
                        lawn_grid(GREEN, DARK_GREEN,cursor_column,cursor_rows,1);
                        last_col = cursor_column;
                        last_row = cursor_rows;
                        level1 = 1;
                        }
                        else if(cursor_column != last_col || cursor_rows!= last_row)      {
                        lawn_grid(GREEN, DARK_GREEN, cursor_column,cursor_rows,0);
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
                        lawn_grid(DARK_PURPLE,PURPLE,cursor_column,cursor_rows,1);
                        }
                        else if(cursor_column != last_col || cursor_rows!= last_row){
                        lawn_grid(DARK_PURPLE,PURPLE,cursor_column,cursor_rows,0);
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
                        lawn_grid(BLUE, DARK_BLUE,cursor_column,cursor_rows,1);
                        }
                        if(cursor_column != last_col || cursor_rows!= last_row){
                        lawn_grid(BLUE, DARK_BLUE,cursor_column,cursor_rows,0);
                        last_col = cursor_column;
                        last_row = cursor_rows;
                        }
                        break;
                case RENDER_GAMEOVER:
                static unsigned char drawn = 0;
    if(!drawn){
        fillScreen(RED);

        
        unsigned char tens = (gametimer / 100) % 10;
        unsigned char ones = (gametimer / 10) % 10;
        drawNumber(50, 60, tens, WHITE);
        drawNumber(75, 60, ones, WHITE);
        drawn = 1;
    }
    if(gamestate == 0){  
        drawn = 0;
        state = MENU;
                }
                                break;
                        default:
                                break;

        
     
}
 return state;
}
enum REMOTE_STATE{RESET_IDLE, RESET};
int TickFct_RESET(int state){
        //transitons
        static decode_results actions;
        static unsigned char i = 0;
        static unsigned char j =0;
        switch (state){
                case RESET_IDLE:
                
                if(IRdecode(&actions)){
                        remote_input = actions.value;
                        if(remote_input != last_remote_code &&remote_input == BUTTON0){

                        state = RESET;
                        last_remote_code = remote_input;
                        
                        }
                        
                        IRresume();
                }
                
        break;
        case RESET:
                state = RESET_IDLE;
                break;
        break;
        }
        

        //actions
switch(state){
        case RESET_IDLE:
        break;
        case RESET:
        sun = 3;
        reset1 = 1;
        reset2 = 1; // sets the reset in order to get to the main menu
        reset3 = 1;
        gamestate = 0; // changes the gamestate in order to get into the main menu
        last_remote_code = 0;
        menu = 0;
        for(i = 0; i < 3; i++){ // this is to reset the grid
                for(j = 0; j < 7; j++){
                grid[i][j] = 0;
                }
        }
        for(i = 0; i < SHOTS; i++){
        PEA[i].peashooters = 0;
    }
        for(i = 0; i < ZOMBIES; i++){
                ZOMBIE_ARRAY[i].zombies = 0;
        }

        break;
        default:
        break;

}

   return state;
}
 



enum PLANT_STATES{PLANT_IDLE, PLANT_SELECTION, SUNFLOWER, PEASHOOTER, WALLNUT};
        int TickFct_Plant(int state){
                        static unsigned  char plant_column = 0xFF;
                        static unsigned  char plant_rows = 0xFF;
                                                
                switch(state){
                case PLANT_IDLE:
                        if(cursor_column != plant_column || cursor_rows != plant_rows){ // so i can spawn in more plants
                                last_remote_code = 0;
                                plant_column = cursor_column;
                                plant_rows = cursor_rows;
                        }
                        if(remote_input != last_remote_code && gamestate >= 1 && gamestate <= 3){
                        state = PLANT_SELECTION;
                        }
                        
                break;
                case PLANT_SELECTION:
                        if(remote_input != last_remote_code){
                        if(remote_input == BUTTON1){
                                state = SUNFLOWER;
                        }
                        else if(remote_input == BUTTON2){
                                state = PEASHOOTER;
                        }
                        else if(remote_input == BUTTON3){
                                state = WALLNUT;
                        }
                        }
                        break;

                        case SUNFLOWER:
                        if(sun >= 1){
                        grid[cursor_rows][cursor_column] = 5;
                         plant_health[cursor_rows][cursor_column] = 200;
                        sun -= 1;
                        }
                        last_remote_code = remote_input;
                        state = PLANT_IDLE;
                        break;
                        case PEASHOOTER:
                        if(sun >= 2){
                        grid[cursor_rows][cursor_column] = 4;
                         plant_health[cursor_rows][cursor_column] = 200;
                        sun -=2;
                        last_remote_code = remote_input;
                        }
                        state = PLANT_IDLE;
                        break;
                        case WALLNUT:
                        grid[cursor_rows][cursor_column] = 3;
                        plant_health[cursor_rows][cursor_column] = 900;
                         sun -= 3;
                        last_remote_code = remote_input;
                        state = PLANT_IDLE;
                        break;
                        default:
                        break;
        }
        return state;
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
                }
                //actions

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
                        }else if(counter < 36){
                                NOTEPLAYING(G);}  else{
                        counter = 0;
                        }
                        counter++;

                        break;   

                        
                        break;
                        case BUZZER_GAME:
                        if(counter <= 8){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 10){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 16){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 18){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 24){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 26){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 28){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 30){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 36){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 38){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 40){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 46){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 50){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 52){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 58){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 66){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 70){
                                NOTEPLAYING(D);
                        }
                        else if(counter <= 73){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter <= 76){
                                NOTEPLAYING(D);
                        }
                        else{
                                counter = 0;
                        }
                
                        counter++;
                        break;
                        
                        case BUZZER_GAMEOVER:
                        counter = 0;
                        if(counter < 12){
                                NOTEPLAYING(D);
                        }
                        else if(counter < 16){
                                NOTEPLAYING(OFF);
                        }
                        else if(counter < 28){
                                NOTEPLAYING(OFF);
                        }
                        else{
                                NOTEPLAYING(OFF);
                                //need to manual it to reset will score the score of the 
                        }

                        counter++;
                        break;
                }

                return state;
        }




enum SHOOT_STATES {SHOOT_IDLE, SHOOT_FIRE};
        int TickFct_Peas(int state){
        static unsigned long counter = 0;
        static unsigned char row = 0;
        static unsigned char col = 0;

        switch(state){
                // transitons 
                case SHOOT_IDLE:
                        if(gamestate >= 1 && gamestate  <= 3){
                                state = SHOOT_FIRE;
                        }
                        break;
                case SHOOT_FIRE:
                        if(gamestate == 0){
                                state = SHOOT_IDLE;
                        }
                break;
                
        }
        //actions
        switch(state){
        case SHOOT_IDLE:
        break;

        case SHOOT_FIRE:
                counter++;
        if(counter >= 50){
                counter = 0; // how long the pea shooter takes to shoot
        for(row = 0; row < 3; row++ ){
                for(col  = 0; col < 7; col++){
                        if(grid[row][col] == 4){
                                 peaspawn(row, col);
                        }
                       
                }
        }
        }
        
                break;
                default:
                break;
        } 
        


        return state;
        


}

enum SUN_STATES{SUN_IDLE, SUN_GAME};
        int TickFct_Sun(int state){
        static unsigned char r = 0;
        static unsigned char c  = 0;
        static unsigned char counter = 0;
        //trasitons
        switch(state){
                case SUN_IDLE:
                if(gamestate >= 1 && gamestate <= 3){
                        state = SUN_GAME;
                }
                break;
                case SUN_GAME:
                if(gamestate == 0){
                state = SUN_IDLE; 
        }
                break;
        
        }
        switch (state)
        {
        case SUN_IDLE:
                break;
        case SUN_GAME:
                counter++;
                if(counter >= 200){ // how long the sun takes to add the counter
                for(r = 0; r < 3; r++){
                        for(c = 0; c < 7; c++){
                                if(grid[r][c] == 5){
                                        if(sun < 10){
                                                sun++; // the limit
                                        }
                                }
                        }
                }
                }
       
        break;
        default:
                break;
        }
        return state;
        }

enum ZOMBIES_PERIOD{ZOMBIE_IDLE, ZOMBIE_SPAWNING};
        
        int TickFct_Zombie(int state){
                static unsigned long counter = 0; // transition for the zombies to spawn in
                static unsigned char zob = 0;
                
                switch(state){
                        case ZOMBIE_IDLE:
                        gametimer = 0;
                                if(gamestate >= 1 && gamestate <= 3){
                                        state = ZOMBIE_SPAWNING;
                                        counter = 0;
                                }
                        break;
                        case ZOMBIE_SPAWNING:
                                if(gamestate == 0){
                                        state = ZOMBIE_IDLE;
                                        counter = 0;
                                }
                                else if(gamestate == 4){
                                        state = ZOMBIE_IDLE;
                                        counter = 0;
                                }
                        break;
                }
                //actions for the zombies to spawn in
                switch(state){
                        case ZOMBIE_IDLE:
                        break;
                        case ZOMBIE_SPAWNING:
                                counter++;
                                gametimer++;
                                if(counter >= 200){
                                        counter = 0;
                                         zob = 1;
                                        if(gamestate == 1){
                                               
                                                spawn(zob, 1);
                                        }
                                        else if(gamestate == 2){
                                                spawn((zob+1) % 3, 2);
                                                spawn(zob, 2);
                                        }
                                        else if(gamestate == 3){
                                                spawn((zob+1) % 3, 3);
                                                spawn((zob+2) % 3, 3);
                                                spawn(zob, 3);
                                        }

                                }

                        break;
                        default:
                        break;
                
        }
        return state;
}

        void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime >= tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}



//TODO: Create your tick functions for each task



int main(void) {
    //TODO: initialize all your inputs and ouputs
 // e.g. DDRx = 0xFF; for output, DDRx = 0x00; for input
    DDRC = 0x30;   // PC3 is my joystick horizontal position, PC1 is my joystick vertical position, and PC2 is my joystick button
    PORTC = 0x00;
    PORTC |= (1 << 0);
    DDRD = 0xFB; // PD2 is IR receiver input, PD3-PD5 is shift register for LCD, PD6 is buzzer for audio output
    PORTD = 0x00;
    DDRB = 0xB7; // this is the TFT
    PORTB = 0x00;




        ADC_init();   // initializes ADC
        display_init();
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

    tasks[3].period = PLANT_PERIOD;
    tasks[3].state = PLANT_IDLE;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &TickFct_Plant;

    tasks[4].period = MUSIC_PERIOD;
    tasks[4].state = BUZZER_MENU;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &TickFct_Buzzer;

    tasks[5].period = RENDER_PERIOD;  // 50ms
    tasks[5].state  = SHOOT_IDLE;
    tasks[5].elapsedTime = 0;
    tasks[5].TickFct = &TickFct_Peas;

    
    tasks[6].period = SUN_PERIOD;
    tasks[6].state = SUN_IDLE;
    tasks[6].elapsedTime = 0;
    tasks[6].TickFct = &TickFct_Sun;

        
    tasks[7].period = ZOMBIE_PERIOD;
    tasks[7].state = ZOMBIE_IDLE;
    tasks[7].elapsedTime = 0;
    tasks[7].TickFct = &TickFct_Zombie;

    while (1) {}

    return 0;
}

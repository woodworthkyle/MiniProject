/***********************************************************************
; ECE 362 - Mini-Project - Spring 2015
;***********************************************************************
;	 	   			 		  			 		  		
; Completed by: Team 2
;
;
; Academic Honesty Statement:  In entering my name above, I hereby certify
; that I am the individual who created this HC(S)12 source file and that I 
; have not copied the work of any other student (past or present) while 
; completing it. I understand that if I fail to honor this agreement, I will 
; receive a grade of ZERO and be subject to possible disciplinary action.
;                             
;***********************************************************************/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

// X and Y limits
#define MIN_X 0
#define MAX_X 80
#define MIN_Y 0
#define MAX_Y 30

// Colors available for VGA
#define COLOR_BLACK 0x00
#define COLOR_RED 0x01
#define COLOR_GREEN 0x02
#define COLOR_YELLOW 0x03
#define COLOR_BLUE 0x04
#define COLOR_PINK 0x05
#define COLOR_SKYBLUE 0x06
#define COLOR_WHITE 0x07

// Some initial parameters for the bird
#define ORIGIN_BIRD_X 5
#define ORIGIN_BIRD_Y 13
#define ORIGIN_BIRD_HEIGHT 2
#define ORIGIN_BIRD_WIDTH 6

// Parameters for the game
#define BIRD_FALL_TIME 2
#define BIRD_JUMP_HEIGHT 4
#define FALL_ACCELERATION 2

// Pipe standards
#define NUM_PIPES 4
#define PIPE_HEIGHT_MAX 30
#define PIPE_WIDTH_MIN 8
#define PIPE_SPACE_MIN 16

// VGA Commands
#define WRITE_CHAR 0x03
#define WRITE_COLOR 0x04

// Main Menu
#define MAIN_NUM_OPTIONS 3

// Accelerometer Data
struct __accel {
  unsigned char x;
  unsigned char y;
  unsigned char z;  
};
typedef struct __accel Accel;

// Address for VGA RAM
struct _addr {
  unsigned char upper;
  unsigned char lower;  
};
typedef struct _addr Address;

// Cartesian Point (x, y)
struct _point {
  unsigned char x;
  unsigned char y;  
};
typedef struct _point Point;

// Frame (width, height)
struct _frame {
  unsigned char width;
  unsigned char height;  
};
typedef struct _frame Frame;

// Rectangle (x, y, width, height)
struct _rect {
  Point origin;
  Frame frame; 
};
typedef struct _rect Rect;

// Pipe struct
struct _pipe {
  Rect rect;
  unsigned char topHeight;
  unsigned char gap;
  unsigned char currentPipe;
  Rect rectTop;
  Rect rectBottom;  
};
typedef struct _pipe Pipe;


/*********************************************************************** 
*
* Some global variables for easy manipulation
*
***********************************************************************/
Point globalPoint;
Address globalAddr;
Address tmpAddr;
Accel tmpAccel;
unsigned char i = 0;
unsigned char timerAccum = 0;
unsigned char asciiRet = 0;


/*********************************************************************** 
*
* Harware Level
*
***********************************************************************/
int ms_delay(int x);
void spiOut(byte data);
unsigned char htoa(unsigned char c);
char inchar(void);
void outchar(char x);

/*********************************************************************** 
*
* Drawing to screen
*
***********************************************************************/
Address point2Address(Point p);
void writeChar(byte addrUpper, byte addrLower, byte data);
void writeColor(byte addrUpper, byte addrLower, byte data);
void writeColorChar(Point p, byte color, byte c);
void clearRect(Rect r);
void clearScreen();

unsigned char clrOutter = 0;
unsigned char clrInner = 0;
unsigned char clrWidth = 0;
unsigned char clrHeight = 0;
unsigned char clrRow = 0;
unsigned char clrCol = 0;

/*********************************************************************** 
*
* Pipes
*
***********************************************************************/
void drawPipeSet(Pipe p);
Pipe calcPipeRects(Pipe p);
Pipe movePipeLeft(Pipe p);

Pipe pipes[NUM_PIPES];
unsigned char pipeFront = 0;
unsigned char pipeEnd = 0;
unsigned char pipeLeave = 0;
unsigned char passPipe = 0;

/*********************************************************************** 
*
* Bird
*
***********************************************************************/
void drawBird(Rect f);
void moveBirdUp();
void moveBirdDown();
void birdJump(void);
void collision(void);

unsigned char hitPipe = 0;
unsigned char pipeLeftTopY;
unsigned char pipeLeftBotY;
unsigned char pipeLeftX;
unsigned char pipeRightX;
unsigned char birdX;
unsigned char birdY;
Rect birdRect;

/*********************************************************************** 
*
* Push Buttons
*
***********************************************************************/
unsigned char toppb = 0;
unsigned char prevTop = 0;
unsigned char botpb = 0;
unsigned char prevBot = 0;

/***********************************************************************
*
* Sound Logic
*
***********************************************************************/

int buzzcount = 0;
unsigned char buzz = 0;
int wincount = 0;
unsigned char winbuzz = 0;


/*********************************************************************** 
*
* Main Logic
*
***********************************************************************/
void updatePipeSet();
void drawStart();
void drawScore();
void gameOver();

unsigned char updateDisplay = 0;
unsigned char start = 0;
unsigned char gameScore = 0;
unsigned char gameOverFlag = 0;


/*********************************************************************** 
*
* Main Menu
*
***********************************************************************/
void drawMainMenu();

unsigned char initialDraw = 0;
unsigned char mainMenu = 0;
unsigned char menuSelect = 0;
unsigned char play = 0;

const char line1[80] = "   ___________.__                       .__.__  __________.__           .___";
const char line2[80] = "   \\_   _____/|  | _____    ______ ____ |__|__| \\______   \\__|______  __| _/";	 	   		
const char line3[80] = "    |    __)  |  | \\__  \\  /  ___// ___\\|  |  |  |    |  _/  \\_  __ \\/ __ | ";
const char line4[80] = "    |     \\   |  |__/ __ \\_\\___ \\\\  \\___|  |  |  |    |   \\  ||  | \\/ /_/ | ";
const char line5[80] = "    \\___  /   |____(____  /____  >\\___  >__|__|  |______  /__||__|  \\____ | ";
const char line6[80] = "        \\/              \\/     \\/     \\/                \\/               \\/ ";

const char option1[12] = "Start Game";
const char option2[12] = "Secret Mode";
const char option3[12] = "About";
const char youDied[12] = "Game Over";
const char pressAny[40] = "Press Any Push Button to Continue";
char dispScore[9] = "Score 0  ";




/***********************************************************************
Initializations
***********************************************************************/
void  initializations(void) {

  // Set the PLL speed (bus clock = 24 MHz)
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL


  // Disable watchdog timer (COPCTL register)
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

  // Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
         
         
  // Add additional port pin initializations here
  DDRAD = 0x00;
  ATDDIEN = 0xC0;
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x18;
  ATDCTL4 = 0x85;
  
  // Initialize the SPI to baud rate of 6 Mbs
  SPIBR = 0x12;
  SPICR1 = 0x52;
  SPICR2 = 0x00;
	 	   			 		  			 		  		
  // Initialize digital I/O port pins
  DDRM = 0xFF;
  PTM = 0x00;
  DDRT = 0xFF;
  PTT = 0x00;
  
  // Initialize RTI for 2.048 ms interrupt rate
  RTICTL = 0x1F;
  CRGINT = 0x80;
  
  // Initialize TIM for 10ms interrupt rate
  TSCR1 = 0x80;  
  TSCR2 = 0x0C;  
  TIOS = 0x80; 
  TC7 = 15000;
  TIE = 0x80;
  
  // Initialize PWM  
  MODRR = 0x02;
  PWMCTL = 0x00;
  PWMPOL = 0x00;
  PWMCAE = 0x00;
  PWMPER1 = 0xFF;
  PWMDTY1 = 0x00;
  PWMPRCLK = 0x04; 
  PWMCLK = 0x02;
  PWMSCLA = 0x03;
  PWME = 0x02;


     
}

// Delay for x milliseconds
int ms_delay(int x) {
  asm {
    pshx
    pshy
    tfr D,Y
md1:ldx #5999
md2:dex
    bne md2
    dey
    bne md1
    puly
    pulx  
  }
  return x;  
}

// Convert a point into a address in VGA RAM 
Address point2Address(Point p) {
  
  if(p.y % 2) {
    // Odd row
    tmpAddr.lower = p.x + 128;
    tmpAddr.upper = (unsigned char)((p.y - 1)/2);  
  } else {
    // Even row
    tmpAddr.lower = p.x;
    tmpAddr.upper = p.y/2;
  } 
  return tmpAddr;
}

// Hex value to ASCII character
// - Used for SCI output
unsigned char htoa(unsigned char c) {
  c &= 0x0F;
  asciiRet = 0;
  if(c >= 0x0A && c <= 0x0F) {
    asciiRet = c + 0x37;
  } else {
    asciiRet = c + 0x30;
  }
  return asciiRet;
}

// Clear entire screen
void clearScreen() {
  clrRow = 0x00;
  clrCol = 0x00;
  while(clrRow <= 0x0F) {
    clrCol = 0x00;
    while(clrCol <= 0xD0) {
      writeChar(clrRow, clrCol, ' ');
      writeColor(clrRow, clrCol, COLOR_WHITE);
      clrCol++;
      if(clrCol == 0x50) {
        clrCol = 0x80;
      }
    }
    clrRow++;
  }
}

// Clears a rectangle on the screen
void clearRect(Rect r) {
  clrOutter = 0;
  clrInner = 0;
  
  clrWidth = r.frame.width;
  clrHeight = r.frame.height;
  globalPoint.x = r.origin.x;
  globalPoint.y = r.origin.y;
  
  while(clrOutter <= clrHeight) {
    globalPoint.x = r.origin.x;
    clrInner = 0;
    while(clrInner <= clrWidth) {
      tmpAddr = point2Address(globalPoint);
      writeChar(tmpAddr.upper, tmpAddr.lower, ' ');
      writeColor(tmpAddr.upper, tmpAddr.lower, COLOR_BLACK);
      globalPoint.x++;
      clrInner++;  
    }
    
    globalPoint.y++;
    clrOutter++;
  }
}

// Draw the bird defined in a rectangle
// -The x and y is really the only thing that matters
void drawBird(Rect f) {
  
  globalPoint.x = f.origin.x;
  globalPoint.y = f.origin.y;
  
  // Top half of bird (== (. )
  writeColorChar(globalPoint, COLOR_SKYBLUE, '=');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '=');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, ' ');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '(');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '.');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, ' ');
  
  // Bottom half of bird ( \___\)
  globalPoint.x = f.origin.x;
  globalPoint.y++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, ' ');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '\\');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '_');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '_');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '_');
  globalPoint.x++;
  writeColorChar(globalPoint, COLOR_SKYBLUE, '\\');
  
}


// Bounding rect
// Top pipe height
// Pipe spacing
void drawPipeSet(Pipe p) {   
  
  // Draw Top Pipe
  globalPoint.x = p.rect.origin.x;
  globalPoint.y = p.rect.origin.y;          
  while(globalPoint.y < p.topHeight) {
    globalPoint.x = p.rect.origin.x;
    writeColorChar(globalPoint, COLOR_GREEN, '|');
    globalPoint.x += p.rect.frame.width;
    writeColorChar(globalPoint, COLOR_GREEN, '|');
    globalPoint.y++;
  }
  
  globalPoint.x = p.rect.origin.x;
  i = 0;
  while(i <= p.rect.frame.width) {
    writeColorChar(globalPoint, COLOR_GREEN, '=');    
    globalPoint.x++;
    i++;
  }
  
  // Draw Bottom Pipe
  globalPoint.x = p.rect.origin.x;
  globalPoint.y = p.topHeight + p.gap;
  i = 0;
  while(i <= p.rect.frame.width) {
    writeColorChar(globalPoint, COLOR_GREEN, '=');    
    globalPoint.x++;
    i++;
  }
  
  globalPoint.x = p.rect.origin.x;  
  globalPoint.y = p.topHeight + p.gap + 1;
  while(globalPoint.y <= MAX_Y) {
    globalPoint.x = p.rect.origin.x;
    writeColorChar(globalPoint, COLOR_GREEN, '|');
    globalPoint.x += p.rect.frame.width;
    writeColorChar(globalPoint, COLOR_GREEN, '|');
    globalPoint.y++;
  }
     
}

// Shiftout SPI data
void spiOut(byte data) {
  while(!(SPISR&0x20));
  SPIDR = data;  
}

// Write character to VGA character RAM
void writeChar(byte addrUpper, byte addrLower, byte data) {
  // Send command then rest of data
  spiOut(WRITE_CHAR);
  spiOut(addrUpper);
  spiOut(addrLower);
  spiOut(data);
  spiOut(0x00);    
}

// Write color to VGA color RAM
void writeColor(byte addrUpper, byte addrLower, byte data) {
  // Send command then rest of data
  spiOut(WRITE_COLOR);
  spiOut(addrUpper);
  spiOut(addrLower);
  spiOut(data);
  spiOut(0x00);    
}

void writeColorChar(Point p, byte color, byte c) {
  globalAddr = point2Address(p);
  writeChar(globalAddr.upper, globalAddr.lower, c);
  writeColor(globalAddr.upper, globalAddr.lower, color);  
}

// Draw main menu screen
void drawMainMenu() {
  
  clearScreen();
  
  globalPoint.x = 0;
  globalPoint.y = 0;
  i = 0;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line1[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 0;
  globalPoint.y++;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line2[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 0;
  globalPoint.y++;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line3[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 0;
  globalPoint.y++;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line4[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 0;
  globalPoint.y++;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line5[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 0;
  globalPoint.y++;
  for(i = 0; i < 80; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, line6[i]);
    globalPoint.x++;    
  }
  
  
  // Draw test bird
  birdRect.origin.x = 24;
  birdRect.origin.y = 13;
  birdRect.frame.height = ORIGIN_BIRD_HEIGHT;
  birdRect.frame.width = ORIGIN_BIRD_WIDTH;
  drawBird(birdRect);
  
  i = 0;
  globalPoint.x = 32;
  globalPoint.y = 14;
  for(i = 0; i < 12; i++) {
    writeColorChar(globalPoint, COLOR_GREEN, option1[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 32;
  globalPoint.y += 2;
  for(i = 0; i < 12; i++) {
    writeColorChar(globalPoint, COLOR_GREEN, option2[i]);
    globalPoint.x++;    
  }
  
  i = 0;
  globalPoint.x = 32;
  globalPoint.y += 2;
  for(i = 0; i < 12; i++) {
    writeColorChar(globalPoint, COLOR_GREEN, option3[i]);
    globalPoint.x++;    
  }
  
}

// Calculate bounding rectangles for the top and bottom pipes of a pipe set
Pipe calcPipeRects(Pipe p) {
  p.rectTop.origin.x = p.rect.origin.x;
  p.rectTop.origin.y = p.rect.origin.y;
  p.rectTop.frame.width = p.rect.frame.width;
  p.rectTop.frame.height = p.topHeight;
  
  p.rectBottom.origin.x = p.rect.origin.x;
  p.rectBottom.origin.y = p.rect.origin.y + p.topHeight + p.gap;
  p.rectBottom.frame.width = p.rect.frame.width;
  p.rectBottom.frame.height = MAX_Y - p.rectBottom.origin.y;
  
  return p;
}

// This is the initial start of game
void drawStart() {
  // Clear screen
  clearScreen();
  
  // Draw test bird
  birdRect.origin.x = ORIGIN_BIRD_X;
  birdRect.origin.y = ORIGIN_BIRD_Y;
  birdRect.frame.height = ORIGIN_BIRD_HEIGHT;
  birdRect.frame.width = ORIGIN_BIRD_WIDTH;
  drawBird(birdRect); 
  
  // Draw test pipes
  pipes[0].rect.origin.x = 25;
  pipes[0].rect.origin.y = 0;
  pipes[0].rect.frame.height = PIPE_HEIGHT_MAX;
  pipes[0].rect.frame.width = PIPE_WIDTH_MIN;
  pipes[0].topHeight = 10;
  pipes[0].gap = 10;
  pipes[0] = calcPipeRects(pipes[0]);
  drawPipeSet(pipes[0]);
  
  pipes[1].rect.origin.x = pipes[0].rect.origin.x + pipes[0].rect.frame.width + PIPE_SPACE_MIN;
  pipes[1].rect.origin.y = 0;
  pipes[1].rect.frame.height = PIPE_HEIGHT_MAX;
  pipes[1].rect.frame.width = PIPE_WIDTH_MIN;
  pipes[1].topHeight = 5;
  pipes[1].gap = 10;
  pipes[1] = calcPipeRects(pipes[1]);
  drawPipeSet(pipes[1]);
  
  
  pipes[2].rect.origin.x = pipes[1].rect.origin.x + pipes[1].rect.frame.width + PIPE_SPACE_MIN;
  pipes[2].rect.origin.y = 0;
  pipes[2].rect.frame.height = PIPE_HEIGHT_MAX;
  pipes[2].rect.frame.width = PIPE_WIDTH_MIN;
  pipes[2].topHeight = 8;
  pipes[2].gap = 10;
  pipes[2] = calcPipeRects(pipes[2]);
  drawPipeSet(pipes[2]);
  
  
  pipes[3].rect.origin.x = pipes[2].rect.origin.x + pipes[2].rect.frame.width + PIPE_SPACE_MIN;
  pipes[3].rect.origin.y = 0;
  pipes[3].rect.frame.height = PIPE_HEIGHT_MAX;
  pipes[3].rect.frame.width = PIPE_WIDTH_MIN;
  pipes[3].topHeight = 11;
  pipes[3].gap = 10;
  pipes[3] = calcPipeRects(pipes[3]);
  drawPipeSet(pipes[3]);
  
  pipeFront = 0;
  pipeEnd = NUM_PIPES - 1; 
}

// output current game score in top right
void drawScore() {
  unsigned char ones;
  unsigned char tens;
  i = 0;
  globalPoint.x = 70;
  globalPoint.y = 1;
  
  // either two or one digit number
  if((gameScore / 10) > 0) {
    ones = gameScore % 10;
    tens = gameScore / 10;
    dispScore[6] = tens + '0';
    dispScore[7] = ones + '0';
  } else {
    dispScore[6] = gameScore + '0';
  }
  
  for(i = 0; i < 9; i++) {
    writeColorChar(globalPoint, COLOR_WHITE, dispScore[i]);
    globalPoint.x++; 
  }
  
}

void gameOver() {
  clearScreen();
  i = 0;
  globalPoint.x = 32;
  globalPoint.y = 14;
  for(i = 0; i < 12; i++) {
    writeColorChar(globalPoint, COLOR_GREEN, youDied[i]);
    globalPoint.x++;    
  }
  globalPoint.y++;
  globalPoint.x = 20;
  for(i = 0; i < 40; i++) {
    writeColorChar(globalPoint, COLOR_GREEN, pressAny[i]);
    globalPoint.x++;
  }
  
  //If the high score is beaten, sound an alarm
  if(gameScore > 2){
    PWMDTY1 = 0x80;
    wincount = 0;
    winbuzz = 1;
  }

  
}

// Moves bird up
void moveBirdUp() {
  clearRect(birdRect);
  //clearScreen();
  birdRect.origin.y--;
  drawBird(birdRect);
}

// Simulates a "jump"
void birdJump() {
  clearRect(birdRect);
  birdRect.origin.y -= BIRD_JUMP_HEIGHT;
  drawBird(birdRect); 
}

// Moves bird down
void moveBirdDown() {
  clearRect(birdRect);
  //clearScreen();
  birdRect.origin.y++;
  drawBird(birdRect);
}

// Moves pipe to left
Pipe movePipeLeft(Pipe p) {
  //clearRect(p.rect);
  clearRect(p.rectTop);
  clearRect(p.rectBottom);
  p.rect.origin.x--;
  
  // set the pipe closest to the bird
  if(p.rect.origin.x <= 15) {
    p.currentPipe = 1;
  } else {
    p.currentPipe = 0;
  }
  
  // pipe has passed the bird, increment the score
  if(p.rect.origin.x < 5 && p.rect.origin.x > 3) {
    gameScore++;
  }
  
  p = calcPipeRects(p);
  drawPipeSet(p);
  return p;      
}

// Moves pipe to end of buffer if it has left the screen
void updatePipeSet() {
  
  // Check if front pipe leaves screen
  pipeLeave = (pipes[pipeFront].rect.origin.x + pipes[pipeFront].rect.frame.width + 1)&0x00FF;
  if(pipeLeave == 0) {
    // Pipe has left screen
    
    //
    //
    // Random pipe generation?
    //
    //
    pipes[pipeFront].rect.origin.x = pipes[pipeEnd].rect.origin.x + pipes[pipeEnd].rect.frame.width + PIPE_SPACE_MIN;
    pipes[pipeFront].rect.origin.y = 0;
    
    
    pipeEnd = pipeFront;
    pipeFront++;
    if(pipeFront >= NUM_PIPES) {
      pipeFront = 0;  
    } 
  } 
}


// check for the bird running into pipes
void collision() {
  int j;
  birdX = birdRect.origin.x;
  // normalize to the height of the screen
  birdY = birdRect.origin.y % MAX_Y; 
  
  for(j=0; j<4; j++) {
    pipeLeftTopY = pipes[j].topHeight;
    pipeLeftBotY = pipes[j].topHeight + pipes[j].gap;
    // normalize to the width of the screen
    pipeLeftX = pipes[j].rect.origin.x % MAX_X;
    pipeRightX = pipes[j].rect.origin.x + pipes[j].rect.frame.width % MAX_X;  
    
    if(pipes[j].currentPipe) {
      // hit the top half of the pipe
      if(birdY <= pipeLeftTopY && birdX >= pipeLeftX && (birdX + ORIGIN_BIRD_WIDTH) <= pipeRightX) {
        hitPipe = 1;
      }
      
      // hit the bottom half of the pipe
      if((birdY + ORIGIN_BIRD_HEIGHT) >= pipeLeftBotY && birdX >= pipeLeftX && (birdX + ORIGIN_BIRD_WIDTH) <= pipeRightX) {
        hitPipe = 1;
      }
    }
  }
}


	 		  			 		  		
/***********************************************************************
Main
***********************************************************************/
void main(void) {
  // local variables
  int birdfall = 0;
  int accel = 1;
  int count = 0;
  
  DisableInterrupts;
	initializations(); 		  			 		  		
	EnableInterrupts;
  
  
  // For debugging purposes
  mainMenu = 1;
  initialDraw = 1;
  start = 0;
  play = 0;
  menuSelect = 1;
  
  for(;;) {
  //loop
    
    // Game Over Screen 
    if(gameOverFlag) {
      if(toppb || botpb) {
        toppb = 0;
        botpb = 0;
        gameScore = 0;
        mainMenu = 1;
        initialDraw = 1;
        updateDisplay = 1;
        gameOverFlag = 0;
      }
    }
    
    // Main Menu/Title Screen
    if(mainMenu) {      
      
      // Initial draw of main menu
      if(initialDraw && updateDisplay) {
        initialDraw = 0;
        drawMainMenu();  
      }
      
      if(toppb) {
        toppb = 0;
        menuSelect--;
        if(menuSelect <= 0) {
          menuSelect = MAIN_NUM_OPTIONS;
          moveBirdDown();
          moveBirdDown();
          moveBirdDown();
          moveBirdDown();
        } else {
          moveBirdUp();
          moveBirdUp();  
        }
      }
      
      if(botpb) {
        botpb = 0;
        mainMenu = 0;
        start = 1;
      }
    }
    
    // This state is executed right before a new game starts
    if(start) {
      start = 0;
      drawStart();
      drawScore();     
      play = 1;
    }
    
    // bird hit the pipe, restart game
    if(hitPipe) {
      hitPipe = 0;
      start = 0;
      play = 0;
      //gameScore = 0; moved to if(gameOverFlag) so high score can be assessed
      gameOverFlag = 1;
      dispScore[7] = ' ';
      gameOver();
    }
    
    //PWM should be silent if no buzz is needed
    if(!buzz & !winbuzz)
      PWMDTY1 = 0;
    
    //High score buzzer logic
    if(winbuzz){
      if(wincount == 25)
        PWMDTY1 = 0x40;
      if(wincount == 50)
        PWMDTY1 = 0x80;
    }


    
    // Game logic
    if(updateDisplay && play) {
      // "pop"the bird up
      if(toppb) {
        PWMDTY1 = 0x80; //turn on the PWM
        buzzcount = 0;
        buzz = 1;    
        toppb = 0;
        accel = 1;
        birdJump();
      } else {
        birdfall++;
      }
      
      // control the downward fall of the bird
      if(birdfall == BIRD_FALL_TIME) {
        birdfall = 0;
        count = accel;
        moveBirdDown();        
        
        // accelerate the birds fall
        do {
          moveBirdDown();
          count -= FALL_ACCELERATION;
        } while (count > 0);
        accel++;
      }      
      
      // move pipes
      updateDisplay = 0;      
      pipes[0] = movePipeLeft(pipes[0]);
      pipes[1] = movePipeLeft(pipes[1]);
      pipes[2] = movePipeLeft(pipes[2]);
      pipes[3] = movePipeLeft(pipes[3]);
      updatePipeSet();
      
      // calculate if the bird has hit a pipe
      collision();
      drawScore();
    }
    
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}




/***********************************************************************                       
; RTI interrupt service routine: RTI_ISR
;
;  Initialized for 2.048 ms interrupt rate
;
;  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
;
;  If change in state from "high" to "low" detected, set pushbutton flag
;     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
;     Recall that pushbuttons are momentary contact closures to ground
;
;  Also, increments 2-byte variable "random" each time interrupt occurs
;  NOTE: Will need to truncate "random" to 12-bits to get a reasonable delay   			 		  			

 		  		
;***********************************************************************/
interrupt 7 void RTI_ISR(void)
{
  	// set CRGFLG bit 
  	CRGFLG = CRGFLG | 0x80;
  	
  	timerAccum++;
  	if(timerAccum == 50) {
  	  updateDisplay = 1;
      timerAccum = 0;  	  
  	}
  	
  	if(!(PTAD&0x80) && prevTop) {
  	  toppb = 1;
  	}
    prevTop = (PTAD&0x80);  	
  	
  	
  	if(!(PTAD&0x40) && prevBot) {
  	  botpb = 1;
  	}
    prevBot = (PTAD&0x40);
    

    
}

/***********************************************************************                       
;  TIM interrupt service routine
;
;  Initialized for 1.00 ms interrupt rate
;
;  Increment (3-digit) variable "react" by one 			 		  			 		  		
;***********************************************************************/
interrupt 15 void TIM_ISR(void)
{
  // set TFLG1 bit 
   TFLG1 = TFLG1 | 0x80;
   
   //Flapping noise logic
 	 if(buzz && buzzcount < 10) {
   if(buzzcount == 5)
    PWMDTY1 = PWMDTY1 / 2;
   buzzcount++;
  }
  else if(buzz){
   buzzcount = 0;
   buzz = 0;
  }

  //Winning Siren Logic
  if(winbuzz && wincount < 50) {
    wincount++ ;
  } else if (winbuzz){
    wincount = 0;
    winbuzz++;
    if(winbuzz > 6)
      winbuzz = 0;
  }
 	
 	 

}

/***********************************************************************
; Character I/O Library Routines for 9S12C32
;***********************************************************************
; Name:         inchar
; Description:  inputs ASCII character from SCI serial port and returns it
; Example:      char ch1 = inchar();
;***********************************************************************/
char  inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
 
}

/***********************************************************************
; Name:         outchar
; Description:  outputs ASCII character passed in outchar()
;                  to the SCI serial port
; Example:      outchar('x');
;***********************************************************************/
void outchar(char ch) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = ch;
}
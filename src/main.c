//---#defines---
#include <stdbool.h>
#include "gba.h"
#include "mgba.h"//logger library
//---Math functions---

//---Global variables---
#define GBA_SH 160                                        //actual gba screen height
#define SCREEN_WIDTH 120                                        //game screen width
#define SCREEN_HEIGHT 80                                        //game screen height
#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))                //15 bit, 0-31, 5bit=r, 5bit=g, 5bit=b 
int lastFr=0,FPS=0;                                        //for frames per second

typedef struct
{
 int x;
 int y;
}myCursor; 
myCursor Cursor;

//init map
unsigned char mapBackBuffer[SCREEN_WIDTH][SCREEN_HEIGHT]={0};
unsigned char mapFrontBuffer[SCREEN_WIDTH][SCREEN_HEIGHT]={0};

//calculates which pixels should be turned off and on
void calculate_gol(){
  //copy back buffer to front buffer the bad way!
  //TODO: there has to be a better way (maybe swtich the pointers around?)
  for(int y=1;y<SCREEN_HEIGHT-1;y++){
    for(int x=1;x<SCREEN_WIDTH-1;x++){
      mapBackBuffer[x][y] = mapFrontBuffer[x][y];
    }
  }

  for(int y=1;y<SCREEN_HEIGHT-1;y++){
    for(int x=1;x<SCREEN_WIDTH-1;x++){ 
      int neighbor_counter = 0;
      //check all 8 directions for neighbours
      if(mapBackBuffer[x-1][y-1]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x][y-1]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x+1][y-1]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x-1][y]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x+1][y]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x-1][y+1]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x][y+1]==1){
        neighbor_counter ++;
      }
      if(mapBackBuffer[x+1][y+1]==1){
        neighbor_counter ++;
      }

      if(neighbor_counter == 3){
        mapFrontBuffer[x][y] = 1;
      }
      else if(neighbor_counter == 2){
        //just do nothing
      }
      else{
        mapFrontBuffer[x][y] = 0;
      }
    }
  }
}

//draws game of life

void draw_gol()
{int x,y;
  for(x=0;x<SCREEN_WIDTH;x++)
  {
   for(y=0;y<SCREEN_HEIGHT;y++){ 
      if(mapFrontBuffer[x][y]==1){
        VRAM[y*GBA_SH+x]=RGB(0,0,0);
      }
      else{
        VRAM[y*GBA_SH+x]=RGB(8,12,16);
      }
    }
  }
}

void set_a_cell(int value){
  //mapBackBuffer[Cursor.x][Cursor.y] = value;
  mapFrontBuffer[Cursor.x][Cursor.y] = value;
}

void clear_screen(int value){
  for(int y=1;y<SCREEN_HEIGHT-1;y++){
    for(int x=1;x<SCREEN_WIDTH-1;x++){
      //mapBackBuffer[x][y] = value;
      mapFrontBuffer[x][y] = value;
    }
  }
}

void buttons()
{
  if(KEY_R ){ Cursor.x+=1; if(Cursor.x>SCREEN_WIDTH-2){ Cursor.x=SCREEN_WIDTH-2;}}
  if(KEY_L ){ Cursor.x-=1; if(Cursor.x<   1){   Cursor.x=   1;}}
  if(KEY_U ){ Cursor.y-=1; if(Cursor.y<   1){   Cursor.y=   1;}}
  if(KEY_D ){ Cursor.y+=1; if(Cursor.y>SCREEN_HEIGHT-2){ Cursor.y=SCREEN_HEIGHT-2;}}
  if(KEY_A ){ calculate_gol();} 
  else{ VRAM[Cursor.y*GBA_SH+Cursor.x]=RGB(0,31,0);}  //draw the cursor only if the simulation is not running
  if(KEY_B ){ } 
  if(KEY_LS){ set_a_cell(0);} 
  if(KEY_RS){ set_a_cell(1);} 
  if(KEY_ST){ clear_screen(0);} 
  if(KEY_SL){ } 
}

void init()
{
  //position the cursor  in the middle of the screen
  Cursor.x = SCREEN_WIDTH/2;
  Cursor.y = SCREEN_HEIGHT/2;
}

int main()
{
 //Init mode 5------------------------------------------------------------------
 *(u16*)0x4000000 = 0x405;                                           //mode 5 background 2
 //*(u16*)0x400010A = 0x82;                                            //enable timer for fps
 //*(u16*)0x400010E = 0x84;                                            //cnt timer overflow

 //scale small mode 5 screen to full screen-------------------------------------
 REG_BG2PA=256/2;                                                    //256=normal 128=scale 
 REG_BG2PD=256/2;                                                    //256=normal 128=scale 

 init();   
 mgba_console_open();//connect to logger

 while(1)
 { 
    draw_gol();
    buttons();                                                        //button input

    //mgba_printf(4,"works");

    //swap buffers---------------------------------------------------------------
    while(*Scanline<160){}	                                         //wait all scanlines 
    if  ( DISPCNT&BACKB){ DISPCNT &= ~BACKB; VRAM=(u16*)VRAM_B;}      //back  buffer
    else{                 DISPCNT |=  BACKB; VRAM=(u16*)VRAM_F;}      //front buffer  
 }
 mgba_close();
}

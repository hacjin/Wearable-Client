#include <Thread.h>
#include <ThreadController.h>
#include <TinyScreen.h>
#include <SPI.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

// AD0 low = 0x68 (default for InvenSense evaluation boa rd)
// AD0 high = 0x69
MPU6050 accelgyro;


int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
String packet;



//menu index
int index = 0;
int modebit =0;
int cnt_loop = 0;
int pre_btn = 0;


// ThreadController that will controll all threads
ThreadController controll = ThreadController();

Thread packetThread = Thread();

TinyScreen display = TinyScreen(0);


void packCallback(){
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    gx = map(gx,-16383, 16383, -360, 360);
    gy = map(gy,-16383, 16383, -360, 360);
    gz = map(gz,-16383, 16383, -360, 360);
    ax = map(ax,-16383, 16383, -98, 98);
    ay = map(ay,-16383, 16383, -98, 98);
    az = map(az,-16383, 16383, -98, 98);
    mx = map(mx,-16383, 16383, -360, 360);
    my = map(my,-16383, 16383, -360, 360);
    mz = map(mz,-16383, 16383, -360, 360); 
    packet="S"+String(index)+"/"+String(gx)+"/"+String(gy)+"/"+String(gz)+"/"+String(ax)+"/"+String(ay)+"/"+String(az)+"/"+String(mx)+"/"+String(my)+"/"+String(mz);
    Serial.print(packet);

}

void setup(){
	Serial.begin(9600);
  display.begin();
  display.setFlip(1);//LCD 화면 전환
  display.setFont(liberationSans_16ptFontInfo);
  display.setCursor(2,0);
  display.print("Motion");
  display.setFont(liberationSans_14ptFontInfo);
  display.setCursor(2,20);
  display.print("Accelerator");  
  
  accelgyro.initialize();
  
	packetThread.onRun(packCallback);
	controll.add(&packetThread);

}

void menu(){
  //screen button
  byte rightdownButton=(1<<3);   // 오른쪽 아래
  byte leftdownButton=(1<<0);  // 왼쪽아래

  if(display.getButtons()&leftdownButton){
    if(index > 0 && pre_btn != 1) {
      index = index -1;
      pre_btn = 1;
      display.clearWindow(0,0,96,64);
      display.setFont(liberationSans_16ptFontInfo);
      display.setCursor(2,0);
      display.print("Motion");
      display.setFont(liberationSans_14ptFontInfo);
      display.setCursor(2,20);
      display.print("Accelerator");
    }
  }
  else if(display.getButtons()&rightdownButton){
    if(index < 6 && pre_btn != 4) {
      index = index +1;
      pre_btn = 4;
      display.clearWindow(0,0,96,64);
      display.setFont(liberationSans_16ptFontInfo);
      display.setCursor(2,0);
      display.print("Motion");
      display.setFont(liberationSans_14ptFontInfo);
      display.setCursor(2,20);
      display.print("Accelerator");
    }
  }
  else if(pre_btn!=0){
    cnt_loop++;
    if(cnt_loop==5){
      pre_btn = 0;
      cnt_loop = 0;
    }   
  }
  

  switch(index) {
    case 0: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : Basic");
            break;
    case 1: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : Mouse");
            break;
    case 2: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : Process");
         
            break;
    case 3: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : ProgramMark");
         
            break;     
    case 4: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : Audio");
         
            break;   
    case 5: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : PPT");

            break;   
    case 6: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,40);
            display.print(" Mode : GomPlayer");

            break;              
  }
}
void loop(){
    
  menu();
	controll.run();
}

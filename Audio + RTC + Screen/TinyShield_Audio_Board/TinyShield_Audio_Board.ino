#include "WaveUtil.h"
#include "WaveHC.h"
#include "DSRTCLib.h"
#include <avr/pgmspace.h>
#include <TinyScreen.h>
#include <Thread.h>
#include <ThreadController.h>

// MP3 Audio part start
const char* const fileNam[100] PROGMEM =  {"test.wav", "test1.wav","test2.wav"};
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

int mvol = 2;
int midx=0;
int cnt_loop = 0;
int pre_btn = 0;
int cnt_loop2 = 0;
int pre_btn2= 0;
int index=0;

TinyScreen display = TinyScreen(0);

// ThreadController that will controll all threads
ThreadController controll = ThreadController();

Thread menuThread = Thread();
Thread mpThread = Thread();
DS1339 RTC = DS1339();
void setup() {
  byte i;
  Serial.begin(9600);
  display.begin();
  display.setFlip(1);//LCD 화면 전환
  RTC.start();
  menuThread.onRun(menu);
  controll.add(&menuThread);
}

void loop() {
  RTC.readTime();
 
  controll.run();
}

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}
void ramCheck(){
 putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
    // Set the output pins for the DAC control. This pins are defined in the library

  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}

void mpCallBack(){
   //screen button
  byte rightdownButton=(1<<3);   // 오른쪽 아래
  byte leftdownButton=(1<<0);  // 왼쪽아래
  byte rightupButton=(1<<2); // 오른쪽 위
  byte leftupButton=(1<<1); // 왼쪽위
  char* data1;
  data1= (char*)pgm_read_word(&(fileNam[midx]));

  display.setFont(liberationSans_12ptFontInfo);
  display.setCursor(2,0);
  switch(midx){
    case 0: display.print("Mc The Max");
            display.setFont(liberationSans_14ptFontInfo);
            display.setCursor(0,19);
            display.print("Singing...");
            break;
    case 1: display.print("Zion T");
            display.setFont(liberationSans_14ptFontInfo);
            display.setCursor(0,19);
            display.print("Singing...");
            break;
    case 2: display.print("Baek A Yeon");
            display.setFont(liberationSans_14ptFontInfo);
            display.setCursor(0,19);
            display.print("Singing...");
            break;
  }
  
  Serial.print("Playing ");
  Serial.println(data1);
  playfile(data1);  

  
  while(wave.isplaying){
    if(display.getButtons()&rightupButton){
      if( pre_btn !=4 ){// 볼륨
       mvol++;
       wave.volume = mvol;
       pre_btn=4;
       if(mvol==6)
       {
        mvol=0;
        wave.volume=mvol;
       }
      }    
      
    }else if(display.getButtons()&leftdownButton){ //끄기
      wave.stop();
      controll.remove(&mpThread);
    }
    else if(display.getButtons()&leftupButton){ //음원넘기기
      if(midx < 100 && pre_btn !=1){
       midx++;
       Serial.println(midx);     // FAT16 or FAT32?
       pre_btn=1;
      }
      wave.stop();
      controll.remove(&mpThread);
      controll.add(&mpThread);
      break;      
    }
    else if(pre_btn!=0){
      cnt_loop++;
      if(cnt_loop==5){
        pre_btn=0;
        cnt_loop=0;
      }
    }
    //Do other stuff while playing
  };
}

void menu(){
  //screen button
  byte rightdownButton=(1<<3);   // 오른쪽 아래
  byte leftdownButton=(1<<0);  // 왼쪽아래
  byte rightupButton=(1<<2); // 오른쪽 위
  byte leftupButton=(1<<1); // 왼쪽위

  if(index != 1)
    printTime();
  
  if(display.getButtons()&leftdownButton){
    if(index > 0 && pre_btn2 != 1) {
      index = index -1;
      pre_btn2 = 1;
      display.clearWindow(0,0,96,64);
    }
  }
  else if(display.getButtons()&rightdownButton){
    if(index < 2 && pre_btn2 != 2) {
      index = index +1;
      pre_btn2 = 2;
      display.clearWindow(0,0,96,64);
    }
  }
  else if(pre_btn2!=0){
    cnt_loop2++;
    if(cnt_loop2==3){
      pre_btn2 = 0;
      cnt_loop2 = 0;
    }   
  }
   
  switch(index) {
    case 0: 
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,42);
            display.print(" Mode : Basic");   
            if (wave.isplaying) {
              wave.stop(); // stop it
            }
            break;
    case 1: 
            display.clearWindow(0,0,96,64);
            display.setFont(liberationSans_10ptFontInfo);
            display.setCursor(0,42);
            display.print(" Mode : Mp3");
            mpThread.onRun(mpCallBack);
            controll.add(&mpThread);
            break;  
  }
}
void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }else{
    ramCheck();
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file ");
    midx=0;
    Serial.print(name); 
    return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
  }
  wave.volume = mvol;
  // ok time to play! start playback
  wave.play();
}

void printTime()
{ 
  display.setFont(liberationSans_14ptFontInfo);
  display.setCursor(2,0);
  display.print(RTC.getYears());
  display.print("/");  
  if(int(RTC.getMonths()) < 10){
    display.print("0");
    display.print(int(RTC.getMonths()));
  }
  else{
    display.print(int(RTC.getMonths()));
  }
  display.print("/");  
  if(int(RTC.getDays()) < 10){
    display.print("0");
    display.print(int(RTC.getDays()));
  }
  else{
    display.print(int(RTC.getDays()));
  }
  display.setFont(liberationSans_22ptFontInfo);
  display.setCursor(0,19);
  display.print("  ");
  if(int(RTC.getHours()) < 10){
    display.print("0");
    display.print(int(RTC.getHours()));
  }
  else{
    display.print(int(RTC.getHours()));
  }
  display.print(":");
  if(int(RTC.getMinutes()) < 10){
    display.print("0");
    display.print(int(RTC.getMinutes()));
  }
  else{
    display.print(int(RTC.getMinutes()));
  }
  display.print(":");
  if(int(RTC.getSeconds()) < 10){
    display.print("0");
    display.print(int(RTC.getSeconds()));
  }
  else{
    display.print(int(RTC.getSeconds()));
  }  
}

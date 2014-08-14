#include "DS3234lib3.h"
#include "Arduino.h"
#include <EEPROM.h>

  //****************************************************************

DS3234::DS3234()
{
}

  //****************************************************************

byte DS3234::ConvertIntToPackedBCD(int integer){
  byte BCDH = (integer/10);
  byte BCDL = (integer - BCDH*10);
  byte BCDP = (BCDH<<4) | BCDL;
  return BCDP;
  }

  //****************************************************************YOOHoo EAster eGG
  
void DS3234::fetchAndSetTime(){
  Serial.begin(19200);
  
  DS3234::spiInit();
  int timeoutCount = 0;
  boolean timeout = false;
  while((!Serial) && (!timeout)){              //Wait for serial connection to open, with timeout contingency
    for(int i = 0; i < 1000; i++) asm("nop");
    timeoutCount = timeoutCount + 1;
    if(timeoutCount > 100) timeout = true;
    }
    
  Serial.println("GT");                       //Send request to Get Time
  
  while((Serial.available() == 0) && (!timeout)){    //Wait until host responds, with timeout contingency
    for(int i = 0; i < 1000; i++) asm("nop");
    timeoutCount = timeoutCount + 1;
    if(timeoutCount > 100) return;
    }
    
  PORTB &= ~(1<<PORTB2);                      //Open SPI connection with DS3234 (SS=Lo)
  SPDR = 0x81;                                //Ox81 is write address of minutes register
  while(!(SPSR & (1<<SPIF)));
  byte x = SPDR;
  for(int i = 0; i < 6; i++){   
    SPDR = DS3234::ConvertIntToPackedBCD(Serial.parseInt()); //Repeatedly parse time data from serial buffer then
    while(!(SPSR & (1<<SPIF)));                      //write 0x81 through 0x86  DS3234 registers
    byte z = SPDR;
    }
  PORTB |= (1<<PORTB2);         //Close SPI connection with DS3234 (SS=Hi)
    
  for(int i = 0; i < 1000; i++) asm("nop");
    
  PORTB &= ~(1<<PORTB2);        //Open SPI connection with DS3234 (SS=Lo)
  SPDR = 0x80;                  //Ox80 is address of seconds register
  while(!(SPSR & (1<<SPIF)));
  x = SPDR;
  byte seconds = DS3234::ConvertIntToPackedBCD(Serial.parseInt());
  while(Serial.available() != 0){  //Clear serial buffer
    Serial.read();
    }
  while(Serial.available() == 0){}  //Stop here until serial buffer gets filled again
  SPDR = seconds;                  //Ox80 is address of seconds register
  while(!(SPSR & (1<<SPIF)));
  PORTB |= (1<<PORTB2);         //Close SPI connection with DS3234 (SS=Hi)
  Serial.begin(19200);
  for(int i = 0; i < 3; i++){
    Serial.print("CT");
    Serial.println(DS3234::timeStamp());
    delay(1000);
    }
  }

  //**************************bobo**************************************

void DS3234::getLaunchParameters(long &_interval, int &_dayStart, int &_hourStart, int &_minStart){

	 while(1){
		 Serial.begin(19200);
		 int timeoutCount = 0;
		 boolean timeout = false;
		 while((!Serial) && (!timeout)){              //Wait for serial connection to open, with timeout contingency
			for(int i = 0; i < 1000; i++) asm("nop");
			timeoutCount = timeoutCount + 1;
			if(timeoutCount > 100) timeout = true;
			}
		 if(timeout == true) break;
		 Serial.println("LP");
		 
		 while((Serial.available() == 0) && (!timeout)){    //Wait until host responds, with timeout contingency
			for(int i = 0; i < 1000; i++) asm("nop");
			timeoutCount = timeoutCount + 1;
			if(timeoutCount > 100) timeout = true;
			}
		 
		 if(timeout == true) break;
		 
		 char logFileName[12];
		 byte i = 0;
		 unsigned int interval = Serial.parseInt();
		 int dayStart = Serial.parseInt();
		 int hourStart = Serial.parseInt();
		 int minStart = Serial.parseInt();
		 
		 //Code to parse file name
		 while((Serial.read() != '/n') && (i < 12)){
		  logFileName[i] = Serial.read();
			i++;
		 }
			
		 //  Write all parameters to EEPROM
		 EEPROM.write(0x01, interval & 0x00FF);
		 EEPROM.write(0x02, (interval & 0xFF00) >> 8 );
		 EEPROM.write(0x03, dayStart);
		 EEPROM.write(0x04, hourStart);
		 EEPROM.write(0x05, minStart);
		 
		 for(int i = 0; i < 12; i++){
			EEPROM.write(0x06 + i, logFileName[i]);
		 }
		 
		 while(Serial.available() != 0){  //Clear serial buffer
		 Serial.read();}
		
		 delay(1000);
		 
		 Serial.print("PC");
		 Serial.print(dayStart);
		 Serial.print(",");
		 Serial.print(hourStart);
		 Serial.print(",");
		 Serial.print(minStart);
		 Serial.println(",");
		 
		 while(Serial.available() != 0){  //Clear serial buffer
			Serial.read();
			}
		 break;
		}
	_interval = EEPROM.read(0x01) + (EEPROM.read(0x02) << 8);
	_dayStart = EEPROM.read(0x03);
	_hourStart = EEPROM.read(0x04);
	_minStart = EEPROM.read(0x05);
}

  //****************************************************************

String DS3234::timeStamp(){
  DS3234::spiInit();
  char time[14];
  PORTB &= ~(1<<PORTB2);        //Put SS low to signal SPI begin
  SPDR = 0;                     //Write SPDR to 0x00, address of seconds to be read
  while(!(SPSR & (1<<SPIF)));  //Wait for transmission to finish
  byte x = SPSR;                     //Sequence clears SPIF
  x = SPDR;
  for(int i = 6; i >= 0; i--){
    SPDR = 0xFF;                  //Arbitrary data to shift (this is a read operation)
    while(!(SPSR & (1<<SPIF)));   //Wait for transmission to finish
    byte recieved = SPDR;
    time[2*i + 1] = (recieved & 0x0F) + 48;
    time[2*i] = ((recieved & 0xF0) >> 4) + 48;
    }
  //PORTB |= (1<<PORTB2);			//Put SS high to signal SPI finish
  char stamp[18] = {time[2],time[3],'/',time[4],time[5],'/',time[0],time[1],' ',time[8],time[9],':',time[10],time[11],':',time[12],time[13]};
  for(int i=0; i<18; i++)
	{
		timestamp[i] = stamp[i]-48;
	}
	String StampString(stamp);
  return StampString;
  //return stamp;
  }

  //****************************************************************
	
void DS3234::getFileTimeStamp(){
	year = 2000+timestamp[6]*10+timestamp[7];
	month = timestamp[0]*10+timestamp[1];
	day = timestamp[3]*10+timestamp[4];
	hour = timestamp[9]*10+timestamp[10];
	minute = timestamp[12]*10+timestamp[13];
	second = timestamp[15]*10+timestamp[16];
}

	//****************************************************************

void DS3234::alarmFlagClear(){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8F;                  //Ox8F is address of DS3234 control/status register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0x00;                  //Clear alarm 2 flag
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  }

  //****************************************************************

void DS3234::spiInit(){
  DDRB |= ((1<<DDB5) | (1<<DDB3) | (1<<DDB2));  //Set SPI clock pin, mosi, and SS to output
  PORTB |= (1<<PORTB2);	// Set SS to HIGH
  SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<CPOL) | (1<<CPHA); //SPI enable, in master mode, speed /128
  byte x = SPSR;  //Read SPI status register
  x = SPDR;  //Read SPI data register (sequence clears SPIF)  
  }

  //****************************************************************
  
boolean DS3234::alarm2set(int date, int hours, int mins){
  DS3234::spiInit();
  int startTime[4] = {mins, hours, date, 0x06};  //Create start day & time array
  PORTB &= ~(1<<PORTB2);                   //Open SPI connection with DS3234 (SS=Lo)
  SPDR = 0x8B;                             //Ox8B is write address of Alarm2 minutes register
  while(!(SPSR & (1<<SPIF)));              //wait for SPIF
  byte z = SPDR;                           //read SPDR to clear SPIF    
  for(int i = 0; i < 4; i++){   
    SPDR = DS3234::ConvertIntToPackedBCD(startTime[i]); //Repeatedly parse time data from serial buffer then
    while(!(SPSR & (1<<SPIF)));                      //write 0x8B through 0x8E clock registers.  0x8E sets enables alarm
    byte z = SPDR;
    }
  PORTB |= (1<<PORTB2);                     //Close SPI connection with DS3234 (SS=Hi)
}
  
	 //****************************************************************CORN!
  
void DS3234::secondAlarmSet(int s){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8E;                  //Ox8E is address of DS3234 control register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b00000101;            //Set SQW pin to interrupt functionality, enable alarm 1
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  
  
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x87;                  //Ox8B is address of first DS3234 first alarm register (address then auto-incremented)
  while(!(SPSR & (1<<SPIF)));
  SPDR = DS3234::ConvertIntToPackedBCD(s);            //Set SQW pin to interrupt functionality, enable alarm 1
  while(!(SPSR & (1<<SPIF)));
  for(int i = 0; i < 3; i++){
    SPDR = 0b10000000;          //0x80 sets alarm mask bit in each register
    while(!(SPSR & (1<<SPIF)));
    }
  
  PORTB |= (1<<PORTB2);        //Put SS high (SPI end)
  }
  
	   //****************************************************************
		 
void DS3234::minuteAlarmSet(int m){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8E;                  //Ox8E is address of DS3234 control register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b00000110;            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  
  
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8B;                  //Ox8B is address of first DS3234 first alarm register (address then auto-incremented)
  while(!(SPSR & (1<<SPIF)));
  SPDR = DS3234::ConvertIntToPackedBCD(m);            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));
  for(int i = 0; i < 2; i++){
    SPDR = 0b10000000;          //0x80 sets alarm mask bit in each register
    while(!(SPSR & (1<<SPIF)));
    }
  
  PORTB |= (1<<PORTB2);        //Put SS high (SPI end)
  }
  
	   //***********************BERG, yo*****************************************
		 
void DS3234::hourAlarmSet(int m, int h){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8E;                  //Ox8E is address of DS3234 control register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b00000110;            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  
  
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8B;                  //Ox8B is address of first DS3234 first alarm register (address then auto-incremented)
  while(!(SPSR & (1<<SPIF)));
  SPDR = DS3234::ConvertIntToPackedBCD(m);            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));
	SPDR = DS3234::ConvertIntToPackedBCD(h);            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b10000000;          //0x80 sets alarm mask bit in each register
  while(!(SPSR & (1<<SPIF)));
    
  
  PORTB |= (1<<PORTB2);        //Put SS high (SPI end)
  }
  
	   //****************************************************************

void DS3234::setNextAlarm(){
  if(alarmType==0) // alarm in seconds
  {
		if(t>=60)
		{
			t=t-60;
		}
		DS3234::secondAlarmSet(t);
  }
	else if(alarmType==1)	// alarm in minutes
	{
		if(t>=3600)
		{
			t=t-3600;
		}
		DS3234::minuteAlarmSet(t/60);
	}
	else if(alarmType==2) // alarm in hours (converted to minutes)
	{
		if(t>=86400)
		{
			t=t-86400;
		}
		int hour = t/3600;
		DS3234::hourAlarmSet(min, hour);  //send global variable min which is start logging minute
	}
  t=interval+t;  // t is a global variable
}

  //****************************************************************
  
void DS3234::checkInterval(int h, int m, long intervul){
	hr=h;
	min=m;
	interval=intervul;
	if(interval <= 59)
  {
		alarmType=0;
		t=interval;
  }
  else if(interval >=60 && interval <= 3599)
  {
		alarmType=1;
		t=(min*60)+interval;
  }
  else if(interval >= 3600)
  {
		alarmType=2;
		t=(hr*3600)+interval;
  }
  else return;
  }
  
    //****************************************************************t'was a long night
  
int DS3234::GetSeconds(){
  DS3234::spiInit();
  
  PORTB &= ~(1<<PORTB2);        //Put SS low to signal SPI begin
  SPDR = 0;                     //Write SPDR to 0x00, address of seconds to be read
  while(!(SPSR & (1<<SPIF)));  //Wait for transmission to finish
  byte x = SPSR;                     //Sequence clears SPIF
  x = SPDR;
  delay(2);

  SPDR = 0xFF;                  //Arbitrary data to shift (this is a read operation)
  while(!(SPSR & (1<<SPIF)));   //Wait for transmission to finish
  byte shit = SPSR;
  shit = SPDR; 
  int seconds = ((shit & 0x0F) + (shit >> 4) * 10);
  PORTB |= (1<<PORTB2);                     //Close SPI connection with DS3234 (SS=Hi)

  return seconds;
  }

	 //****************************************************************

void DS3234::minuteAlarmBegin(){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8E;                  //Ox8E is address of DS3234 control register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b00000110;            //Set SQW pin to interrupt functionality, enable alarm 2
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  
  
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8B;                  //Ox8B is address of first DS3234 first alarm register (address then auto-incremented)
  while(!(SPSR & (1<<SPIF)));
  for(int i = 0; i < 3; i++){
    SPDR = 0b10000000;          //0x80 sets alarm mask bit in each register
    while(!(SPSR & (1<<SPIF))); 
    }
  
  PORTB |= (1<<PORTB2);        //Put SS high (SPI end)
  }

  //****************************************************************
  
void DS3234::secondAlarmBegin(){
  DS3234::spiInit();
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x8E;                  //Ox8E is address of DS3234 control register
  while(!(SPSR & (1<<SPIF)));
  SPDR = 0b00000101;            //Set SQW pin to interrupt functionality, enable alarm 1
  while(!(SPSR & (1<<SPIF)));   
  PORTB |= (1<<PORTB2);         //End transmission
  
  
  PORTB &= ~(1<<PORTB2);        //Begin transmission
  SPDR = 0x87;                  //Ox8B is address of first DS3234 first alarm register (address then auto-incremented)
  while(!(SPSR & (1<<SPIF)));
  for(int i = 0; i < 4; i++){
    SPDR = 0b10000000;          //0x80 sets alarm mask bit in each register
    while(!(SPSR & (1<<SPIF)));
    }
  
  PORTB |= (1<<PORTB2);        //Put SS high (SPI end)
  }

	  //****************************************************************asm("nop");
 
#include <SPI.h>

// SS:   pin 10
// MOSI: pin 11
// MISO: pin 12
// SCK:  pin 13

#define READ_ARRAY        0x0B
#define CHIP_ERASE        0x60
#define BYTE_PAGE_PROGRAM 0x02
#define WRITE_ENABLE      0x06
#define WRITE_DESABLE     0x04
#define READ_SR_BYTE_1    0x05
#define READ_SR_BYTE_2    0x35
#define READ_ID           0x90
#define READ_MF_ID        0x9F
#define WRITE_SR          0x01






#define DUMMY_BYTE        0x00




void setup(void) {

    pinMode(13,OUTPUT);
    SPI.begin();
    SPI.setDataMode(0);
    SPI.setBitOrder(MSBFIRST);
    digitalWrite(SS, HIGH);
    
    Serial.begin(115200);
    Serial1.begin(115200);
    
}




void return_manufacturer_id(void);
void return_status_registers(void);
void return_firsts_bytes_from_flash(void);
void flash(void);


void loop(void) {

  unsigned char serial_rx;



        while(!Serial.available());
        serial_rx = Serial.read();
        
        if(serial_rx == 'a')                           // read manufacturer id
        {
          return_manufacturer_id();
        }

          
        else if(serial_rx == 'b')
        {
          return_status_registers();
        }
        
     
        else if(serial_rx == 'c')                     //read while 135,183 bitstream bytes
        {
          return_firsts_bytes_from_flash();
        }

        else if(serial_rx == 'd')
        {
          erase_chip();
        }
        else if(serial_rx == 'f')            
        {
          flash();
        }
        else if(serial_rx == 'x')
        {
          Serial1.write(0x00);
          while(1)
          {
            if(Serial1.available())
            {
              Serial.write(Serial1.read());
            }
          }
        }
}

void return_manufacturer_id(void)
{
  digitalWrite(SS, LOW);
  SPI.transfer(READ_MF_ID);
          
  Serial.write(SPI.transfer(0));
  Serial.write(SPI.transfer(0));
  Serial.write(SPI.transfer(0));
          
  digitalWrite(SS, HIGH);
}

void return_status_registers(void)
{
  
  digitalWrite(SS,LOW);
  SPI.transfer(WRITE_SR); //disable write protection to the whole device
  SPI.transfer(0x00);
  SPI.transfer(0x02);
  digitalWrite(SS, HIGH);
          
  digitalWrite(SS, LOW);
  SPI.transfer(READ_SR_BYTE_1);
  Serial.write(SPI.transfer(0));
  digitalWrite(SS,HIGH);

  digitalWrite(SS, LOW);
  SPI.transfer(READ_SR_BYTE_2);
  Serial.write(SPI.transfer(0));
  digitalWrite(SS,HIGH);
}

void return_firsts_bytes_from_flash(void)
{
  digitalWrite(SS, LOW);

  SPI.transfer(READ_ARRAY);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(DUMMY_BYTE);//dummy byte
  
  for(unsigned int i = 0; i < 100; i++){
    Serial.write(SPI.transfer(DUMMY_BYTE));
  }
  
  digitalWrite(SS, HIGH);
}

void erase_chip(void)
{
  digitalWrite(SS, LOW);
    SPI.transfer(WRITE_ENABLE);
  digitalWrite(SS, HIGH);

  //chip erase
  digitalWrite(SS, LOW);
    SPI.transfer(CHIP_ERASE);
  digitalWrite(SS, HIGH);

  //wait for erase complete
  digitalWrite(SS, LOW);
    SPI.transfer(READ_SR_BYTE_1);
    char sr;
    do
    {
      sr = SPI.transfer(DUMMY_BYTE);
      Serial.write(sr);
    }while(sr & 0x01);
  digitalWrite(SS, HIGH);  
}

void flash(void)
{
  uint32_t addr = 0;
  
  char sr;
  unsigned int count = 0;
  char program_buffer[256];

    


                  

  
  do
  {
    digitalWrite(SS, LOW);
      SPI.transfer(WRITE_ENABLE);
    digitalWrite(SS, HIGH);

     
    for(int i = 0; i != 32; i++)
    {
      while(Serial.available() == 0);
      program_buffer[i] = Serial.read();
    }
    
    digitalWrite(SS,LOW);
      SPI.transfer(BYTE_PAGE_PROGRAM);
      SPI.transfer((uint8_t)(addr>>16));
      SPI.transfer((uint8_t)(addr>>8));
      SPI.transfer((uint8_t)(addr));
              
      for(int i = 0; i != 32; i++) SPI.transfer(program_buffer[i]); 
       
    digitalWrite(SS,HIGH);

    digitalWrite(SS, LOW);
      SPI.transfer(READ_SR_BYTE_1);
      do
      {              
        sr = SPI.transfer(0);
      }while(sr & 0x01);
    digitalWrite(SS, HIGH);
            
    addr += 32;
    Serial.write('x');
            
  }while(addr < 135179);
}

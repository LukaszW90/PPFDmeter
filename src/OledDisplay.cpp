#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common.hpp"
#include "HwConfig.hpp"
#include "I2c.hpp"
#include "OledDisplay.hpp"
#include "displayDatabase.hpp"


#define OLED_CONTROL_BYTE_CMD_STREAM  0x00
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

#define POKEMON_WIDTH  56
#define POKEMON_HEIGHT 56


static void OledSendCmd(uint8_t cmd); 
static void drawDigit(  const int row, const int column, const int digit);
static void drawNumber8x16_RightAlign(  const int row, const int column_px, const int number);
static void drawIco16x16(  const int row, const int column, const uint8_t* ico);
void OledDrawPixel(int x, int y, uint8_t color, uint8_t*buffer);

static uint8_t oled_buffer[OledDisplay::DisplayArraySize];
static uint8_t poke_buffer[OledDisplay::DisplayArraySize];
static uint8_t poke_buffer_mask[OledDisplay::DisplayArraySize];


void OledDisplay::init(void) {
    vTaskDelay(pdMS_TO_TICKS(150));
    OledSendCmd(0xAE); 
    OledSendCmd(0x20); // Tryb adresowania pamięci
    OledSendCmd(0x00); // 0x00 = Horizontal Addressing Mode
    OledSendCmd(0xB0); 
    OledSendCmd(0xC8); 
    OledSendCmd(0x00); 
    OledSendCmd(0x10); 
    OledSendCmd(0x40); 
    OledSendCmd(0x81); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xCF); 
    OledSendCmd(0xA1); 
    OledSendCmd(0xA6); 
    OledSendCmd(0xA8); 
    OledSendCmd(0x3F); 
    OledSendCmd(0xA4); 
    OledSendCmd(0xD3); 
    OledSendCmd(0x00); 
    OledSendCmd(0xD5); 
    OledSendCmd(0x80);
    OledSendCmd(0xD9); 
    OledSendCmd(0xF1);
    OledSendCmd(0xDA); 
    OledSendCmd(0x12);
    OledSendCmd(0xDB); 
    OledSendCmd(0x40);
    OledSendCmd(0x8D); 
    OledSendCmd(0x14); 
    OledSendCmd(0xAF); 

    clearScreen(); 
    Refresh();
}

void OledDisplay::Refresh()
{
    for (int page = 0; page < 8; page++) {
        I2C::write_bytes(HwConfig::I2cAddr::Oled, OLED_CONTROL_BYTE_DATA_STREAM, &oled_buffer[page * 128], 128);
    }
}

void OledDisplay::clearScreen(void) 
{
    // Wypełniamy cały bufor 1024 bajtów zerami (czarny ekran)
    clearBuffer();
    // Natychmiast wysyłamy wyczyszczony stan przez I2C do kontrolera SSD1306
    Refresh();
}

void OledDisplay::clearBuffer(void) 
{
    memset(oled_buffer, 0, sizeof(oled_buffer));

}

void OledDisplay::drawScreen(MeasurementData& data, ConfigData& config, StatusData& status)
{
   clearBuffer();
   memcpy(&oled_buffer[0], &background_bmp[0], OledDisplay::DisplayArraySize); 

   drawNumber8x16_RightAlign(0, 80, data.lux );
   drawNumber8x16_RightAlign(2, 80, data.ppfd );
   drawNumber8x16_RightAlign(4, 80, data.temperature );
   drawDigit(4,88,12);
   drawDigit(4,96,13);

   drawNumber8x16_RightAlign(6, 80, data.humidity );
   drawDigit(6,88,10);
   drawDigit(6,96,11);
    drawIco16x16(0,128-16,&ico16x16_bmp::lightProfiles[static_cast<int>(config.Profile)][0]);
    if (status.BleStatus)
        drawIco16x16(2,128-16,&icons16x16[0][0]);
    if (status.recording)
    {
        drawIco16x16(4,128-16,&icons16x16[0][0]);
    }
    drawIco16x16(6,128-16,&icons16x16[0][0]);
}

// NOWA DYNAMICZNA FUNKCJA RYSUJĄCA SPRITE NA POZYCJI X, Y
void OledDisplay::drawVictrebelAt(int target_x, int target_y,const uint8_t* sprite, uint8_t *buffer)
{
    // Czyścimy bufor pamięci, żeby usunąć ducha z poprzedniej klatki
    memset(buffer, 0, sizeof(oled_buffer)); 

    for (int y = 0; y < POKEMON_HEIGHT; y++) {
        for (int x = 0; x < POKEMON_WIDTH; x++) {
            int byte_idx = (y * (POKEMON_WIDTH / 8)) + (x / 8);
            int bit_idx = 7 - (x % 8);
            uint8_t pixel = (sprite[byte_idx] >> bit_idx) & 0x01;

            if (pixel) {
                // Rysujemy piksel przesunięty o dynamiczną współrzędną target_x i target_y
                OledDrawPixel(target_x + x, target_y + y, 1,buffer);
            }
        }
    }
}



    // Rozmiary obiektów do sprawdzania kolizji
    const int displayWidth = 128;
    const int displayHeight = 64;
    const int spriteWidth = 56;
    const int spriteHeight = 56;

void OledDisplay::drawScreenSaver()
{
 // 2. Parametry startowe fizyki naszej "gry"
    static int posX = 32;       // Startowa pozycja X (środek ekranu)
    static int posY = 0;        // Startowa pozycja Y
    static int speedX = 2;      // Prędkość w poziomie (piksele na klatkę)
    static int speedY = 1;      // Prędkość w pionie
    
// Aktualizacja współrzędnych
        posX += speedX;
        posY += speedY;

        // KOLIZJA Z LEWĄ LUB PRAWĄ ŚCIANĄ
        if (posX <= 0 || posX >= (DisplayWidth_px - spriteWidth)) {
            speedX = -speedX; // Odwróć ruch w poziomie
        }

        // KOLIZJA Z GÓRNĄ LUB DOLNĄ ŚCIANĄ 
        // (Wysokość ekranu to 64 i sprite to 64, więc w pionie na ekranie 128x64 
        // ruch będzie minimalny lub zerowy, zależnie od ustawienia posY)
        if (posY < 0 || posY > (DisplayHeight_px - spriteHeight)) {
            speedY = -speedY; // Odwróć ruch w pionie
        }


if (speedX<0)
{
    drawVictrebelAt(posX,posY, &victreebelSprite56x56[0][0],&poke_buffer[0]);
    drawVictrebelAt(posX,posY, &victreebelSprite56x56_mask[0][0],&poke_buffer_mask[0]);
}
else
{
    drawVictrebelAt(posX,posY, &victreebelSprite56x56[1][0],&poke_buffer[0]);
    drawVictrebelAt(posX,posY, &victreebelSprite56x56_mask[1][0],&poke_buffer_mask[0]);
}

    for (int i =0;i<OledDisplay::DisplayArraySize;i++)
    {
        if(i%2)
        oled_buffer[i] &= 0x55;
        else oled_buffer[i]  &= (~0x55);
     poke_buffer[i] = (oled_buffer[i] & ~poke_buffer_mask[i]) | (poke_buffer[i] & poke_buffer_mask[i]);
    //   oled_buffer[i] = (poke_buffer[i]);
    }

memcpy(&oled_buffer[0], &poke_buffer[0], OledDisplay::DisplayArraySize); 

}

/*************************************************************************** */

static void OledSendCmd(uint8_t cmd) 
{
    I2C::write_bytes(HwConfig::I2cAddr::Oled, OLED_CONTROL_BYTE_CMD_STREAM, &cmd, 1);
}
// Funkcja mapująca piksel bezpośrednio na specyficzny układ pamięci stron SSD1306
void OledDrawPixel(int x, int y, uint8_t color, uint8_t*buffer) 
{
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    
    // Kluczowe rzutowanie matematyczne: SSD1306 dzieli ekran pionowo na 8 stron
    if (color) {
        buffer[x + (y / 8) * 128] |= (1 << (y % 8));
    } else {
        buffer[x + (y / 8) * 128] &= ~(1 << (y % 8));
    }
}

/* 128 rows, 8 column */
void drawDigit(  const int row, const int column, const int digit)
{
      int bufferIndex = column + OledDisplay::DisplayWidth_px*row;
 //   if (digit>1) return;

   uint8_t size = (column>(OledDisplay::DisplayWidth_px-Digit::width))?(OledDisplay::DisplayWidth_px-column):Digit::width; 
  //  if ((bufferIndex+OledDisplay::DisplayWidth_px+size)<OledDisplay::DisplaySize_px)
        memcpy(&oled_buffer[bufferIndex], &BmpDigits8x16[digit][0], size); 
    /* second row of digit*/
    bufferIndex+=OledDisplay::DisplayWidth_px;
   // if ((bufferIndex+size)<OledDisplay::DisplaySize_px)
       memcpy(&oled_buffer[bufferIndex], &BmpDigits8x16[digit][Digit::width], Digit::width); 


}

void drawIco16x16(  const int row, const int column, const uint8_t* ico)
{
       int bufferIndex = column + OledDisplay::DisplayWidth_px*row;
 //   if (digit>1) return;

 //  uint8_t size = (column>(OledDisplay::DisplayWidth_px-Digit::width))?(OledDisplay::DisplayWidth_px-column):Digit::width; 
  //  if ((bufferIndex+OledDisplay::DisplayWidth_px+size)<OledDisplay::DisplaySize_px)
        memcpy(&oled_buffer[bufferIndex], ico, 16); 
    /* second row of digit*/
    bufferIndex+=OledDisplay::DisplayWidth_px;
   // if ((bufferIndex+size)<OledDisplay::DisplaySize_px)
       memcpy(&oled_buffer[bufferIndex], ico+16, 16); 
}

static void drawNumber8x16_RightAlign(  const int row, const int column_px, const int number)
{
    constexpr int DIGIT_OFFSET_PX = 8;
    int digitsNmb = 1; /* number of digits to show - without leading zeros */

    int digits[5]{};

    if (number == SENSOR_ERROR)
    {
        drawDigit(row, column_px, 14 );
        return;
    }


    digits[0] = number % 10;          // 1
    digits[1] = (number / 10) % 10;   // 2
    digits[2] = (number / 100) % 10;  // 3
    digits[3] = (number / 1000) % 10; // 4
    digits[4] = (number / 10000) % 10;// 5

    for(int i=4;i>0;i--)
    {  
        if(digits[i]==0)
        {
            continue;
        }
        digitsNmb++;
    }

    for (int i=00;i<5;i++)
    {
        if(i==digitsNmb) return;
        drawDigit(row, column_px-DIGIT_OFFSET_PX*i, digits[i] );
    }
}
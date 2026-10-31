#include "common.hpp"

#include "HwInput.hpp"
#include "WifiManager.hpp"
#include "controller.hpp"
#include "OledDisplay.hpp"



void controller::init()
{

}
 // 2. Parametry startowe fizyki naszej "gry"
    int posX = 32;       // Startowa pozycja X (środek ekranu)
    int posY = 0;        // Startowa pozycja Y
    int speedX = 2;      // Prędkość w poziomie (piksele na klatkę)
    int speedY = 1;      // Prędkość w pionie

    // Rozmiary obiektów do sprawdzania kolizji
    const int displayWidth = 128;
    const int displayHeight = 64;
    const int spriteWidth = 64;
    const int spriteHeight = 64;

void controller::periodic()
{
//  OledDisplay::drawVictrebel();

// Aktualizacja współrzędnych
        posX += speedX;
        posY += speedY;

        // KOLIZJA Z LEWĄ LUB PRAWĄ ŚCIANĄ
        if (posX <= 0 || posX >= (displayWidth - spriteWidth)) {
            speedX = -speedX; // Odwróć ruch w poziomie
        }

        // KOLIZJA Z GÓRNĄ LUB DOLNĄ ŚCIANĄ 
        // (Wysokość ekranu to 64 i sprite to 64, więc w pionie na ekranie 128x64 
        // ruch będzie minimalny lub zerowy, zależnie od ustawienia posY)
        if (posY < 0 || posY > (displayHeight - spriteHeight)) {
            speedY = -speedY; // Odwróć ruch w pionie
        }

        // Wyrenderowanie klatki na podstawie aktualnych współrzędnych
        OledDisplay::drawVictrebelAt(posX, posY);

}
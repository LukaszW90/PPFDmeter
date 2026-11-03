enum BatteryLevel
{
    BAT_EMPTY,
    BAT_25,
    BAT_50,
    BAT_75,
    BAT_FULL,

    BAT_COUNT
};

const uint8_t batteryIcons[BAT_COUNT][8];


-----------------------------------

struct MeasurementData
{
    ...

    float dliSession;
    float dliTotal;
};

Display::printNumber10x16RightAlign(
    int value,
    int row,
    int column);

struct Sprite
{
    const uint8_t* bitmap;
  //  const uint8_t* mask;
    uint8_t width;
    uint8_t height;
};
drawSprite(...)
    

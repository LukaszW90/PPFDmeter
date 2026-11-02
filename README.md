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

------------------------------------------------------

dst = (background & ~mask)
    | (pokemon & mask);
    
for(...)
{
    pixel =
        (backgroundPixel & !maskPixel)
        | (pokemonPixel & maskPixel);
}

    dstByte =
    (bgByte & ~maskByte)
    | (spriteByte & maskByte);

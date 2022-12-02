#ifndef _SHOP_H
#define _SHOP_H
#define COLOUR_RED 0
#define COLOUR_GREEN 1
#define COLOUR_BLUE 2
#define COLOUR_CYAN 3
#define COLOUR_MAGNETA 4
#define COLOUR_YELLOW 5
#define COLOUR_PINK 6
#define COLOUR_PURPLE 7
#define COLOUR_WHITE 8
#define COLOUR_ORANGE 9
#define COLOUR_BLACK 10
#define COLOUR_DARK_BLUE 11
#define COLOUR_SKY_BLUE 12
#define COLOUR_CHARTREUSE 13
#define COLOUR_GREY 14
#define COLOUR_GOLD 15
#define MODE_HIGH_GRAVITY 1
#define MODE_HIGH_SPEED 2
#define MODE_CLOUDS 4
#define MODE_ONE_HIT 8
#define MODE_EVERYTHING 16
#define MODE_MIRROR 32
#define MODE_ENDLESS 64
#define MODE_HEARTLESS 128
#define MODE_PROTECT 256
#define MODE_TIME_TRIAL 512
#define MODE_JUMP 1024
#define MODE_TIME 2048
#define MODE_LEGACY 4096
#define MODE_SLIDING 8192
#define MODE_SPINNING 16384
#define MODE_GAUNTLET 32768
const float COLOUR_VALUES[16][3];
unsigned char selectedColour;
unsigned short selectedModes;
void resetShop();
void tickShop();
unsigned short isModeEnabled(unsigned short);
void renderShop();
void shopTouchEnd(int, int);
#endif

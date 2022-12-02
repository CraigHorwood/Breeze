extern "C"
{
#include "save.h"
#include "title.h"
}
#import <UIKit/UIKit.h>
NSUserDefaults* defaults;
void loadGame()
{
    defaults = [NSUserDefaults alloc];
    [defaults initWithSuiteName:@"group.com.craighorwood.breeze"];
    if ([defaults objectForKey:@"Breeze_TopScore"] != nil)
    {
        topScore = [defaults integerForKey:@"Breeze_TopScore"];
        coins = [defaults integerForKey:@"Breeze_Coins"];
        bought = [defaults integerForKey:@"Breeze_Bought"];
    }
}
void saveGame()
{
    [defaults setInteger:topScore forKey:@"Breeze_TopScore"];
    [defaults setInteger:coins forKey:@"Breeze_Coins"];
    [defaults setInteger:bought forKey:@"Breeze_Bought"];
    [defaults synchronize];
}

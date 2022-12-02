extern "C"
{
    #include "motion.h"
    float xAccel, yAccel, zAccel;
    float xAccelMax, yAccelMax, zAccelMax;
    void getAccel(float* accel)
    {
        accel[0] = xAccel;
        accel[1] = yAccel;
        accel[2] = zAccel;
    }
}
#import <CoreMotion/CoreMotion.h>
CMMotionManager* motionManager;
@interface Accelerator: NSObject
- (void) setAcceleration:(CMAcceleration) acceleration;
@end
@implementation Accelerator
- (void) setAcceleration:(CMAcceleration) acceleration
{
    xAccel = acceleration.x;
    yAccel = acceleration.y;
    zAccel = acceleration.z;
}
@end
void initMotion()
{
    xAccel = 0;
    yAccel = 0;
    zAccel = 0;
    xAccelMax = 0;
    yAccelMax = 0;
    zAccelMax = 0;
    motionManager = [[CMMotionManager alloc] init];
    motionManager.accelerometerUpdateInterval = 0.015625;
    Accelerator* acc = [[Accelerator alloc] init];
    [motionManager startAccelerometerUpdatesToQueue:[NSOperationQueue currentQueue]
                                        withHandler:^(CMAccelerometerData* accelerometerData, NSError* error) {
                                            [acc setAcceleration: accelerometerData.acceleration];
    }];
}

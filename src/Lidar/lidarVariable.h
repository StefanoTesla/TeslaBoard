#ifndef LIDAR_VARIABLE
#define LIDAR_VARIABLE

struct LidarStruct{
    bool safe;
    int speed;
    int angleStep;
    int ptInScan;
};
LidarStruct Lidar;

struct lidarConfig{
    int startAngle;
    int endAngle;
    int minDist;
};

struct lidarSaveConfigStruct{
    bool execute = false;
    bool failed = false;
    bool restartNeeded = false;
};
  
struct lidarLoadConfigStruct{
    bool isValid = false;
};
  
#endif
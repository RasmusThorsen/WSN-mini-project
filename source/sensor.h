#ifndef SENSOR_H
#define SENSOR_H

struct Sensor {
    char name[15];
    int value;
};

struct Sensor read_temperature();
struct Sensor read_humidity();
struct Sensor read_light();

#endif
#include "sensor.h"
#include <string.h>
#include <stdlib.h>

// Import of sensor libraries if they worked correctly in cooja
// Temperature and Humidity
// #include "dev/sht11/sht11-sensor.h"
// Light
// #include "dev/sht11/light-sensor.h"

struct Sensor read_temperature()
{
    struct Sensor temp;
    strncpy(temp.name, "Temperature", 15);

    // If Cooja returned actually real temperatures back indstead of 24
    /*
    float temp_raw = sht11_sensor.value(SHT11_SENSOR_TEMP);
    if(temp_raw != -1) {
        temp.value = (int)((0.01*temp_raw) - 39.60);
    } 
    */

    temp.value = 24;

    return temp;
};

struct Sensor read_humidity()
{
    struct Sensor humidity;
    strncpy(humidity.name, "Humidity", 15);

    // If Cooja Works
    /*
    float humidity_raw = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
    if(humidity_raw != -1) {
        humidity.value = (int)(((0.0405*humidity_raw) - 4) + ((-2.8 * 0.000001)*(pow(humidity_raw,2))));
    }
    */

    humidity.value = 100;

    return humidity;
};

struct Sensor read_light()
{
    struct Sensor light;
    strncpy(light.name, "Light", 15);

    // If Cooja Works
    // There are 2 ways to measure light
    /*
    float light_raw_solar = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
    if(light_raw_solar != -1) {
        light.value = (int)(light_raw_solar * 0.4071);
    }
    */

    light.value = 1000;

    return light;
};


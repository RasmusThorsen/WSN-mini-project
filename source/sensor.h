#ifndef SENSOR_H
#define SENSOR_H

struct Sensor {
    char name[15];
    int intergerValue;
    int decimal;
};


#include "lib/random.h"

// Source https://www.uncg.edu/cmp/downloads/lwsndr.html - SingleHopLabelledReadings - singlehop-outdoor-moteid3-data.txt 
// Data manipulated to be able to import in to Danish version of Excel (dot and comma replacement)
// In Excel the following Formula where used to select the data OFFSET('SHEET'!$A$2;(ROW()-1)*50;0)
// This select roughly every 50 maesurement 
static float temperature[] = {33.19, 32.41, 32.09, 32.47, 31.81, 32.04, 32.24, 31.95, 31.77, 31.22, 31.11, 30.95, 31.24, 31.03, 
                            30.61, 30.38, 30.13, 30.04, 29.98, 29.86, 29.25, 29.19, 28.93, 28.89, 28.79, 28.88, 28.87, 28.72, 
                            28.55, 28.45, 28.21, 28.31, 28.27, 28.30, 28.31, 28.27, 27.83, 27.55, 27.49, 27.36, 27.23, 27.60, 
                            27.18, 27.19, 27.29, 27.33, 27.19, 27.13, 27.09, 27.05, 26.63, 26.48, 26.33, 26.26, 26.08, 25.98, 
                            25.86, 25.68, 25.53, 25.39, 25.19, 25.04, 25.10, 25.07, 25.43, 25.79, 25.73, 25.88, 26.16, 26.16, 
                            26.00, 25.95, 25.73, 25.42, 25.02, 24.83, 24.67, 24.59, 24.53, 24.37, 24.31, 24.16, 24.05, 24.05, 
                            23.99, 23.85, 23.74, 23.63, 23.56, 23.44, 23.37, 23.31, 23.25, 23.16, 23.16, 23.12, 22.95, 22.87, 
                            22.87, 22.84};

static float humidity[] = {36.13, 37.85, 38.05, 38.02, 37.85, 37.88, 38.02, 37.71, 38.70, 39.45, 39.62, 40.13, 39.59, 40.09, 
                            41.21, 41.91, 42.02, 42.32, 42.58, 42.95, 44.55, 44.81, 45.51, 45.67, 46.16, 46.03, 45.93, 46.33, 
                            46.85, 47.11, 47.86, 47.67, 47.77, 47.57, 47.99, 48.16, 49.68, 50.13, 50.51, 51.09, 51.83, 50.61, 
                            51.79, 51.51, 51.35, 50.93, 51.25, 51.67, 51.54, 52.49, 54.17, 54.54, 55.51, 55.64, 56.60, 56.79, 
                            57.25, 57.99, 58.67, 58.70, 58.94, 59.34, 56.08, 56.11, 51.47, 45.51, 43.85, 41.38, 41.07, 40.53, 
                            40.64, 41.04, 41.71, 42.32, 42.99, 43.02, 42.25, 42.45, 42.58, 43.29, 43.35, 43.85, 44.18, 44.52, 
                            44.91, 45.04, 45.04, 44.71, 44.52, 44.58, 44.91, 44.71, 44.52, 44.65, 44.52, 44.52, 45.01, 44.78, 
                            44.91, 45.18};
static int temp_index = 0;
static int humid_index = 0;

int read_temperature(struct Sensor *temp)
{
    strncpy(temp->name, "Temperature", 15);

    // If Cooja returned actually real temperatures back indstead of 24
    /*
    float temp_raw = sht11_sensor.value(SHT11_SENSOR_TEMP);
    if(temp_raw != -1) {
        temp.value = (int)((0.01*temp_raw) - 39.60);
    } 
    */
            
    float temp_raw = temperature[temp_index];

    temp->intergerValue = (int)temp_raw;
    temp->decimal = ((temp_raw - temp->intergerValue) * 100) + ((random_rand() % 100) - 50);

    if(temp->decimal >= 100) {
        temp->intergerValue++;
        temp->decimal -= 100;
    }
    else if(temp->decimal < 0) 
    {
        temp->intergerValue--;
        temp->decimal += 100;
    }  
   
    temp_index++;

    if (temp_index >= sizeof(temperature)/sizeof(*temperature))
    {
        temp_index = 0;
    }     
    return 0;
};

int read_humidity(struct Sensor *humid)
{
    strncpy(humid->name, "Humidity", 15);

    // If Cooja Worked with humidity
    /*
    float humidity_raw = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
    if(humidity_raw != -1) {
        humidity.value = (int)(((0.0405*humidity_raw) - 4) + ((-2.8 * 0.000001)*(pow(humidity_raw,2))));
    }
    */
            
    float humidity_raw = humidity[humid_index];

    humid->intergerValue = (int)humidity_raw;
    humid->decimal = ((humidity_raw - humid->intergerValue) * 100) + ((random_rand() % 100) - 50);

    if(humid->decimal >= 100) {
        humid->intergerValue++;
        humid->decimal -= 100;
    }
    else if(humid->decimal < 0) 
    {
        humid->intergerValue--;
        humid->decimal += 100;
    }  
   
    humid_index++;

    if (humid_index >= sizeof(humidity)/sizeof(*humidity))
    {
        humid_index = 0;
    }     
    return 0;
};

#endif
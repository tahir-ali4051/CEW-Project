#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(int argc, char *argv[]) {
    system("clear");
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <City_Name>\n", argv[0]);
        return 1;
    }

    const char *api_key = "ce18c3b2bf0093492bf3b15733e685ca";
    char city[100];
    strncpy(city, argv[1], sizeof(city) - 1);
    city[sizeof(city) - 1] = '\0';

    // Format the city name
    format_city_name(city);

    // Fetch weather data
    char raw_data_filename[] = "raw_data.json";
    if (!fetch_weather_data(api_key, city, raw_data_filename)) {
        fprintf(stderr, "Failed to fetch weather data.\n");
        return 1;
    }

    // Append raw data to history
    if (!append_to_history(raw_data_filename, "raw_data_history.json")) {
        fprintf(stderr, "Failed to append raw data to history.\n");
        return 1;
    }

    // Parse weather data
    WeatherData weather;
    if (!parse_weather_data(raw_data_filename, &weather)) {
        fprintf(stderr, "Failed to parse weather data.\n");
        return 1;
    }

    // Save processed data
    save_processed_data("processed_data_history.txt", &weather);

    // Display current weather
    printf("\nCurrent Weather for %s:\n", city);
    printf("----------------------------\n");
    printf("Temperature: %.2f°C\n", weather.temperature);
    printf("Condition: %s\n", weather.description);
    printf("----------------------------\n");

    // Temperature alerts with Zenity pop-ups
    if (weather.temperature > 30) {
        printf("ALERT: High Temperature! %.2f°C\n", weather.temperature);
        system("zenity --warning --text='ALERT: High Temperature!'");
    }
    if (weather.temperature < 10) {
        printf("ALERT: Low Temperature! %.2f°C\n", weather.temperature);
        system("zenity --warning --text='ALERT: Low Temperature!'");
    }

    // Check and calculate the average
    if (is_average_calculation_time("processed_data_history.txt")) {
        float avg_temp = calculate_average_temperature("processed_data_history.txt");
        if (avg_temp != -1) {
            printf("\nAverage Temperature for the last 24 readings: %.2f°C\n", avg_temp);
            save_average_temperature("average_temperatures.txt", avg_temp);
        }
    }

    return 0;
}

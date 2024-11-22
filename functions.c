#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "functions.h"

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    strcat((char *)stream, (char *)ptr);
    return size * nmemb;
}

void format_city_name(char *city) {
    if (city[0] >= 'a' && city[0] <= 'z') {
        city[0] = city[0] - 'a' + 'A';
    }
    for (int i = 1; city[i] != '\0'; i++) {
        if (city[i] >= 'A' && city[i] <= 'Z') {
            city[i] = city[i] - 'A' + 'a';
        }
    }
}

int fetch_weather_data(const char *api_key, const char *city, const char *filename) {
    CURL *curl;
    CURLcode res;
    char url[512], response[4096] = {0};

    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric", city, api_key);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
        return 0;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Failed to save raw data.\n");
        return 0;
    }
    fprintf(file, "%s\n", response);
    fclose(file);
    return 1;
}

int append_to_history(const char *raw_data_file, const char *history_file) {
    FILE *source = fopen(raw_data_file, "r");
    FILE *history = fopen(history_file, "a");
    if (!source || !history) {
        fprintf(stderr, "Failed to open raw or history file.\n");
        if (source) fclose(source);
        return 0;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), source)) {
        fprintf(history, "%s", buffer);
    }

    fclose(source);
    fclose(history);
    return 1;
}

int parse_weather_data(const char *filename, WeatherData *weather) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }

    char buffer[4096];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON.\n");
        return 0;
    }

    cJSON *main = cJSON_GetObjectItem(json, "main");
    cJSON *weather_array = cJSON_GetObjectItem(json, "weather");

    if (main && weather_array) {
        weather->temperature = cJSON_GetObjectItem(main, "temp")->valuedouble;
        strncpy(weather->description, cJSON_GetObjectItem(cJSON_GetArrayItem(weather_array, 0), "description")->valuestring, sizeof(weather->description) - 1);
    } else {
        cJSON_Delete(json);
        return 0;
    }

    cJSON_Delete(json);
    return 1;
}

void save_processed_data(const char *filename, WeatherData *weather) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Failed to save processed data.\n");
        return;
    }

    fprintf(file, "Temperature: %.2f°C\n", weather->temperature);
    fprintf(file, "Condition: %s\n", weather->description);
    fprintf(file, "----------------------------\n");
    fclose(file);
}

int is_average_calculation_time(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    int lines = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }
    fclose(file);

    return (lines / 3) % 24 == 0;  // Assuming 3 lines per reading
}

float calculate_average_temperature(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    float total = 0, temp;
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "Temperature: %f°C", &temp) == 1) {
            total += temp;
            count++;
        }
    }
    fclose(file);

    return count > 0 ? total / count : -1;
}

void save_average_temperature(const char *filename, float avg_temp) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Failed to save average temperature.\n");
        return;
    }
    fprintf(file, "Average Temperature: %.2f°C\n", avg_temp);
    fclose(file);
}

#include <iostream>
#include <iomanip>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct WeatherData {
    std::string city;
    std::string date;
    std::string description;
    double temperature;
    double humidity;
    double windSpeed;
    double pressure;
    WeatherData* next;
};

const std::string API_URL = "http://api.weatherbit.io/v2.0/forecast/daily?key=2050bb0febcd479d91fc0ef529e288ff&city=";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

WeatherData* fetchDataForCity(const std::string& city) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        return nullptr;
    }

    std::string fullApiUrl = API_URL + city + "&days=7";
    curl_easy_setopt(curl, CURLOPT_URL, fullApiUrl.c_str());

    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to perform HTTP request: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return nullptr;
    }

    WeatherData* head = nullptr;
    WeatherData* tail = nullptr;

    try {
        json jsonData = json::parse(responseData);

        if (jsonData.find("data") != jsonData.end() && !jsonData["data"].empty()) {
            const json& forecastData = jsonData["data"];

            for (const json& dayData : forecastData) {
                std::string date = dayData["datetime"];
                std::string description = dayData["weather"]["description"];
                double temperature = dayData["temp"];
                double humidity = dayData["rh"];
                double windSpeed = dayData["wind_spd"];
                double pressure = dayData["pres"];

                WeatherData* newData = new WeatherData;
                newData->city = city;
                newData->date = date;
                newData->description = description;
                newData->temperature = temperature;
                newData->humidity = humidity;
                newData->windSpeed = windSpeed;
                newData->pressure = pressure;
                newData->next = nullptr;

                if (!head) {
                    head = newData;
                    tail = newData;
                } else {
                    tail->next = newData;
                    tail = newData;
                }
            }
        } else {
            std::cerr << "No weather data found for the specified city." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    curl_easy_cleanup(curl);

    return head;
}

void displayWeatherData(WeatherData* data, const std::string& city) {
    // Print header
    std::cout << std::left << std::setw(12) << "Date"
              << std::setw(25) << "Description"
              << std::setw(18) << "Temperature (°C)"
              << std::setw(14) << "Humidity (%)"
              << std::setw(20) << "Wind Speed (m/s)"
              << std::setw(15) << "Pressure (mb)"
              << std::endl;

    // Print data
    while (data) {
        std::cout << std::left << std::setw(12) << data->date
                  << std::setw(25) << data->description
                  << std::setw(18) << std::fixed << std::setprecision(2) << data->temperature
                  << std::setw(14) << std::fixed << std::setprecision(2) << data->humidity
                  << std::setw(20) << std::fixed << std::setprecision(2) << data->windSpeed
                  << std::setw(15) << std::fixed << std::setprecision(2) << data->pressure
                  << std::endl;

        data = data->next;
    }
    std::cout << "-------------------" << std::endl;
}

void freeWeatherData(WeatherData* data) {
    WeatherData* current = data;
    while (current) {
        WeatherData* next = current->next;
        delete current;
        current = next;
    }
}

int main() {
    WeatherData* city1Weather = nullptr;
    WeatherData* city2Weather = nullptr;

    int choice;
    std::string city1, city2;

    do {
        std::cout << "\nWELCOME TO SKYSENSE\n";
        std::cout << "\nMenu:\n";
        std::cout << "1. Fetch weather data for City 1\n";
        std::cout << "2. Fetch weather data for City 2\n";
        std::cout << "3. Display previously fetched weather data for City 1\n";
        std::cout << "4. Display previously fetched weather data for City 2\n";
        std::cout << "5. Compare weather data between two cities\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter the city name for City 1: ";
                std::cin >> city1;
                city1Weather = fetchDataForCity(city1);
                break;

            case 2:
                std::cout << "Enter the city name for City 2: ";
                std::cin >> city2;
                city2Weather = fetchDataForCity(city2);
                break;

            case 3:
                if (city1Weather) {
                    std::cout << "Weather data for " << city1 << ":\n";
                    displayWeatherData(city1Weather, city1);
                } else {
                    std::cout << "No previously fetched weather data available for City 1.\n";
                }
                break;

            case 4:
                if (city2Weather) {
                    std::cout << "Weather data for " << city2 << ":\n";
                    displayWeatherData(city2Weather, city2);
                } else {
                    std::cout << "No previously fetched weather data available for City 2.\n";
                }
                break;

            case 5:
                if (city1Weather && city2Weather) {
                    std::cout << "Comparing weather data between " << city1 << " and " << city2 << ":\n";
                    std::cout << "-------------------------------------\n";
                    std::cout << "Weather data for " << city1 << ":\n";
                    displayWeatherData(city1Weather, city1);
                    std::cout << "-------------------------------------\n";
                    std::cout << "Weather data for " << city2 << ":\n";
                    displayWeatherData(city2Weather, city2);
                    std::cout << "-------------------------------------\n";

                    
                    double avgTempCity1 = 0.0;
                    double avgTempCity2 = 0.0;
                    int numDays = 0;

                    WeatherData* tempData1 = city1Weather;
                    WeatherData* tempData2 = city2Weather;

                    while (tempData1 && tempData2) {
                        avgTempCity1 += tempData1->temperature;
                        avgTempCity2 += tempData2->temperature;
                        tempData1 = tempData1->next;
                        tempData2 = tempData2->next;
                        numDays++;
                    }

                    avgTempCity1 /= numDays;
                    avgTempCity2 /= numDays;

                    std::cout << "-------------------------------------\n";
                    std::cout << "Average Temperature for " << city1 << ": " << std::fixed << std::setprecision(2) << avgTempCity1 << "°C\n";
                    std::cout << "Average Temperature for " << city2 << ": " << std::fixed << std::setprecision(2) << avgTempCity2 << "°C\n";
                    std::cout << "-------------------------------------\n";

                    
                    if (avgTempCity1 > avgTempCity2) {
                        std::cout << city1 << " is hotter on average.\n";
                    } else if (avgTempCity1 < avgTempCity2) {
                        std::cout << city2 << " is hotter on average.\n";
                    } else {
                        std::cout << "Average temperatures are the same for both cities.\n";
                    }
                } else {
                    std::cout << "Insufficient weather data available for comparison. Please fetch data for both cities.\n";
                }
                break;

            case 6:
                std::cout << "Exiting the program.\n";
                freeWeatherData(city1Weather);
                freeWeatherData(city2Weather);
                break;

            default:
                std::cout << "Invalid choice. Please enter a valid option.\n";
                break;
        }
    } while (choice != 6);

    return 0;
}

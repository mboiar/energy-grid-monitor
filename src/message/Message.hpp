#pragma once

#include <string>
#include <variant>

struct WeatherData {
  double avg_temperature;
  double avg_sunlight;
  double avg_wind;
};

struct GridData {
  std::string demand;
  std::string generation;
  std::string frequency;
};

using Message = std::variant<WeatherData, GridData>;
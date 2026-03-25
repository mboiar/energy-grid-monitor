#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

struct MqttConfig {
  std::string broker;
  uint16_t port;
  std::string username;
  std::string password;
  std::string topic;
};

struct AppConfig {
  MqttConfig mqtt;
};

AppConfig load_config(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open config file: " + filename);
  }
  json j;
  file >> j;

  AppConfig cfg;
  // MQTT section
  cfg.mqtt.broker = j["mqtt"]["broker"].get<std::string>();
  cfg.mqtt.port = j["mqtt"]["port"].get<uint16_t>();
  cfg.mqtt.username = j["mqtt"]["username"].get<std::string>();
  cfg.mqtt.password = j["mqtt"]["password"].get<std::string>();
  cfg.mqtt.topic = j["mqtt"]["topic"].get<std::string>();

  return cfg;
}

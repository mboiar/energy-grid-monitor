#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

template <typename T>
T get_or_default(const json &j, const std::string &key,
                 const T &default_value) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null()) {
    return default_value;
  }
  return it->get<T>();
}

struct LoadData {
  std::string dtime;
  double load;

  static LoadData from_json(const json &j) {
    LoadData r;
    r.dtime = j.value("dtime", "");
    r.load = get_or_default<int>(j, "load_actual", 0);
    return r;
  }
};

void to_json(json &j, const LoadData &p) {
  j = json{{"dtime", p.dtime}, {"load", p.load}};
}

struct GenData {
  std::string dtime;
  std::vector<std::pair<std::string, double>> power_by_plant;
};

void to_json(json &j, const GenData &p) {
  j = json{{"dtime", p.dtime}, {"power_by_plant", p.power_by_plant}};
}

struct FlowData {
  std::string dtime;
  std::vector<std::pair<std::string, double>> power_by_country;
};

void to_json(json &j, const FlowData &p) {
  j = json{{"dtime", p.dtime}, {"power_by_country", p.power_by_country}};
}

using LoadHistory = std::vector<LoadData>;
using GenHistory = std::vector<GenData>;
using FlowHistory = std::vector<FlowData>;

struct EnergyHistory {
  LoadHistory load;
  GenHistory gen;
  FlowHistory flow;
};

void to_json(json &j, const EnergyHistory &p) {
  j = json{{"load_history", p.load},
           {"gen_history", p.gen},
           {"flow_history", p.flow}};
}

#pragma once

#include <semaphore.h>

#include "http/HttpFetcher.hpp"
#include "message/Message.hpp"
#include "task/PeriodicTask.hpp"
#include "utils/ThreadSafeQueue.hpp"

class PSE_API_Handler : public PeriodicTask {

public:
  PSE_API_Handler(std::chrono::milliseconds interval,
                  std::shared_ptr<sem_t> sem,
                  std::shared_ptr<ThreadSafeQueue<json>> queue,
                  std::shared_ptr<HttpFetcher> http_fetcher)
      : PeriodicTask(interval), sem(sem), pub_queue(queue),
        http_fetcher(http_fetcher) {}
  ~PSE_API_Handler() { stop(); }

protected:
  void callback() override {

    std::string dtime_filter_offset = get_relative_rounded_datetime(60);
    std::string dtime_filter_now = get_relative_rounded_datetime(0);

    // Fetch cross‑border flows
    json flows = fetch_pse_endpoint(
        "przeplywy-mocy",
        {{"select", "dtime,section_code,value"},
         {"filter", "dtime ge '" + dtime_filter_offset + "' and dtime le '" +
                        dtime_filter_now + "'"}}); // -> section_code,
                                                   //   value

    // Fetch load
    json load = fetch_pse_endpoint(
        "kse-load", {{"select", "dtime,load_actual"},
                     {"filter", "dtime ge '" + dtime_filter_offset +
                                    "'"}}); // -> section_code,
                                            //   value

    json payload;
    payload["flow"] = flows;
    payload["load"] = load;

    pub_queue->push(payload);
    sem_post(sem.get());
  }

private:
  std::shared_ptr<sem_t> sem;
  std::shared_ptr<ThreadSafeQueue<json>> pub_queue;
  std::shared_ptr<HttpFetcher> http_fetcher;

  std::string get_relative_rounded_datetime(int offset_min) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    now_t -= offset_min * 60;

    std::tm tm = *std::localtime(&now_t);

    int minutes = tm.tm_min;
    tm.tm_min = (minutes / 15) * 15;
    tm.tm_sec = 0;

    std::time_t final_t = std::mktime(&tm);

    std::tm *final_tm = std::localtime(&final_t);
    std::stringstream ss;
    ss << std::put_time(final_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
  }

  json fetch_pse_endpoint(const std::string &endpoint,
                          const std::map<std::string, std::string> &options) {
    std::string url = "https://api.raporty.pse.pl/api/" + endpoint;
    if (!options.empty()) {
      url += "?";
      for (auto &p : options) {
        url += "$" + p.first + "=" + http_fetcher->url_encode(p.second) + "&";
      }
    }
    url.pop_back();
    std::string response = http_fetcher->get(url);

    return json::parse(response);
  }
};
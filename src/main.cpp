#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <semaphore.h>
#include <string>
#include <tuple>
#include <vector>

#include "Settings.hpp"
#include "http/HttpFetcher.hpp"
#include "message/Message.hpp"
#include "mqtt/MQTTSender.hpp"
#include "pse_api/PSE_API.hpp"
#include "utils/Config.hpp"
#include "utils/ThreadSafeQueue.hpp"

using json = nlohmann::json;

// Global flag to indicate shutdown request
std::atomic<bool> shutdown_requested{false};

// Signal handler for Ctrl+C
void signal_handler(int sig) {
  if (sig == SIGINT) {
    shutdown_requested = true;
  }
}

// Simple usage example
int main() {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  std::signal(SIGINT, signal_handler);

  try {
    AppConfig cfg = load_config("config.json");

    std::shared_ptr<sem_t> fetch_publish_sem = std::make_shared<sem_t>();
    std::shared_ptr<ThreadSafeQueue<json>> message_queue =
        std::make_shared<ThreadSafeQueue<json>>();

    std::cout << "Starting tasks (press Ctrl+C to stop)...\n";

    std::shared_ptr<HttpFetcher> http_fetcher = std::make_shared<HttpFetcher>();
    ;

    PSE_API_Handler pse_handler{std::chrono::seconds(PSE_FETCH_INTERVAL),
                                fetch_publish_sem, message_queue, http_fetcher};
    MqttSender mqtt_sender{
        std::chrono::seconds(PUBLISH_INTERVAL),
        fetch_publish_sem,
        message_queue,
        cfg.mqtt.broker,
        cfg.mqtt.port,
        cfg.mqtt.username,
        cfg.mqtt.password,
        cfg.mqtt.topic,
    };

    pse_handler.start();
    mqtt_sender.start();

    while (!shutdown_requested) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nShutdown requested. Stopping task...\n";
    pse_handler.stop();
    mqtt_sender.stop();

    std::cout << "Tasks stopped. Exiting.\n";

  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  curl_global_cleanup();

  return 0;
}

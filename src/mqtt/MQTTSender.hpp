#pragma once

#include <semaphore.h>

#include "message/Message.hpp"
#include "utils/ThreadSafeQueue.hpp"
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <thread>

#include "task/PeriodicTask.hpp"

class MqttSender : public PeriodicTask {
public:
  MqttSender(std::chrono::milliseconds interval, std::shared_ptr<sem_t> sem,
             std::shared_ptr<ThreadSafeQueue<json>> queue,
             const std::string &broker, uint16_t port,
             const std::string &username, const std::string &password,
             const std::string &topic)
      : PeriodicTask(interval), sem(sem), sub_queue(queue), topic_(topic),
        ioc_(), client_(ioc_) {

    // Set up MQTT client
    client_.brokers(broker, port)
        .credentials(username, password)
        .async_run(boost::asio::detached);

    // Start the io_context in a background thread
    io_thread_ = std::thread([this]() { ioc_.run(); });
  }

  ~MqttSender() override {
    stop();
    ioc_.stop();
    if (io_thread_.joinable())
      io_thread_.join();
  }

protected:
  void callback() override {
    json payload;

    sem_wait(sem.get());

    if (!sub_queue->wait_and_pop_for(payload, std::chrono::milliseconds(0)))
      return;

    std::cout << payload.dump() << std::endl;

    boost::mqtt5::publish_props props;
    props[boost::mqtt5::prop::payload_format_indicator] = uint8_t(1);
    props[boost::mqtt5::prop::topic_alias] = uint16_t(12);
    props[boost::mqtt5::prop::response_topic] = "/test";
    props[boost::mqtt5::prop::subscription_identifier].push_back(40);
    client_.async_publish<boost::mqtt5::qos_e::at_least_once>(
        topic_, payload.dump(), boost::mqtt5::retain_e::no, props,
        [](boost::mqtt5::error_code ec, boost::mqtt5::reason_code rc,
           boost::mqtt5::puback_props props) {
          if (ec) {
            std::cerr << "MQTT publish error: " << ec.message() << std::endl;
          }
        });
  }

private:
  std::shared_ptr<sem_t> sem;
  std::shared_ptr<ThreadSafeQueue<json>> sub_queue;

  std::string topic_;
  boost::asio::io_context ioc_;
  boost::mqtt5::mqtt_client<boost::asio::ip::tcp::socket> client_;
  std::thread io_thread_;
};
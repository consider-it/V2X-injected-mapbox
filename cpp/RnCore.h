#ifndef CONSIDERIT_RNCORE_H
#define CONSIDERIT_RNCORE_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <functional>

#include <mosquitto.h>
#include <loguru.hpp>

#include <NativeCore.h>
#include <Caches.h>

namespace CIT
{
  class RnCore
  {
  public:
    std::shared_ptr<V2X::NativeCore> core = std::make_shared<V2X::NativeCore>();

    /* do not allow copying or moving an instance (for now) */
    RnCore(const RnCore &other) = delete;             // copy ctor
    RnCore(RnCore &&other) = delete;                  // move ctor
    RnCore &operator=(const RnCore &other) = delete;  // copy assigment ctor
    RnCore &operator=(const RnCore &&other) = delete; // move assigment ctor

    ~RnCore();

    RnCore(std::string &serverHostname, int serverPort);
    int connect();
    void switchBroker(std::string &serverHostname);
    void close();
    inline void registerObuInfoCallback(std::function<void(V2X::ObuInfo)> callback) { this->obuCallback = callback; };
    inline void registerGeojsonCallback(Caches::FeatureCollectionType featureCollectionId, std::function<void(std::string)> callback)
    {
      this->core->registerGeojsonCallback(featureCollectionId, callback);
    };
    inline std::vector<V2X::TlSpat> glosa(const GeoJSON::LngLat &ownPos, double heading, unsigned long long obuTimeMs, Intersection::LaneType laneType)
    {
      return this->core->glosa(ownPos, heading, obuTimeMs, laneType);
    };
    inline void updateCache() { this->core->updateCache(); };

  private:
    static void onMessage(struct mosquitto *mosq, void *coreRef, const struct mosquitto_message *message);
    static void onDisconnect(struct mosquitto *mosq, void *coreRef, int rc);
    static bool isConnected;

    std::function<void(V2X::ObuInfo)> obuCallback;
    std::string brokerHostname{};
    int brokerPort{};
    struct mosquitto *client{nullptr};
  };
}

#endif /* CONSIDERIT_RNCORE_H */

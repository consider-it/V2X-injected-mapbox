#ifndef CONSIDERIT_RNCORE_H
#define CONSIDERIT_RNCORE_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
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

        void connect();

        void close();

        inline void updateSettings(V2X::Settings settings)
        {
          this->core->updateSettings(settings);
        }

        inline void registerOnConnectCallback(
            std::function<void(int)> callback) { this->onBrokerConnected = callback; };

        inline void registerObuInfoCallback(
            std::function<void(V2X::ObuInfo)> callback) { this->obuCallback = callback; };

        inline void registerGeojsonCallback(Caches::FeatureCollectionType featureCollectionId,
                                            std::function<void(std::string)> callback)
        {
            this->core->registerGeojsonCallback(featureCollectionId, callback);
        };

        inline std::vector<V2X::TlSpat>
        glosa(const GeoJSON::LngLat &ownPos, double heading, unsigned long long obuTimeMs,
              Intersection::LaneType laneType)
        {
            return this->core->glosa(ownPos, heading, obuTimeMs, laneType);
        };

        inline void updateCache() { this->core->updateCache(); };

    private:
        static void
        onMessage(struct mosquitto *mosq, void *coreRef, const struct mosquitto_message *message);

        static void onDisconnect(struct mosquitto *mosq, void *coreRef, int rc);

        static void onLog(struct mosquitto *mosq, void *obj, int level, const char *str);

        static void onConnect(struct mosquitto *mosq, void *coreRef, int rc);

        bool isConnected = false;

        std::atomic_bool loopShouldStop{false};
        std::thread mosquittoThread;
        std::function<void(V2X::ObuInfo)> obuCallback;
        std::function<void(int)> onBrokerConnected;
        std::string brokerHostname{};
        int brokerPort{};
        struct mosquitto *client{nullptr};
    };
}

#endif /* CONSIDERIT_RNCORE_H */

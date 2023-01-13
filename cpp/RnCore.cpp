#include <stdexcept>
#include <android/log.h>

#include "RnCore.h"

const char *TAG{"RnCore"};
char *const TOPIC_SPATEM = "v2x/rx/spatem";
char *const TOPIC_OBU_INFO = "v2x/rx/obuinfo";
char *const TOPIC_MAPEM = "v2x/rx/mapem";
char *const TOPIC_DENM = "v2x/rx/denm";
char *const TOPIC_CAM = "v2x/rx/cam";
char *const TOPIC_CPM = "v2x/rx/cpm";
char *const TOPIC_IVIM = "v2x/rx/ivim";
char *const TOPIC_DENM_LOOPBACK = "v2x/loopback/denm";
char *const TOPIC_CPM_LOOPBACK = "v2x/loopback/cpm";

void android_log(void *user_data, const loguru::Message &message)
{
	switch (message.verbosity)
	{
	case loguru::Verbosity_ERROR:
		__android_log_print(ANDROID_LOG_ERROR, TAG, "%s%s%s", message.preamble, message.prefix,
							message.message);
		break;
	case loguru::Verbosity_WARNING:
		__android_log_print(ANDROID_LOG_WARN, TAG, "%s%s%s", message.preamble, message.prefix,
							message.message);
		break;
	case loguru::Verbosity_INFO:
		__android_log_print(ANDROID_LOG_INFO, TAG, "%s%s%s", message.preamble, message.prefix,
							message.message);
		break;
	case loguru::Verbosity_MAX:
		__android_log_print(ANDROID_LOG_VERBOSE, TAG, "%s%s%s", message.preamble,
							message.prefix, message.message);
		break;
	default:
		__android_log_print(ANDROID_LOG_DEBUG, TAG, "%s%s%s", message.preamble, message.prefix,
							message.message);
		break;
	}
}

int reconnectionAttemptCount = 0;

CIT::RnCore::RnCore(std::string &serverHostname, int serverPort) : brokerHostname{
																	   std::move(serverHostname)},
																   brokerPort{serverPort}
{
	// setting up loguru to pass logs on to Android's logcat tool
	loguru::g_preamble_header = false;
	loguru::g_preamble_date = false;	// The date field
	loguru::g_preamble_time = false;	// The time of the current day
	loguru::g_preamble_uptime = false;	// The time since init call
	loguru::g_preamble_thread = false;	// The logging thread
	loguru::g_preamble_file = true;		// The file from which the log originates from
	loguru::g_preamble_verbose = false; // The verbosity field
	loguru::g_preamble_pipe = false;	// The pipe symbol right before the message

	loguru::remove_all_callbacks();
	loguru::add_callback("android_logger", android_log, nullptr, loguru::Verbosity_INFO);
	loguru::g_stderr_verbosity = loguru::Verbosity_INFO;

	mosquitto_lib_init();
	this->client = mosquitto_new(NULL, true, this);
	if (nullptr == this->client)
	{
		LOG_F(ERROR, "Could not setup MQTT library");
		throw std::runtime_error("MQTT library setup: Out of memory");
	}
}

CIT::RnCore::~RnCore()
{
	if (this->isConnected)
	{
		mosquitto_disconnect(this->client);
	}
	if (this->mosquittoThread.joinable())
	{
		this->loopShouldStop = true;
		this->mosquittoThread.join();
	}

	mosquitto_destroy(this->client);
	mosquitto_lib_cleanup();
}

void CIT::RnCore::close()
{
	this->core->unregisterGeojsonCallbacks();
	this->obuCallback = nullptr;
	if (this->isConnected)
	{
		mosquitto_disconnect(this->client);
	}
	if (this->mosquittoThread.joinable())
	{
		this->loopShouldStop = true;
		this->mosquittoThread.join();
	}
}

void CIT::RnCore::connect()
{
	if (this->mosquittoThread.joinable())
	{
		this->loopShouldStop = true;
		this->mosquittoThread.join();
	}
	this->mosquittoThread = std::thread([this]
										{
        mosquitto_threaded_set(this->client, true);

        mosquitto_message_callback_set(this->client, RnCore::onMessage);
        mosquitto_connect_callback_set(this->client, RnCore::onConnect);
        mosquitto_disconnect_callback_set(this->client, RnCore::onDisconnect);
        //mosquitto_log_callback_set(this->client, RnCore::onLog);

        int rc = mosquitto_connect_async(this->client, this->brokerHostname.c_str(),
                                         this->brokerPort,
                                         5);
        if (MOSQ_ERR_SUCCESS != rc) {
            LOG_F(INFO, "Connection to MQTT broker failed, reason code %i.", rc);
            if (this->onBrokerConnected != nullptr) {
				this->onBrokerConnected(rc);
			}
        }
        while (!this->loopShouldStop) {
            mosquitto_loop(this->client, -1, 1);
            std::this_thread::sleep_for(std::chrono::microseconds(20));
        } });
}

void CIT::RnCore::onMessage(struct mosquitto *mosq, void *coreRef,
							const struct mosquitto_message *message)
{
	RnCore *rnCore = static_cast<RnCore *>(coreRef);
	if (strcmp(message->topic, "v2x/rx/obuinfo") == 0 && rnCore->obuCallback != nullptr)
	{
		V2X::ObuInfo obuInfo = rnCore->core->decodeObuInfo(message->payload, message->payloadlen);
		rnCore->obuCallback(obuInfo);
	}
	else
	{
		rnCore->core->pushV2xProtoMessage(message->payload, message->payloadlen);
	}
}

void CIT::RnCore::onDisconnect(struct mosquitto *mosq, void *coreRef, int rc)
{
	LOG_F(INFO, "Connection to MQTT broker was lost, reason code %i", rc);
	auto *rnCore = static_cast<RnCore *>(coreRef);
	rnCore->isConnected = false;
	if (rnCore->onBrokerConnected != nullptr)
	{
		rnCore->onBrokerConnected(rc);
	}
}

void CIT::RnCore::onConnect(struct mosquitto *mosq, void *coreRef, int rc)
{
	auto *rnCore = static_cast<RnCore *>(coreRef);
	rnCore->isConnected = true;
	LOG_F(INFO, "Connected to MQTT broker %s", rnCore->brokerHostname.c_str());

	char *const const topics[] =
		{TOPIC_SPATEM, TOPIC_OBU_INFO, TOPIC_MAPEM, TOPIC_DENM, TOPIC_CAM, TOPIC_CPM,
		 TOPIC_IVIM, TOPIC_DENM_LOOPBACK, TOPIC_CPM_LOOPBACK};
	mosquitto_subscribe_multiple(rnCore->client, nullptr, 9, topics, 0, 0, NULL);

	if (rnCore->onBrokerConnected != nullptr)
	{
		rnCore->onBrokerConnected(rc);
	}
}

void CIT::RnCore::onLog(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	LOG_F(INFO, "Mosquitto: %s", str);
}

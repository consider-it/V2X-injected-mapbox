#include <jni.h>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <type_traits>
#include <android/log.h>
#include <loguru.hpp>

#include <NativeCore.h>
#include <Caches.h>
#include <Intersection.h>
#include <GeoJson.h>
#include "RnCore.h"

static std::shared_ptr<CIT::RnCore> core = nullptr;
JavaVM *jvm;
jmethodID obuInfoCb;
jmethodID geoJsonCb;
jobject rnCoreModuleInstance;

void sendObuInfoToJava(V2X::ObuInfo obuInfo)
{
    JNIEnv *mosquittoEnv;
    int getEnvStat = jvm->GetEnv((void **)&mosquittoEnv, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "RnCore", "GetEnv: not attached. Attempting to attach...");
        if (jvm->AttachCurrentThread(&mosquittoEnv, NULL) != 0)
        {
            __android_log_print(ANDROID_LOG_ERROR, "RnCore", "Failed to attach");
        }
    }
    else if (getEnvStat == JNI_OK)
    {
        mosquittoEnv->CallVoidMethod(rnCoreModuleInstance, obuInfoCb, obuInfo.lon, obuInfo.lat, obuInfo.heading, obuInfo.kphSpeed, obuInfo.time);
    }
    else if (getEnvStat == JNI_EVERSION)
    {
        __android_log_print(ANDROID_LOG_ERROR, "RnCore", "GetEnv: version not supported");
    }
}

void sendGeoJSONToJava(int fcType, std::string &geojson)
{
    JNIEnv *mosquittoEnv;
    int getEnvStat = jvm->GetEnv((void **)&mosquittoEnv, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "RnCore", "GetEnv: not attached. Attempting to attach...");
        if (jvm->AttachCurrentThread(&mosquittoEnv, NULL) != 0)
        {
            __android_log_print(ANDROID_LOG_ERROR, "RnCore", "Failed to attach");
        }
    }
    else if (getEnvStat == JNI_OK)
    {
        auto size = geojson.length();
        const char *cstrJson = geojson.c_str();
        jbyteArray arr = mosquittoEnv->NewByteArray(size);
        mosquittoEnv->SetByteArrayRegion(arr, 0, size, (jbyte *)cstrJson);
        mosquittoEnv->CallVoidMethod(rnCoreModuleInstance, geoJsonCb, fcType, arr);
    }
    else if (getEnvStat == JNI_EVERSION)
    {
        __android_log_print(ANDROID_LOG_ERROR, "RnCore", "GetEnv: version not supported");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_mapbox_rctmgl_modules_RCTMGLModule_nativeInit(JNIEnv *env, jobject callingObject, jstring brokerUri)
{
    env->GetJavaVM(&jvm);
    std::string uri{env->GetStringUTFChars(brokerUri, NULL)};
    core = std::make_shared<CIT::RnCore>(uri, 1883);
    rnCoreModuleInstance = env->NewGlobalRef(callingObject);
    jclass rnCoreModule = env->GetObjectClass(rnCoreModuleInstance);
    obuInfoCb = env->GetMethodID(rnCoreModule, "javaObuInfoCallback", "(DDDDD)V");
    geoJsonCb = env->GetMethodID(rnCoreModule, "javaGeoJSONCallback", "(I[B)V");
    core->registerObuInfoCallback(&sendObuInfoToJava);
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_GREEN_LANES, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_GREEN_LANES, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_RED_LANES, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_RED_LANES, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_YELLOW_LANES, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_YELLOW_LANES, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_DARK_LANES, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_DARK_LANES, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_INTERSECTION_REFPT, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_INTERSECTION_REFPT, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_DENM, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_DENM, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_CAM, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_CAM, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_CPM_DETECTION_AREAS, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_CPM_DETECTION_AREAS, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_CPM_SENSOR_LOCATIONS, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_CPM_SENSOR_LOCATIONS, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_CPM_OBJECTS, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_CPM_OBJECTS, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_IVIM, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_IVIM, geojson); });
    core->registerGeojsonCallback(Caches::FeatureCollectionType::FC_MISC, [](std::string geojson)
                                  { sendGeoJSONToJava(Caches::FeatureCollectionType::FC_MISC, geojson); });
    core->connect();
}

extern "C" JNIEXPORT void JNICALL
Java_com_mapbox_rctmgl_modules_RCTMGLModule_nativeSwitchBroker(JNIEnv *env, jobject callingObject, jstring brokerUri)
{
    std::string uri{env->GetStringUTFChars(brokerUri, NULL)};
    core->switchBroker(uri);
}

extern "C" JNIEXPORT void JNICALL
Java_com_mapbox_rctmgl_modules_RCTMGLModule_nativeUpdateCache(JNIEnv *env, jobject thiz)
{
    core->updateCache();
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_mapbox_rctmgl_modules_RCTMGLModule_nativeGlosa(JNIEnv *env, jobject thiz, jdouble lon, jdouble lat, jdouble heading, jdouble obuTime, jint laneType)
{
    auto lngLat = GeoJSON::LngLat{lon, lat};
    auto spats = core->glosa(lngLat, heading, (unsigned long)obuTime, static_cast<Intersection::LaneType>(laneType));

    jclass tlSpatClass = env->FindClass("com/mapbox/rctmgl/models/TlSpat");
    jmethodID spatConstructor = env->GetMethodID(tlSpatClass, "<init>", "(IIII)V");

    jobjectArray tlSpats = (jobjectArray)env->NewObjectArray(spats.size(), tlSpatClass, NULL);

    for (int i = 0; i < spats.size(); i++)
    {
        env->SetObjectArrayElement(tlSpats, i, env->NewObject(tlSpatClass, spatConstructor, spats[i].direction, spats[i].phase, spats[i].remainingSec, spats[i].advisedSpeedKph));
    }

    return tlSpats;
}

extern "C" JNIEXPORT void JNICALL
Java_com_mapbox_rctmgl_modules_RCTMGLModule_nativeClose(JNIEnv *env, jclass clazz)
{
    core->close();
    env->DeleteGlobalRef(rnCoreModuleInstance);
}
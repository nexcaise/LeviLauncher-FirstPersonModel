#pragma once

#ifdef __ANDROID__
    #include <android/log.h>
    #define LOG_TAG "FirstPersonModel"
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
    #define LOGI(...) printf(__VA_ARGS__)
    #define LOGD(...) printf(__VA_ARGS__)
    #define LOGE(...) printf(__VA_ARGS__)
#endif
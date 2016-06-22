/*
 * android-specific implementation of the AdjustMarmalade extension.
 * Add any platform-specific functionality here.
 */
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
#include "AdjustMarmalade_internal.h"

#include <jni.h>
#include "IwDebug.h"
#include "s3eEdk.h"
#include "s3eEdk_android.h"
#include "rapidjson/document.h"

static jobject g_Obj;
static jmethodID g_adjust_Start;
static jmethodID g_adjust_TrackEvent;
static jmethodID g_adjust_SetEnabled;
static jmethodID g_adjust_IsEnabled;
static jmethodID g_adjust_SetOfflineMode;
static jmethodID g_adjust_OnPause;
static jmethodID g_adjust_OnResume;
static jmethodID g_adjust_SetReferrer;

char* get_json_string(rapidjson::Document &jsonDoc, const char* member_name)
{
    if (!jsonDoc.HasMember(member_name)) {
        return NULL;
    }

    const char* source = jsonDoc[member_name].GetString();
    char* target = adjust_CopyString(source);
    
    return target;
}

adjust_attribution_data* get_attribution_data(const char* jsonCString)
{
    adjust_attribution_data* attribution;
    rapidjson::Document jsonDoc;

    if (jsonDoc.Parse<0>(jsonCString).HasParseError()) {
        IwTrace(ADJUSTMARMALADE,("Error parsing response data json"));
        
        return NULL;
    }

    attribution = new adjust_attribution_data();
    
    attribution->tracker_token   = get_json_string(jsonDoc, "tracker_token");
    attribution->tracker_name    = get_json_string(jsonDoc, "tracker_name");
    attribution->network         = get_json_string(jsonDoc, "network");
    attribution->campaign        = get_json_string(jsonDoc, "campaign");
    attribution->ad_group        = get_json_string(jsonDoc, "ad_group");
    attribution->creative        = get_json_string(jsonDoc, "creative");
    attribution->click_label     = get_json_string(jsonDoc, "click_label");

    return attribution;
}

void the_attribution_callback(JNIEnv* env, jobject obj, jstring attributionString) 
{
    const char* attributionCString = env->GetStringUTFChars(attributionString, NULL);
    adjust_attribution_data* attribution = get_attribution_data(attributionCString);

    s3eEdkCallbacksEnqueue(S3E_DEVICE_ADJUST,
                           S3E_ADJUST_CALLBACK_ADJUST_ATTRIBUTION_DATA,
                           attribution,
                           sizeof(*attribution),
                           NULL,
                           S3E_FALSE,
                           &adjust_CleanupAttributionCallback,
                           (void*)attribution);
}

jobject create_global_java_dict(const adjust_param_type* params)
{
    if (params == NULL) {
        return NULL;
    }

    // Get JNI env
    JNIEnv* env = s3eEdkJNIGetEnv();

    // Get the HashMap class
    jclass dict_cls = s3eEdkAndroidFindClass("java/util/HashMap");

    // Get its initial size constructor
    jmethodID dict_cons = env->GetMethodID(dict_cls, "<init>", "(I)V");

    // Construct the java class with the default size
    jobject dict_obj = env->NewObject(dict_cls, dict_cons, params->size());

    // Get the put method
    jmethodID put_method = env->GetMethodID(dict_cls, "put", 
        "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    for (adjust_param_type::const_iterator pos = params->begin();pos != params->end(); ++pos) {
        jstring key = env->NewStringUTF(pos->first);
        jstring value = env->NewStringUTF(pos->second);

        env->CallObjectMethod(dict_obj, put_method, key, value);
    }
    
    jobject dict_jglobal = env->NewGlobalRef(dict_obj);

    return dict_jglobal;
}

static int32 applicationUnpaused(void* systemData, void* userData)
{
    adjust_OnResume_platform();

    return 0;
}

static int32 applicationPaused(void* systemData, void* userData)
{
    adjust_OnPause_platform();

    return 0;
}

s3eResult AdjustMarmaladeInit_platform()
{
    // Get the environment from the pointer
    JNIEnv* env = s3eEdkJNIGetEnv();
    jobject obj = NULL;
    jmethodID cons = NULL;

    const JNINativeMethod methods[] =
    {
        {"attributionCallback", "(Ljava/lang/String;)V",(void*)&the_attribution_callback}
    };

    // Get the extension class
    jclass cls = s3eEdkAndroidFindClass("AdjustMarmalade");
    if (!cls)
        goto fail;

    // Get its constructor
    cons = env->GetMethodID(cls, "<init>", "()V");
    if (!cons)
        goto fail;

    // Construct the java class
    obj = env->NewObject(cls, cons);
    if (!obj)
        goto fail;

    // Get all the extension methods
    g_adjust_Start = env->GetMethodID(cls, "adjust_Start", 
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;ZZ)V");
    if (!g_adjust_Start)
        goto fail;

    g_adjust_TrackEvent = env->GetMethodID(cls, "adjust_TrackEvent", 
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;DLjava/util/Map;Ljava/util/Map;Z)V");
    if (!g_adjust_TrackEvent)
        goto fail;

    g_adjust_SetEnabled = env->GetMethodID(cls, "adjust_SetEnabled", "(Z)V");
    if (!g_adjust_SetEnabled)
        goto fail;

    g_adjust_IsEnabled = env->GetMethodID(cls, "adjust_IsEnabled", "()Z");
    if (!g_adjust_IsEnabled)
        goto fail;

    g_adjust_SetOfflineMode = env->GetMethodID(cls, "adjust_SetOfflineMode", "(Z)V");
    if (!g_adjust_SetOfflineMode)
        goto fail;

    g_adjust_OnPause = env->GetMethodID(cls, "adjust_OnPause", "()V");
    if (!g_adjust_OnPause)
        goto fail;

    g_adjust_OnResume = env->GetMethodID(cls, "adjust_OnResume", "()V");
    if (!g_adjust_OnResume)
        goto fail;

    g_adjust_SetReferrer = env->GetMethodID(cls, "adjust_SetReferrer", "(Ljava/lang/String;)V");
    if (!g_adjust_SetReferrer)
        goto fail;

    // Register the native hooks
    if (env->RegisterNatives(cls, methods,sizeof(methods)/sizeof(methods[0])))
        goto fail;

    IwTrace(ADJUSTMARMALADE, ("ADJUSTMARMALADE init success"));
    g_Obj = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);
    env->DeleteGlobalRef(cls);

    // Add any platform-specific initialisation code here
    // if (s3eDeviceRegister(S3E_DEVICE_UNPAUSE, (s3eCallback)applicationUnpaused, NULL) != S3E_RESULT_SUCCESS) {
    //     return S3E_RESULT_ERROR;
    // }

    // if (s3eDeviceRegister(S3E_DEVICE_PAUSE, (s3eCallback)applicationPaused, NULL) != S3E_RESULT_SUCCESS) {
    //     return S3E_RESULT_ERROR;
    // }

    return S3E_RESULT_SUCCESS;

fail:
    jthrowable exc = env->ExceptionOccurred();
    if (exc)
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        IwTrace(AdjustMarmalade, ("One or more java methods could not be found"));
    }

    env->DeleteLocalRef(obj);
    env->DeleteGlobalRef(cls);
    return S3E_RESULT_ERROR;
}

void AdjustMarmaladeTerminate_platform()
{ 
    // Add any platform-specific termination code here
    s3eDeviceUnRegister(S3E_DEVICE_UNPAUSE, (s3eCallback)applicationUnpaused);
    s3eDeviceUnRegister(S3E_DEVICE_PAUSE, (s3eCallback)applicationPaused);

    JNIEnv* env = s3eEdkJNIGetEnv();
    env->DeleteGlobalRef(g_Obj);
    g_Obj = NULL;
}

s3eResult adjust_Start_platform(adjust_config* config)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    jstring jAppToken = env->NewStringUTF(config->app_token);
    jstring jEnvironment = env->NewStringUTF(config->environment);
    jstring jLogLevel = env->NewStringUTF(config->log_level);
    jstring jSdkPrefix = env->NewStringUTF(config->sdk_prefix);
    jstring jProcessName = env->NewStringUTF(config->process_name);
    jstring jDefaultTracker = env->NewStringUTF(config->default_tracker);
    jboolean jIsEventBufferingEnabled = JNI_FALSE;
    jboolean jIsMacMd5TrackingEnabled = JNI_FALSE;
    jboolean jIsAttributionCallbackSet = JNI_FALSE;

    if (config->is_event_buffering_enabled != NULL) {
        jIsEventBufferingEnabled = (jboolean)(*(config->is_event_buffering_enabled));
    }
    
    if (config->is_mac_md5_tracking_enabled != NULL) {
        jIsMacMd5TrackingEnabled = (jboolean)(*(config->is_mac_md5_tracking_enabled));
    }

    if (config->is_attribution_delegate_set != NULL) {
        jIsAttributionCallbackSet = (jboolean)(*(config->is_attribution_delegate_set));

        if (jIsAttributionCallbackSet == JNI_TRUE) {
            EDK_CALLBACK_REG(ADJUST, ADJUST_ATTRIBUTION_DATA, (s3eCallback)config->attribution_callback, NULL, false);
        }
    }

    env->CallVoidMethod(g_Obj, g_adjust_Start, jAppToken, jEnvironment, jLogLevel, jSdkPrefix, 
        jIsEventBufferingEnabled, jProcessName, jDefaultTracker, jIsMacMd5TrackingEnabled, jIsAttributionCallbackSet);

    env->DeleteLocalRef(jAppToken);
    env->DeleteLocalRef(jEnvironment);
    env->DeleteLocalRef(jLogLevel);
    env->DeleteLocalRef(jProcessName);
    env->DeleteLocalRef(jDefaultTracker);

    return (s3eResult)0;
}

s3eResult adjust_TrackEvent_platform(adjust_event* event)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    jstring jEventToken = env->NewStringUTF(event->event_token);
    jstring jCurrency = env->NewStringUTF(event->currency);
    jstring jTransactionId = env->NewStringUTF(event->transaction_id);
    jstring jReceipt = env->NewStringUTF(event->receipt);

    jobject jCallbackParams = create_global_java_dict(event->callback_params);
    jobject jPartnerParams = create_global_java_dict(event->partner_params);

    jdouble jRevenue = event->revenue != NULL ? *(event->revenue) : -1;
    jboolean jIsReceiptSet = event->is_receipt_set != NULL ? *(event->is_receipt_set) : JNI_FALSE;

    env->CallVoidMethod(g_Obj, g_adjust_TrackEvent, jEventToken, jCurrency, jTransactionId, jReceipt,
        jRevenue, jCallbackParams, jPartnerParams, jIsReceiptSet);

    env->DeleteLocalRef(jEventToken);
    env->DeleteLocalRef(jCurrency);
    env->DeleteLocalRef(jTransactionId);
    env->DeleteLocalRef(jReceipt);
    env->DeleteGlobalRef(jCallbackParams);
    env->DeleteGlobalRef(jPartnerParams);

    return (s3eResult)0;
}

s3eResult adjust_SetEnabled_platform(bool is_enabled)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    env->CallVoidMethod(g_Obj, g_adjust_SetEnabled, is_enabled);
    
    return (s3eResult)0;
}

s3eResult adjust_IsEnabled_platform(bool& is_enabled_out)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    jboolean isEnabled_java = env->CallBooleanMethod(g_Obj, g_adjust_IsEnabled);

    is_enabled_out = (bool)isEnabled_java;

    return (s3eResult)0;
}

s3eResult adjust_SetOfflineMode_platform(bool is_offline_mode_enabled)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    env->CallVoidMethod(g_Obj, g_adjust_SetOfflineMode, is_offline_mode_enabled);

    return (s3eResult)0;
}

s3eResult adjust_SetReferrer_platform(const char* referrer)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    jstring jReferrer = env->NewStringUTF(referrer);
    
    env->CallVoidMethod(g_Obj, g_adjust_SetReferrer, jReferrer);
    env->DeleteLocalRef(jReferrer);

    return (s3eResult)0;
}

s3eResult adjust_OnPause_platform()
{
    // JNIEnv* env = s3eEdkJNIGetEnv();

    // env->CallVoidMethod(g_Obj, g_adjust_OnPause);

    return (s3eResult)0;
}

s3eResult adjust_OnResume_platform()
{
    // JNIEnv* env = s3eEdkJNIGetEnv();

    // env->CallVoidMethod(g_Obj, g_adjust_OnResume);

    return (s3eResult)0;
}

s3eResult adjust_SetDeviceToken_platform(const char* device_token)
{
    return (s3eResult)0;
}

// s3eResult adjust_SetAttributionCallback_platform(adjust_attribution_delegate attribution_callback)
// {
//     JNIEnv* env = s3eEdkJNIGetEnv();

//     env->CallVoidMethod(g_Obj, g_adjust_SetAttributionCallback);

//     EDK_CALLBACK_REG(ADJUST, ADJUST_ATTRIBUTION_DATA, (s3eCallback)attribution_callback, NULL, false);

//     return (s3eResult)0;
// }

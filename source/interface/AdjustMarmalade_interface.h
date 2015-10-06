/*
 * WARNING: this is an autogenerated file and will be overwritten by
 * the extension interface script.
 */
/**
 * Definitions for functions types passed to/from s3eExt interface
 */
typedef  s3eResult(*adjust_Start_t)(adjust_config* config);
typedef  s3eResult(*adjust_TrackEvent_t)(adjust_event* event);
typedef  s3eResult(*adjust_SetEnabled_t)(bool is_enabled);
typedef  s3eResult(*adjust_IsEnabled_t)(bool& is_enabled_out);
typedef  s3eResult(*adjust_SetOfflineMode_t)(bool is_offline_mode_enabled);
typedef  s3eResult(*adjust_OnPause_t)();
typedef  s3eResult(*adjust_OnResume_t)();
typedef  s3eResult(*adjust_SetReferrer_t)(const char* referrer);
typedef  s3eResult(*adjust_SetDeviceToken_t)(const char* device_token);
typedef  s3eResult(*adjust_SetAttributionCallback_t)(adjust_attribution_delegate attribution_callback);

/**
 * struct that gets filled in by AdjustMarmaladeRegister
 */
typedef struct AdjustMarmaladeFuncs
{
    adjust_Start_t m_adjust_Start;
    adjust_TrackEvent_t m_adjust_TrackEvent;
    adjust_SetEnabled_t m_adjust_SetEnabled;
    adjust_IsEnabled_t m_adjust_IsEnabled;
    adjust_SetOfflineMode_t m_adjust_SetOfflineMode;
    adjust_OnPause_t m_adjust_OnPause;
    adjust_OnResume_t m_adjust_OnResume;
    adjust_SetReferrer_t m_adjust_SetReferrer;
    adjust_SetDeviceToken_t m_adjust_SetDeviceToken;
    adjust_SetAttributionCallback_t m_adjust_SetAttributionCallback;
} AdjustMarmaladeFuncs;

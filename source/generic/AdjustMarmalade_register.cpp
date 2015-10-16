/*
 * WARNING: this is an autogenerated file and will be overwritten by
 * the extension interface script.
 */
/*
 * This file contains the automatically generated loader-side
 * functions that form part of the extension.
 *
 * This file is awlays compiled into all loaders but compiles
 * to nothing if this extension is not enabled in the loader
 * at build time.
 */
#include "AdjustMarmalade_autodefs.h"
#include "s3eEdk.h"
#include "AdjustMarmalade.h"
#include "AdjustMarmalade_internal.h"

//Declarations of Init and Term functions
extern s3eResult AdjustMarmaladeInit();
extern void AdjustMarmaladeTerminate();


void AdjustMarmaladeRegisterExt()
{
    /* fill in the function pointer struct for this extension */
    void* funcPtrs[9];
    funcPtrs[0] = (void*)adjust_Start;
    funcPtrs[1] = (void*)adjust_TrackEvent;
    funcPtrs[2] = (void*)adjust_SetEnabled;
    funcPtrs[3] = (void*)adjust_IsEnabled;
    funcPtrs[4] = (void*)adjust_SetOfflineMode;
    funcPtrs[5] = (void*)adjust_OnPause;
    funcPtrs[6] = (void*)adjust_OnResume;
    funcPtrs[7] = (void*)adjust_SetReferrer;
    funcPtrs[8] = (void*)adjust_SetDeviceToken;

    /*
     * Flags that specify the extension's use of locking and stackswitching
     */
    int flags[9] = { 0 };

    /*
     * Register the extension
     */
s3eEdkRegister("AdjustMarmalade", funcPtrs, sizeof(funcPtrs), flags, AdjustMarmaladeInit, AdjustMarmaladeTerminate, sizeof(AdjustMarmaladeGlobals));
}

#if !defined S3E_BUILD_S3ELOADER

#if defined S3E_EDK_USE_STATIC_INIT_ARRAY
int AdjustMarmaladeStaticInit()
{
    void** p = g_StaticInitArray;
    void* end = p + g_StaticArrayLen;
    while (*p) p++;
    if (p < end)
        *p = (void*)&AdjustMarmaladeRegisterExt;
    return 0;
}

int g_AdjustMarmaladeVal = AdjustMarmaladeStaticInit();

#elif defined S3E_EDK_USE_DLLS
S3E_EXTERN_C S3E_DLL_EXPORT void RegisterExt()
{
    AdjustMarmaladeRegisterExt();
}
#endif

#endif

#pragma once

#include "CoreTypes.h"

TD_ZIPPY_API DECLARE_LOG_CATEGORY_EXTERN(TD_Log, Log, All);
TD_ZIPPY_API DECLARE_LOG_CATEGORY_EXTERN(TD_Log_CMC, Log, All);
TD_ZIPPY_API DECLARE_LOG_CATEGORY_EXTERN(TD_Log_CMC_Debug, Log, All);

#define Print_Log_NetRole(InActor, CategoryName, Verbosity, Format) \
if (InActor) \
{ \
    if (const UEnum* EnumObject = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENetRole"))) \
    { \
        const FName EnumName = EnumObject->GetNameByValue(static_cast<int64>(InActor->GetLocalRole())); \
        UE_LOG(CategoryName, Verbosity, TEXT("%s[%d]: [LocalRole=%s]: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *EnumName.ToString(), Format); \
    } \
} else { \
    UE_LOG(CategoryName, Verbosity, TEXT("%s[%d]: [传入的Actor为空]: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, Format); \
}

#define Print_Log(CategoryName, Verbosity, Format) \
UE_LOG(CategoryName, Verbosity, TEXT("%s[%d]: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, Format);
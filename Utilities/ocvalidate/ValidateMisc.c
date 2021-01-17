/** @file
  Copyright (C) 2018, vit9696. All rights reserved.
  Copyright (C) 2020, PMheart. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "ocvalidate.h"
#include "OcValidateLib.h"
#include "KextInfo.h"

#include <Library/BaseLib.h>
#include <Library/OcBootManagementLib.h>
#include <Library/OcConfigurationLib.h>
#include <Protocol/OcLog.h>

/**
  Callback funtion to verify whether Arguments and Path are duplicated in Misc->Entries.

  @param[in]  PrimaryEntry    Primary entry to be checked.
  @param[in]  SecondaryEntry  Secondary entry to be checked.

  @retval     TRUE            If PrimaryEntry and SecondaryEntry are duplicated.
**/
STATIC
BOOLEAN
MiscEntriesHasDuplication (
  IN  CONST VOID  *PrimaryEntry,
  IN  CONST VOID  *SecondaryEntry
  )
{
  //
  // NOTE: Entries and Tools share the same constructor.
  //
  CONST OC_MISC_TOOLS_ENTRY           *MiscEntriesPrimaryEntry;
  CONST OC_MISC_TOOLS_ENTRY           *MiscEntriesSecondaryEntry;
  CONST CHAR8                         *MiscEntriesPrimaryArgumentsString;
  CONST CHAR8                         *MiscEntriesSecondaryArgumentsString;
  CONST CHAR8                         *MiscEntriesPrimaryPathString;
  CONST CHAR8                         *MiscEntriesSecondaryPathString;

  MiscEntriesPrimaryEntry             = *(CONST OC_MISC_TOOLS_ENTRY **) PrimaryEntry;
  MiscEntriesSecondaryEntry           = *(CONST OC_MISC_TOOLS_ENTRY **) SecondaryEntry;
  MiscEntriesPrimaryArgumentsString   = OC_BLOB_GET (&MiscEntriesPrimaryEntry->Arguments);
  MiscEntriesSecondaryArgumentsString = OC_BLOB_GET (&MiscEntriesSecondaryEntry->Arguments);
  MiscEntriesPrimaryPathString        = OC_BLOB_GET (&MiscEntriesPrimaryEntry->Path);
  MiscEntriesSecondaryPathString      = OC_BLOB_GET (&MiscEntriesSecondaryEntry->Path);

  if (!MiscEntriesPrimaryEntry->Enabled || !MiscEntriesSecondaryEntry->Enabled) {
    return FALSE;
  }

  if (AsciiStrCmp (MiscEntriesPrimaryArgumentsString, MiscEntriesSecondaryArgumentsString) == 0
    && AsciiStrCmp (MiscEntriesPrimaryPathString, MiscEntriesSecondaryPathString) == 0) {
    DEBUG ((DEBUG_WARN, "Misc->Entries->Arguments: %a is duplicated ", MiscEntriesPrimaryPathString));
    return TRUE;
  }

  return FALSE;
}

/**
  Callback funtion to verify whether Arguments and Path are duplicated in Misc->Tools.

  @param[in]  PrimaryEntry    Primary entry to be checked.
  @param[in]  SecondaryEntry  Secondary entry to be checked.

  @retval     TRUE            If PrimaryEntry and SecondaryEntry are duplicated.
**/
STATIC
BOOLEAN
MiscToolsHasDuplication (
  IN  CONST VOID  *PrimaryEntry,
  IN  CONST VOID  *SecondaryEntry
  )
{
  CONST OC_MISC_TOOLS_ENTRY         *MiscToolsPrimaryEntry;
  CONST OC_MISC_TOOLS_ENTRY         *MiscToolsSecondaryEntry;
  CONST CHAR8                       *MiscToolsPrimaryArgumentsString;
  CONST CHAR8                       *MiscToolsSecondaryArgumentsString;
  CONST CHAR8                       *MiscToolsPrimaryPathString;
  CONST CHAR8                       *MiscToolsSecondaryPathString;

  MiscToolsPrimaryEntry             = *(CONST OC_MISC_TOOLS_ENTRY **) PrimaryEntry;
  MiscToolsSecondaryEntry           = *(CONST OC_MISC_TOOLS_ENTRY **) SecondaryEntry;
  MiscToolsPrimaryArgumentsString   = OC_BLOB_GET (&MiscToolsPrimaryEntry->Arguments);
  MiscToolsSecondaryArgumentsString = OC_BLOB_GET (&MiscToolsSecondaryEntry->Arguments);
  MiscToolsPrimaryPathString        = OC_BLOB_GET (&MiscToolsPrimaryEntry->Path);
  MiscToolsSecondaryPathString      = OC_BLOB_GET (&MiscToolsSecondaryEntry->Path);

  if (!MiscToolsPrimaryEntry->Enabled || !MiscToolsSecondaryEntry->Enabled) {
    return FALSE;
  }

  if (AsciiStrCmp (MiscToolsPrimaryArgumentsString, MiscToolsSecondaryArgumentsString) == 0
    && AsciiStrCmp (MiscToolsPrimaryPathString, MiscToolsSecondaryPathString) == 0) {
    DEBUG ((DEBUG_WARN, "Misc->Tools->Path: %a 是重复的 ", MiscToolsPrimaryPathString));
    return TRUE;
  }

  return FALSE;
}

/**
  Validate if SecureBootModel has allowed value.

  @param[in]  SecureBootModel  SecureBootModel retrieved from user config.

  @retval     TRUE             If SecureBootModel is valid.
**/
STATIC
BOOLEAN
ValidateSecureBootModel (
  IN  CONST CHAR8  *SecureBootModel
  )
{
  UINTN   Index;
  CONST CHAR8 *AllowedSecureBootModel[] = {
    "Default", "Disabled",
    "j137",  "j680",  "j132",  "j174",  "j140k",
    "j780",  "j213",  "j140a", "j152f", "j160",
    "j230k", "j214k", "j223",  "j215",  "j185", "j185f",
    "x86legacy"
  };

  for (Index = 0; Index < ARRAY_SIZE (AllowedSecureBootModel); ++Index) {
    if (AsciiStrCmp (SecureBootModel, AllowedSecureBootModel[Index]) == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

STATIC
UINT32
CheckMiscBoot (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  OC_MISC_CONFIG    *UserMisc;
  OC_UEFI_CONFIG    *UserUefi;
  UINT32            ConsoleAttributes;
  CONST CHAR8       *HibernateMode;
  UINT32            PickerAttributes;
  UINT32            Index;
  CONST CHAR8       *Driver;
  BOOLEAN           HasOpenCanopyEfiDriver;
  CONST CHAR8       *PickerMode;
  CONST CHAR8       *PickerVariant;
  BOOLEAN           IsPickerAudioAssistEnabled;
  BOOLEAN           IsAudioSupportEnabled;

  ErrorCount        = 0;
  UserMisc          = &Config->Misc;
  UserUefi          = &Config->Uefi;

  ConsoleAttributes = UserMisc->Boot.ConsoleAttributes;
  if ((ConsoleAttributes & ~0x7FU) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Boot->ConsoleAttributes设置了未知的位!\n"));
    ++ErrorCount;
  }

  HibernateMode     = OC_BLOB_GET (&UserMisc->Boot.HibernateMode);
  if (AsciiStrCmp (HibernateMode, "None") != 0
    && AsciiStrCmp (HibernateMode, "Auto") != 0
    && AsciiStrCmp (HibernateMode, "RTC") != 0
    && AsciiStrCmp (HibernateMode, "NVRAM") != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Boot->HibernateMode 不太对 (只能是 None, Auto, RTC, 或 NVRAM)!\n"));
    ++ErrorCount;
  }

  PickerAttributes  = UserMisc->Boot.PickerAttributes;
  if ((PickerAttributes & ~OC_ATTR_ALL_BITS) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Boot->PickerAttributes设置了未知位!\n"));
    ++ErrorCount;
  }

  HasOpenCanopyEfiDriver = FALSE;
  for (Index = 0; Index < UserUefi->Drivers.Count; ++Index) {
    Driver = OC_BLOB_GET (UserUefi->Drivers.Values[Index]);

    if (AsciiStrCmp (Driver, "OpenCanopy.efi") == 0) {
      HasOpenCanopyEfiDriver = TRUE;
    }
  }
  PickerMode        = OC_BLOB_GET (&UserMisc->Boot.PickerMode);
  if (AsciiStrCmp (PickerMode, "Builtin") != 0
    && AsciiStrCmp (PickerMode, "External") != 0
    && AsciiStrCmp (PickerMode, "Apple") != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Boot->PickerMode 不正确 (只能是Builtin, External, 或 Apple)!\n"));
    ++ErrorCount;
  } else if (HasOpenCanopyEfiDriver && AsciiStrCmp (PickerMode, "External") != 0) {
    DEBUG ((DEBUG_WARN, "OpenCanopy.efi在UEFI->Drivers中加载，但Misc->Boot->PickerMode未设置为External!\n"));
    ++ErrorCount;
  }

  PickerVariant     = OC_BLOB_GET (&UserMisc->Boot.PickerVariant);
  if (PickerVariant[0] == '\0') {
    DEBUG ((DEBUG_WARN, "Misc->Boot->PickerVariant不能为空!\n"));
    ++ErrorCount;
  }

  IsPickerAudioAssistEnabled = UserMisc->Boot.PickerAudioAssist;
  IsAudioSupportEnabled      = UserUefi->Audio.AudioSupport;
  if (IsPickerAudioAssistEnabled && !IsAudioSupportEnabled) {
    DEBUG ((DEBUG_WARN, "Misc->Boot->PickerAudioAssist已启用，但未完全启用UEFI->Audio->AudioSupport!\n"));
    ++ErrorCount;
  }

  return ErrorCount;
}

STATIC
UINT32
CheckMiscDebug (
  IN  OC_GLOBAL_CONFIG  *Config
  ) 
{
  UINT32              ErrorCount;
  OC_MISC_CONFIG      *UserMisc;
  UINT64              DisplayLevel;
  UINT64              AllowedDisplayLevel;
  UINT64              HaltLevel;
  UINT64              AllowedHaltLevel;
  UINT32              Target;

  ErrorCount          = 0;
  UserMisc            = &Config->Misc;

  //
  // FIXME: Check whether DisplayLevel only supports values within AllowedDisplayLevel, or all possible levels in DebugLib.h?
  //
  DisplayLevel        = UserMisc->Debug.DisplayLevel;
  AllowedDisplayLevel = DEBUG_WARN | DEBUG_INFO | DEBUG_VERBOSE | DEBUG_ERROR;
  if ((DisplayLevel & ~AllowedDisplayLevel) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Debug->DisplayLevel设置了未知位！\n"));
    ++ErrorCount;
  }
  HaltLevel           = DisplayLevel;
  AllowedHaltLevel    = AllowedDisplayLevel;
  if ((HaltLevel & ~AllowedHaltLevel) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Security->HaltLevel 设置了未知位！\n"));
    ++ErrorCount;
  }

  Target              = UserMisc->Debug.Target;
  if ((Target & ~OC_LOG_ALL_BITS) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Debug->Target 设置了未知位！\n"));
    ++ErrorCount;
  }

  return ErrorCount;
}

STATIC
UINT32
CheckMiscEntries (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_MISC_CONFIG    *UserMisc;
  CONST CHAR8       *Arguments;
  CONST CHAR8       *Comment;
  CONST CHAR8       *AsciiName;
  CONST CHAR16      *UnicodeName;
  CONST CHAR8       *Path;

  ErrorCount        = 0;
  UserMisc          = &Config->Misc;

  for (Index = 0; Index < UserMisc->Entries.Count; ++Index) {
    Arguments       = OC_BLOB_GET (&UserMisc->Entries.Values[Index]->Arguments);
    Comment         = OC_BLOB_GET (&UserMisc->Entries.Values[Index]->Comment);
    AsciiName       = OC_BLOB_GET (&UserMisc->Entries.Values[Index]->Name);
    Path            = OC_BLOB_GET (&UserMisc->Entries.Values[Index]->Path);

    //
    // Sanitise strings.
    //
    // NOTE: As Arguments takes identical requirements of Comment,
    //       we use Comment sanitiser here.
    //
    if (!AsciiCommentIsLegal (Arguments)) {
      DEBUG ((DEBUG_WARN, "Misc->Entries[%u]->参数包含非法字符!\n", Index));
      ++ErrorCount;
    }
    if (!AsciiCommentIsLegal (Comment)) {
      DEBUG ((DEBUG_WARN, "Misc->Entries[%u]->Comment包含非法字符!\n", Index));
      ++ErrorCount;
    }
    
    UnicodeName = AsciiStrCopyToUnicode (AsciiName, 0);
    if (UnicodeName != NULL) {
      if (!UnicodeIsFilteredString (UnicodeName, TRUE)) {
        DEBUG ((DEBUG_WARN, "Misc->Entries[%u]->Name包含非法字符!\n", Index));
        ++ErrorCount;
      }

      FreePool ((VOID *) UnicodeName);
    }

    //
    // FIXME: Properly sanitise Path.
    //
    if (!AsciiCommentIsLegal (Path)) {
      DEBUG ((DEBUG_WARN, "Misc->Entries[%u]->Path包含非法字符!\n", Index));
      ++ErrorCount;
    }
  }

  //
  // Check duplicated entries in Entries.
  //
  ErrorCount += FindArrayDuplication (
    UserMisc->Entries.Values,
    UserMisc->Entries.Count,
    sizeof (UserMisc->Entries.Values[0]),
    MiscEntriesHasDuplication
    );

  return ErrorCount;
}

STATIC
UINT32
CheckMiscSecurity (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_KERNEL_CONFIG  *UserKernel;
  OC_MISC_CONFIG    *UserMisc;
  OC_UEFI_CONFIG    *UserUefi;
  BOOLEAN           IsAuthRestartEnabled;
  BOOLEAN           HasVSMCKext;
  CONST CHAR8       *BootProtect;
  BOOLEAN           IsRequestBootVarRoutingEnabled;
  CONST CHAR8       *AsciiDmgLoading;
  UINT32            ExposeSensitiveData;
  CONST CHAR8       *AsciiVault;
  UINT32            ScanPolicy;
  UINT32            AllowedScanPolicy;
  CONST CHAR8       *SecureBootModel;

  ErrorCount        = 0;
  UserKernel        = &Config->Kernel;
  UserMisc          = &Config->Misc;
  UserUefi          = &Config->Uefi;

  HasVSMCKext = FALSE;
  for (Index = 0; Index < UserKernel->Add.Count; ++Index) {
    if (AsciiStrCmp (OC_BLOB_GET (&UserKernel->Add.Values[Index]->BundlePath), mKextInfo[INDEX_KEXT_VSMC].KextBundlePath) == 0) {
      HasVSMCKext = TRUE;
    }
  }
  IsAuthRestartEnabled = UserMisc->Security.AuthRestart;
  if (IsAuthRestartEnabled && !HasVSMCKext) {
    DEBUG ((DEBUG_WARN, "Misc->Security->启用了AuthRestart，但未在Kernel->Add中加载VirtualSMC!\n"));
    ++ErrorCount;
  }

  BootProtect                    = OC_BLOB_GET (&UserMisc->Security.BootProtect);
  IsRequestBootVarRoutingEnabled = UserUefi->Quirks.RequestBootVarRouting;
  if (AsciiStrCmp (BootProtect, "None") != 0
    && AsciiStrCmp (BootProtect, "Bootstrap") != 0
    && AsciiStrCmp (BootProtect, "BootstrapShort") != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Security->BootProtect 不太对 (只能是 None, Bootstrap, 或 BootstrapShort)!\n"));
    ++ErrorCount;
  } else if (AsciiStrCmp (BootProtect, "Bootstrap") == 0
    || AsciiStrCmp (BootProtect, "BootstrapShort") == 0) {
    if (!IsRequestBootVarRoutingEnabled) {
      DEBUG ((DEBUG_WARN, "Misc->Security->BootProtect 设置为 %a 需要启用 UEFI->Quirks->RequestBootVarRouting!\n", BootProtect));
      ++ErrorCount;
    }
    //
    // NOTE: RequestBootVarRouting requires OpenRuntime.efi, which will be checked in UEFI checker.
    //
  }

  AsciiDmgLoading = OC_BLOB_GET (&UserMisc->Security.DmgLoading);
  if (AsciiStrCmp (AsciiDmgLoading, "Disabled") != 0
    && AsciiStrCmp (AsciiDmgLoading, "Signed") != 0
    && AsciiStrCmp (AsciiDmgLoading, "Any") != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Security->DmgLoading 不太对 (只能是 Disabled, Signed, 或 Any)!\n"));
    ++ErrorCount;
  }

  ExposeSensitiveData = UserMisc->Security.ExposeSensitiveData;
  if ((ExposeSensitiveData & ~OCS_EXPOSE_ALL_BITS) != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Security->ExposeSensitiveData 设置了未知位！\n"));
    ++ErrorCount;
  }

  AsciiVault = OC_BLOB_GET (&UserMisc->Security.Vault);
  if (AsciiStrCmp (AsciiVault, "Optional") != 0
    && AsciiStrCmp (AsciiVault, "Basic") != 0
    && AsciiStrCmp (AsciiVault, "Secure") != 0) {
    DEBUG ((DEBUG_WARN, "Misc->Security->Vault 不太对 (只能是 Optional, Basic, 或 Secure)!\n"));
    ++ErrorCount;
  }

  ScanPolicy        = UserMisc->Security.ScanPolicy;
  AllowedScanPolicy = OC_SCAN_FILE_SYSTEM_LOCK | OC_SCAN_DEVICE_LOCK | OC_SCAN_DEVICE_BITS | OC_SCAN_FILE_SYSTEM_BITS;
  //
  // ScanPolicy can be zero (failsafe value), skipping such.
  //
  if (ScanPolicy != 0) {
    if ((ScanPolicy & ~AllowedScanPolicy) != 0) { 
      DEBUG ((DEBUG_WARN, "Misc->Security->ScanPolicy 设置了未知位！\n"));
      ++ErrorCount;
    }

    if ((ScanPolicy & OC_SCAN_FILE_SYSTEM_BITS) != 0 && (ScanPolicy & OC_SCAN_FILE_SYSTEM_LOCK) == 0) {
      DEBUG ((DEBUG_WARN, "Misc->Security->ScanPolicy需要扫描文件系统, 但是OC_SCAN_FILE_SYSTEM_LOCK (bit 0)未设置!\n"));
      ++ErrorCount;
    }

    if ((ScanPolicy & OC_SCAN_DEVICE_BITS) != 0 && (ScanPolicy & OC_SCAN_DEVICE_LOCK) == 0) {
      DEBUG ((DEBUG_WARN, "Misc->Security->ScanPolicy需要扫描设备, 但是OC_SCAN_DEVICE_LOCK (bit 1)未设置!\n"));
      ++ErrorCount;
    }
  }

  //
  // Validate SecureBootModel.
  //
  SecureBootModel = OC_BLOB_GET (&UserMisc->Security.SecureBootModel);
  if (!ValidateSecureBootModel (SecureBootModel)) {
    DEBUG ((DEBUG_WARN, "Misc->Security->SecureBootModel 不太对!\n"));
    ++ErrorCount;
  }

  return ErrorCount;
}

STATIC
UINT32
CheckMiscTools (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_MISC_CONFIG    *UserMisc;
  CONST CHAR8       *Arguments;
  CONST CHAR8       *Comment;
  CONST CHAR8       *AsciiName;
  CONST CHAR16      *UnicodeName;
  CONST CHAR8       *Path;

  ErrorCount        = 0;
  UserMisc          = &Config->Misc;

  for (Index = 0; Index < UserMisc->Tools.Count; ++Index) {
    Arguments       = OC_BLOB_GET (&UserMisc->Tools.Values[Index]->Arguments);
    Comment         = OC_BLOB_GET (&UserMisc->Tools.Values[Index]->Comment);
    AsciiName       = OC_BLOB_GET (&UserMisc->Tools.Values[Index]->Name);
    Path            = OC_BLOB_GET (&UserMisc->Tools.Values[Index]->Path);

    //
    // Sanitise strings.
    //
    // NOTE: As Arguments takes identical requirements of Comment,
    //       we use Comment sanitiser here.
    //
    if (!AsciiCommentIsLegal (Arguments)) {
      DEBUG ((DEBUG_WARN, "Misc->Tools[%u]->Arguments 包含非法字符！\n", Index));
      ++ErrorCount;
    }
    if (!AsciiCommentIsLegal (Comment)) {
      DEBUG ((DEBUG_WARN, "Misc->Tools[%u]->Comment 包含非法字符！\n", Index));
      ++ErrorCount;
    }
    
    UnicodeName = AsciiStrCopyToUnicode (AsciiName, 0);
    if (UnicodeName != NULL) {
      if (!UnicodeIsFilteredString (UnicodeName, TRUE)) {
        DEBUG ((DEBUG_WARN, "Misc->Tools[%u]->Name 包含非法字符！\n", Index));
        ++ErrorCount;
      }

      FreePool ((VOID *) UnicodeName);
    }

    //
    // FIXME: Properly sanitise Path.
    //
    if (!AsciiCommentIsLegal (Path)) {
      DEBUG ((DEBUG_WARN, "Misc->Tools[%u]->Path 包含非法字符！\n", Index));
      ++ErrorCount;
    }
  }

  //
  // Check duplicated entries in Tools.
  //
  ErrorCount += FindArrayDuplication (
    UserMisc->Tools.Values,
    UserMisc->Tools.Count,
    sizeof (UserMisc->Tools.Values[0]),
    MiscToolsHasDuplication
    );

  return ErrorCount;
}

UINT32
CheckMisc (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32  ErrorCount;
  UINTN   Index;
  STATIC CONFIG_CHECK MiscCheckers[] = {
    &CheckMiscBoot,
    &CheckMiscDebug,
    &CheckMiscEntries,
    &CheckMiscSecurity,
    &CheckMiscTools
  };

  DEBUG ((DEBUG_VERBOSE, "config loaded into %a!\n", __func__));

  ErrorCount = 0;

  for (Index = 0; Index < ARRAY_SIZE (MiscCheckers); ++Index) {
    ErrorCount += MiscCheckers[Index] (Config);
  }

  return ReportError (__func__, ErrorCount);
}
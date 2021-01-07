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

STATIC
UINT32
CheckBooterMmioWhitelist (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_BOOTER_CONFIG  *UserBooter;
  CONST CHAR8       *Comment;
  BOOLEAN           IsMmioWhitelistEnabled;
  BOOLEAN           ShouldEnableDevirtualiseMmio;
  BOOLEAN           IsDevirtualiseMmioEnabled;

  ErrorCount                      = 0;
  UserBooter                      = &Config->Booter;
  IsMmioWhitelistEnabled          = FALSE;
  ShouldEnableDevirtualiseMmio    = FALSE;
  IsDevirtualiseMmioEnabled       = UserBooter->Quirks.DevirtualiseMmio;

  for (Index = 0; Index < UserBooter->MmioWhitelist.Count; ++Index) {
    Comment                = OC_BLOB_GET (&UserBooter->MmioWhitelist.Values[Index]->Comment);
    IsMmioWhitelistEnabled = UserBooter->MmioWhitelist.Values[Index]->Enabled;

    //
    // Sanitise strings.
    //
    if (!AsciiCommentIsLegal (Comment)) {
      DEBUG ((DEBUG_WARN, "Booter->MmioWhitelist[%u]-Comment中包含非法字符!\n", Index));
      ++ErrorCount;
    }

    if (IsMmioWhitelistEnabled) {
      ShouldEnableDevirtualiseMmio = TRUE;
    }
  }

  if (ShouldEnableDevirtualiseMmio && !IsDevirtualiseMmioEnabled) {
    DEBUG ((DEBUG_WARN, "There are enabled entries under Booter->MmioWhitelist, but DevirtualiseMmio is not enabled!\n"));
    ++ErrorCount;
  }

  return ErrorCount;
}

UINT32
CheckBooterPatch (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_BOOTER_CONFIG  *UserBooter;
  CONST CHAR8       *Comment;
  CONST CHAR8       *Arch;
  CONST CHAR8       *Identifier;
  CONST UINT8       *Find;
  UINT32            FindSize;
  CONST UINT8       *Replace;
  UINT32            ReplaceSize;
  CONST UINT8       *Mask;
  UINT32            MaskSize;
  CONST UINT8       *ReplaceMask;
  UINT32            ReplaceMaskSize;

  ErrorCount        = 0;
  UserBooter        = &Config->Booter;

  for (Index = 0; Index < UserBooter->Patch.Count; ++Index) {
    Comment         = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Comment);
    Arch            = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Arch);
    Identifier      = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Identifier);
    Find            = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Find);
    FindSize        = UserBooter->Patch.Values[Index]->Find.Size;
    Replace         = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Replace);
    ReplaceSize     = UserBooter->Patch.Values[Index]->Replace.Size;
    Mask            = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->Mask);
    MaskSize        = UserBooter->Patch.Values[Index]->Mask.Size;
    ReplaceMask     = OC_BLOB_GET (&UserBooter->Patch.Values[Index]->ReplaceMask);
    ReplaceMaskSize = UserBooter->Patch.Values[Index]->ReplaceMask.Size;

    //
    // Sanitise strings.
    //
    if (!AsciiCommentIsLegal (Comment)) {
      DEBUG ((DEBUG_WARN, "Booter->Patch[%u]->Comment中包含非法字符!\n", Index));
      ++ErrorCount;
    }
    if (!AsciiArchIsLegal (Arch, FALSE)) {
      DEBUG ((DEBUG_WARN, "Booter->Patch[%u]->Arch 内容不对 (只能是 Any, i386, 和 x86_64)!\n", Index));
      ++ErrorCount;
    }
    if (!AsciiIdentifierIsLegal (Identifier, FALSE)) {
      DEBUG ((DEBUG_WARN, "Booter->Patch[%u]->标识符包含非法字符!\n", Index));
      ++ErrorCount;
    }

    //
    // Checks for size.
    //
    ErrorCount += ValidatePatch (
      "Booter->Patch",
      Index,
      FALSE,
      Find,
      FindSize,
      Replace,
      ReplaceSize,
      Mask,
      MaskSize,
      ReplaceMask,
      ReplaceMaskSize
      );
  }

  return ErrorCount;
}

STATIC
UINT32
CheckBooterQuirks (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32            ErrorCount;
  UINT32            Index;
  OC_BOOTER_CONFIG  *UserBooter;
  OC_UEFI_CONFIG    *UserUefi;
  CONST CHAR8       *Driver;
  UINT8             MaxSlide;
  BOOLEAN           IsAllowRelocationBlockEnabled;
  BOOLEAN           IsProvideCustomSlideEnabled;
  BOOLEAN           IsEnableSafeModeSlideEnabled;
  BOOLEAN           IsDisableVariableWriteEnabled;
  BOOLEAN           IsEnableWriteUnprotectorEnabled;
  BOOLEAN           HasOpenRuntimeEfiDriver;

  ErrorCount                      = 0;
  UserBooter                      = &Config->Booter;
  UserUefi                        = &Config->Uefi;
  IsAllowRelocationBlockEnabled   = UserBooter->Quirks.AllowRelocationBlock;
  IsProvideCustomSlideEnabled     = UserBooter->Quirks.ProvideCustomSlide;
  IsEnableSafeModeSlideEnabled    = UserBooter->Quirks.EnableSafeModeSlide;
  IsDisableVariableWriteEnabled   = UserBooter->Quirks.DisableVariableWrite;
  IsEnableWriteUnprotectorEnabled = UserBooter->Quirks.EnableWriteUnprotector;
  HasOpenRuntimeEfiDriver         = FALSE;
  MaxSlide                        = UserBooter->Quirks.ProvideMaxSlide;

  for (Index = 0; Index < UserUefi->Drivers.Count; ++Index) {
    Driver = OC_BLOB_GET (UserUefi->Drivers.Values[Index]);

    //
    // Skip sanitising UEFI->Drivers as it will be performed when checking UEFI section.
    //

    if (AsciiStrCmp (Driver, "OpenRuntime.efi") == 0) {
      HasOpenRuntimeEfiDriver = TRUE;
    }
  }

  if (!HasOpenRuntimeEfiDriver) {
    if (IsProvideCustomSlideEnabled) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->ProvideCustomSlide已启用, 但是OpenRuntime.efi没有在UEFI->Drivers中加载!\n"));
      ++ErrorCount;
    }
    if (IsDisableVariableWriteEnabled) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->已启用DisableVariableWrite，但未在UEFI->Drivers中加载OpenRuntime.efi!\n"));
      ++ErrorCount;
    }
    if (IsEnableWriteUnprotectorEnabled) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->EnableWriteUnprotector已启用，但未在UEFI->Drivers中加载OpenRuntime.efi!\n"));
      ++ErrorCount;
    }
  }

  if (!IsProvideCustomSlideEnabled) {
    if (IsAllowRelocationBlockEnabled) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->启用AllowRelocationBlock，但未完全启用ProvideCustomSlide!\n"));
      ++ErrorCount;
    }
    if (IsEnableSafeModeSlideEnabled) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->已启用EnableSafeModeSlide，但未完全启用ProvideCustomSlide!\n"));
      ++ErrorCount;
    }
    if (MaxSlide > 0) {
      DEBUG ((DEBUG_WARN, "Booter->Quirks->ProvideMaxSlide设置为%u，但未完全启用ProvideCustomSlide!\n", MaxSlide));
      ++ErrorCount;
    }
  }

  return ErrorCount;
}

UINT32
CheckBooter (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32  ErrorCount;
  UINTN   Index;
  STATIC CONFIG_CHECK BooterCheckers[] = {
    &CheckBooterMmioWhitelist,
    &CheckBooterPatch,
    &CheckBooterQuirks
  };
  
  DEBUG ((DEBUG_VERBOSE, "config loaded into %a!\n", __func__));

  ErrorCount  = 0;

  for (Index = 0; Index < ARRAY_SIZE (BooterCheckers); ++Index) {
    ErrorCount += BooterCheckers[Index] (Config);
  }

  return ReportError (__func__, ErrorCount);
}
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

#include <OpenCore.h>

#include <UserFile.h>

UINT32
CheckConfig (
  IN  OC_GLOBAL_CONFIG  *Config
  )
{
  UINT32  ErrorCount;
  UINTN   Index;
  STATIC CONFIG_CHECK ConfigCheckers[] = {
    &CheckACPI,
    &CheckBooter,
    &CheckDeviceProperties,
    &CheckKernel,
    &CheckMisc,
    &CheckNVRAM,
    &CheckPlatformInfo,
    &CheckUEFI
  };

  ErrorCount = 0;

  //
  // Pass config structure to all checkers.
  //
  for (Index = 0; Index < ARRAY_SIZE (ConfigCheckers); ++Index) {
    ErrorCount += ConfigCheckers[Index] (Config);
  }

  return ErrorCount;
}

int ENTRY_POINT(int argc, const char *argv[]) {
  UINT8              *ConfigFileBuffer;
  UINT32             ConfigFileSize;
  CONST CHAR8        *ConfigFileName;
  INT64              ExecTimeStart;
  OC_GLOBAL_CONFIG   Config;
  EFI_STATUS         Status;
  UINT32             ErrorCount;

  //
  // Enable PCD debug logging.
  //
  PcdGet8  (PcdDebugPropertyMask)         |= DEBUG_PROPERTY_DEBUG_CODE_ENABLED;
  PcdGet32 (PcdFixedDebugPrintErrorLevel) |= DEBUG_INFO;
  PcdGet32 (PcdDebugPrintErrorLevel)      |= DEBUG_INFO;

  //
  // Print usage.
  //
  if (argc != 2 || (argc > 1 && AsciiStrCmp (argv[1], "--version") == 0)) {
    DEBUG ((DEBUG_ERROR, "\n注意：此版本的ocvalidate仅适用于OpenCore版本 %a!\n\n", OPEN_CORE_VERSION));
    DEBUG ((DEBUG_ERROR, "用法: %a <指定路径/config.plist>\n\n", argv[0]));
    return -1;
  }

  //
  // Read config file (Only one single config is supported).
  //
  ConfigFileName   = argv[1];
  ConfigFileBuffer = UserReadFile (ConfigFileName, &ConfigFileSize);
  if (ConfigFileBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "读取 %a 失败\n", ConfigFileName));
    return -1;
  }

  //
  // Record the current time when action starts.
  //
  ExecTimeStart = GetCurrentTimestamp ();

  //
  // Initialise config structure to be checked, and exit on error.
  //
  Status = OcConfigurationInit (&Config, ConfigFileBuffer, ConfigFileSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "无效的配置\n"));
    return -1;
  }

  ErrorCount = CheckConfig (&Config);
  if (ErrorCount == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "检查 %a用了 %llu 毫秒完成\n",
      ConfigFileName,
      GetCurrentTimestamp () - ExecTimeStart
      ));
  } else {
    DEBUG ((
      DEBUG_ERROR,
      "检查 %a用了 %llu 毫秒完成, 但它有 %u %a 要修复\n",
      ConfigFileName,
      GetCurrentTimestamp () - ExecTimeStart,
      ErrorCount,
      ErrorCount > 1 ? "错误" : "错误"
      ));
  }

  OcConfigurationFree (&Config);
  free (ConfigFileBuffer);

  return 0;
}

INT32 LLVMFuzzerTestOneInput(CONST UINT8 *Data, UINTN Size) {
  VOID *NewData = AllocatePool (Size);
  if (NewData) {
    CopyMem (NewData, Data, Size);
    OC_GLOBAL_CONFIG   Config;
    OcConfigurationInit (&Config, NewData, Size);
    OcConfigurationFree (&Config);
    FreePool (NewData);
  }
  return 0;
}
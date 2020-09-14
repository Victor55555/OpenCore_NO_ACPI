
-----

OpenCore bootloader修改版.

## 修改内容

- 添加来自NDK的可选ACPI控制选项
- 编译代码资源全部来自gitee,方便国内用户编译
- 整合OC引导界面自定义资源(包括自定义图标和中文语音包)
- 编译代码小的改动
- 跟随OC进行迭代

<<<<<<< HEAD
#### 致谢
=======
## Libraries

This repository also contains additional UEFI support common libraries shared by other projects in [Acidanthera](https://github.com/acidanthera). The primary purpose of the library set is to provide supplemental functionality for Apple-specific UEFI drivers. Key features:

- Apple disk image loading support
- Apple keyboard input aggregation
- Apple PE image signature verification
- Apple UEFI secure boot supplemental code
- Audio management with screen reading support
- Basic ACPI and SMBIOS manipulation
- CPU information gathering with timer support
- Cryptographic primitives (SHA-256, RSA, etc.)
- Decompression primitives (zlib, lzss, lzvn, etc.)
- Helper code for ACPI reads and modifications
- Higher level abstractions for files, strings, UEFI variables
- Overflow checking arithmetics
- PE image loading with no UEFI Secure Boot conflict
- Plist configuration format parsing
- PNG image manipulation
- Text output and graphics output implementations
- XNU kernel driver injection and patch engine

Early history of the codebase could be found in [AppleSupportPkg](https://github.com/acidanthera/AppleSupportPkg) and PicoLib library set by The HermitCrabs Lab.

#### OcGuardLib

This library implements basic safety features recommended for the use within the project. It implements fast
safe integral arithmetics mapping on compiler builtins, type alignment checking, and UBSan runtime,
based on [NetBSD implementation](https://blog.netbsd.org/tnf/entry/introduction_to_µubsan_a_clean).

The use of UBSan runtime requires the use of Clang compiler and `-fsanitize=undefined` argument. Refer to
[Clang documentation](https://releases.llvm.org/7.0.0/tools/clang/docs/UndefinedBehaviorSanitizer.html) for more
details.

#### Credits

- The HermitCrabs Lab
- All projects providing third-party code (refer to file headers)
- [AppleLife](https://applelife.ru) team and user-contributed resources
- Chameleon and Clover teams for hints and legacy
- [al3xtjames](https://github.com/al3xtjames)
- [Andrey1970AppleLife](https://github.com/Andrey1970AppleLife)
- [Download-Fritz](https://github.com/Download-Fritz)
- [Goldfish64](https://github.com/Goldfish64)
- [nms42](https://github.com/nms42)
- [PMHeart](https://github.com/PMHeart)
- [savvamitrofanov](https://github.com/savvamitrofanov)
- [usr-sse2](https://github.com/usr-sse2)
>>>>>>> d61d3982ae9710e2a427b1c9874e413e7d199415
- [vit9696](https://github.com/vit9696)
- [ndk](https://github.com/n-d-k)
# Building ENet on PS5

To build ENet under the PS5 SDK, several changes must be made from the template
we were given. These are documented here:

Note that while this compiles and links, it is not yet tested as our game doesn't
yet have network support.

- Update ENet<br>
  the included version of ENet does not include Unix support, the easiest way to
  resolve this is to manually replace [CSC8503CoreClasses/enet](../CSC8503CoreClasses/enet/) with the latest
  headers/sources fetched from [https://github.com/lsalzman/enet](https://github.com/lsalzman/enet)
- Include `enet/unix.h/c` on Unix platforms<br>
  In [CSC8503CoreClasses/CMakeLists.txt](../CSC8503CoreClasses/CMakeLists.txt),
  condition `win32.h/c` to only be built `if(WIN32)` and use `unix.h/c` otherwise
- Patch `enet/unix.c`<br>
  Several changes must be applied to `enet/unix.c`, either use the version in this
  repository or apply the included [enet.patch](./enet.patch) in git bash [addendum 1](#patch-apply)
- Link against POSIX socket libraries<br>
  In [PS5Starter/CMakeLists.txt](../PS5Starter/CMakeLists.txt), link
  the output against the libraries detailed in [addendum 2](#socket-libraries).
  This list was determined from searching the SDKs examples, plus trial-and-error,
  and likely includes several unneeeded libraries.

## Patch Apply
```sh
patch CSC8503CoreClasses/enet/unix.c enet.patch
```

## Socket Libraries
```cmake
PRIVATE SceNpUtility_stub_weak
PRIVATE SceCommonDialog_stub_weak
PRIVATE SceNetCtlApDialog_stub_weak
PRIVATE SceNetCtlAp_stub_weak
PRIVATE SceNetCtl_stub_weak
PRIVATE SceNet_stub_weak
PRIVATE ScePosix_stub_weak
```

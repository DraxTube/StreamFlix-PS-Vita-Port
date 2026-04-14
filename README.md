# StreamFlix Vita

PS Vita port of StreamFlix - a simple streaming content browser.

## Build

### Prerequisites
- [VitaSDK](https://vitasdk.org/) installed
- Or just push to GitHub and let GitHub Actions build the VPK automatically

### Local Build
```bash
cd vita
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
make -j$(nproc)
```

### GitHub Actions (Automatic)
Push to `main` branch or create a tag `v*` to trigger automatic VPK build.
The VPK will be available as a GitHub Actions artifact or as a release asset.

## Install on PS Vita
1. Your PS Vita must have **HENkaku/Enso** installed
2. Install **iTLS-Enso** for HTTPS support
3. Transfer the `.vpk` to your Vita via USB/FTP
4. Install via **VitaShell**

## Features
- Browse trending movies & TV shows
- Search content
- Watch movies and episodes (direct MP4 links)
- D-pad navigation optimized for Vita
- Dark theme with red accents
- Watch history

## Controls
| Button | Action |
|--------|--------|
| D-pad | Navigate |
| X | Select/Confirm |
| O | Back |
| START | Open Search |
| SELECT | Exit (from main menu) |

## Architecture
```
vita/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── main.c/h            # App entry, state machine
│   ├── net/                 # HTTP, JSON, crypto
│   ├── providers/           # Content providers (SuperStream)
│   ├── ui/                  # All UI screens
│   └── db/                  # Watch history
└── sce_sys/                 # PS Vita metadata
```

## Provider
Uses the SuperStream API (same as the Android app) ported to C with:
- OpenSSL for DESede/CBC encryption + MD5
- libcurl for HTTPS requests
- jansson for JSON parsing

## License
Apache-2.0 - Same as the original StreamFlix project.

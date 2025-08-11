# Android Server Development Setup

## IntelliSense Configuration

### Prerequisites

1. **Android NDK**: Download and install the Android NDK
   ```bash
   # Set environment variable
   export ANDROID_NDK_HOME=/path/to/android-ndk
   ```

2. **VS Code Extensions**:
   - C/C++ Extension Pack
   - CMake Tools (optional, for main project)

### Fixing IntelliSense Errors

The project includes VS Code configuration files that should resolve most IntelliSense issues:

- `.vscode/c_cpp_properties.json` - Configures include paths for Android NDK
- `.vscode/settings.json` - Workspace-specific C++ settings

### Common Issues and Solutions

#### 1. "cannot open source file android/log.h"
**Solution**: Set the `ANDROID_NDK_HOME` environment variable:
```bash
export ANDROID_NDK_HOME=/path/to/android-ndk
```

#### 2. Include Path Errors
**Solution**: 
- Ensure Android NDK is installed and `ANDROID_NDK_HOME` is set
- Reload VS Code window (`Ctrl+Shift+P` -> "Developer: Reload Window")
- Select "Android NDK" configuration in the status bar

#### 3. Missing Interface Files
All interface files are now included:
- `audio_capture/audio_capture_interface.h` ✓
- `protocol/protocol_handler.h` ✓
- `screen_capture/capture_interface.h` ✓
- `video_encoder/encoder_interface.h` ✓

### Building the Project

```bash
# From android_server directory
mkdir build
cd build

# Configure for Android
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
         -DANDROID_ABI=arm64-v8a \
         -DANDROID_PLATFORM=android-29

# Build
make -j$(nproc)
```

### Project Structure

```
android_server/src/
├── main.cpp                    # Entry point
├── server.h/.cpp              # Main server orchestration
├── common/                    # Common types and utilities
│   ├── types.h               # Core type definitions
│   └── logger.h              # Logging utilities
├── protocol/                  # Communication protocol
│   ├── packet_types.h        # Packet structure definitions
│   └── protocol_handler.h    # Protocol handling interface
├── video_encoder/            # Video encoding
│   ├── encoder_interface.h   # Encoder interface
│   └── mediacodec_h264_encoder.h/.cpp
├── screen_capture/           # Screen capture
│   ├── capture_interface.h   # Capture interface
│   └── media_projection_capture.h/.cpp
├── audio_capture/            # Audio capture (interface only)
│   └── audio_capture_interface.h
└── network/                  # Network communication
    └── tcp_server.h/.cpp
```

### Next Steps

1. **Desktop H.264 Decoder**: Implement FFmpeg-based decoder for Qt client
2. **Protocol Handler Implementation**: Complete the protocol handler implementation
3. **Audio Capture**: Implement Android audio capture using AAC
4. **Deployment System**: Create ADB deployment scripts 
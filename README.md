# OpenX4 E-Paper Community SDK

A **community-maintained SDK** for building firmware and tools for the **Xteink X4** device, providing a shared set of libraries, utilities, and development workflows that make working with the X4 simple and consistent.

## ✨ **What is this?**

The **OpenX4 E-Paper Community SDK** provides:

* **Common PlatformIO-friendly components** for the Xteink X4
* **Reusable libraries** for display control, graphics, hardware helpers, utilities, etc.
* **Tools** to support flashing, packaging, testing, and device workflows
* A central place for **community contributions**, improvements, and shared knowledge

The SDK is intentionally modular - bring it into your project and use only what you need.

## 📁 Repository Structure

```
community-sdk/
├── libs/           # Reusable components for X4 firmware
│   ├── display/       # E-paper helpers & drivers
│   ├── graphics/      # Drawing, fonts, UI utilities
│   ├── hardware/      # GPIO, power, sensors, timings, etc.
│   └── ...            # Add new modules here!
│
└── tools/          # Dev tools for X4
    ├── flash/         # Flash helpers, scripts, workflows
    ├── assets/        # Conversion tools for images/fonts
    └── ...            # Community-contributed utilities
```

Each lib aims to be **self-contained**, **documented**, and **PlatformIO-friendly**.
Libs should be categorized under `libs/` based on functionality, and then contained within a directory under that root.

## 📦 Adding to Your PlatformIO Project

> The SDK and all its libraries are published as ZIP archives on every [GitHub Release](https://github.com/open-x4-epaper/community-sdk/releases).
> PlatformIO downloads and installs them automatically.

Add the SDK to your `platformio.ini`:

```ini
lib_deps =
  https://github.com/open-x4-epaper/community-sdk/releases/latest/download/open-x4-sdk.zip
```

PlatformIO will automatically install all individual libraries
(`EInkDisplay`, `InputManager`, `BatteryMonitor`, `SDCardManager`, etc.) from the latest release.

Then include the libraries in your project as usual:

```cpp
#include <BatteryMonitor.h>
#include <EpdScreenController.h>
```

### Pinning to a specific version

To use a specific release instead of always the latest, reference the versioned `open-x4-sdk.zip` directly:

```ini
lib_deps =
  https://github.com/open-x4-epaper/community-sdk/releases/download/v1.0.0/open-x4-sdk.zip
```

## 🔄 How Releases Work

On every tag push (e.g. `v1.2.0`), a GitHub Action automatically:

1. Scans `libs/display/` and `libs/hardware/` for directories containing a `library.json`
2. Packages each library into its own ZIP (e.g. `EInkDisplay.zip`)
3. Generates a versioned root `library.json` with exact download URLs for that release
4. Packages the root `library.json` as `open-x4-sdk.zip`
5. Publishes all ZIPs as assets on the GitHub Release

## 🤝 Contributing

This is a **community-driven project** - contributions are not only welcome but encouraged!

Ways you can help:

* Improve or extend existing libraries
* Add new modules to `libs/`
* Build utilities for the `tools/` directory
* Report issues, propose features, or help refine the API
* Improve documentation

### Contribution guidelines (short version)

1. Keep modules self-contained
2. Prefer zero-dependency solutions where practical
3. Document your additions (including a `library.json`)
4. Use clear naming and consistent structure
5. Be friendly and constructive in PR conversations

A full contributing guide will be added as the project grows.

## 📝 License

This SDK is released under an open-source MIT license. To keep things simple, all contributions and code must also
fall under this license.

## 💬 Community

Feel free to open GitHub issues for support, improvements, or discussion around the Xteink X4 ecosystem.
Join the discord here: https://discord.gg/2cdKUbWRE8

# 🎨 ImageBuilder

> **Print Images Directly into the Minecraft World.**

[简体中文](README_zh-CN.md)

ImageBuilder is an Endstone plugin designed to convert local images (PNG, JPG, etc.) into block-based pixel art within Minecraft. It allows you to generate real structures made of blocks directly in-game without external map tools.

---

## ✨ Key Features

* **🚀 Silent Execution:** Commands run silently in the background without flooding the chat.
* **📦 Chunked Building:** Automatically splits large images into smaller chunks to prevent server lag.
* **🔄 Auto-Retry:** Intelligent retry mechanism for block placement failures (e.g., unloaded chunks).
* **🖼️ Preview Generation:** Generates a pixelated preview image during conversion to ensure visual accuracy.
* **⚡ High Performance:** Built with C++ and the `stb` library, enabling image conversion in seconds and rapid building.

---

## 📁 Folder Structure

The plugin will automatically create these directories on its first run:

```text
./plugins/img_print/
├── images/      <-- 📥 Source images (e.g., .png, .jpg)
├── output/      <-- 📤 Auto-generated CSV data and previews
└── language/    <-- 🌐 Language localization files

```

---

## 🧰 Commands

| Command | Description |
| --- | --- |
| `/img-p ls` | 📋 List all source images and converted output files. |
| `/img-p convert <index>` | ⚙️ Convert a source image (by index) into a CSV data file. |
| `/img-p print <index>` | 🏗️ Build the selected output file at your current location. |

**💡 Quick Start Example:**

1. `/img-p convert 1` → Converts the 1st image in `images/` to a data file.
2. `/img-p print 1` → Starts building the 1st file from `output/` at your feet.

---

## ⚙️ Technical Notes

* **📐 Resizing:** Images are automatically scaled to **128×128** pixels by default.
* **📊 Progress Tracking:** Building is processed in chunks with progress displayed in the Action Bar (e.g., `3 / 10`).
* **🛡️ Permissions:** Requires **OP** or equivalent administrative permissions.

---

## 🛠️ Installation

1. Place `img_print.dll` (Windows) or `.so` (Linux) into the server's `plugins` folder.
2. Restart the server to initialize the directory structure.
3. Drop your images into `plugins/img_print/images/` and you're ready to go!

## 🙏 Credits

* Image processing powered by the [stb](https://github.com/nothings/stb) library.
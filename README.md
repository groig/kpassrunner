# KPassRunner

**Description:**
A KRunner plugin for KDE Plasma 6 that lets you quickly search your `pass` (password-store) entries and copy passwords to the clipboard.

---

## Disclaimer

This shit is vibe-coded.

## Features

* Search `pass` entries from KRunner.
* Press Enter to copy the password to clipboard (`pass -c`).
* Supports Plasma 6 / KF6 / Qt6 on Arch Linux.

---

## Installation

1. **Install dependencies**
   Make sure the necessary KF6 and Qt6 development libraries are installed:

   ```bash
   sudo pacman -S extra-cmake-modules qt6-base
   ```

2. **Clone / Prepare Source**

   ```bash
   git clone <your-repo-url> passrunner
   cd kpassrunner
   mkdir build
   cd build
   ```

3. **Build**

   ```bash
   cmake ..
   make
   ```

4. **Install**

   ```bash
   sudo make install
   ```

5. **Restart KRunner**

   ```bash
   kquitapp6 krunner
   kstart krunner
   ```

---

## Usage

* Press `Alt + Space` (or your KRunner shortcut).
* Type `<query>` to search for entries in `pass`.
* Select an entry and press Enter to copy the password to your clipboard.

---

## Notes

* For security, the plugin **does not display passwords**, it only copies them to the clipboard.
* You may need to refresh KRunner after adding new entries to `pass`.

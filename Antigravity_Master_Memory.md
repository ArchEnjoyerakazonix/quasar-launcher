# Antigravity Master Memory — Quasar Launcher

## 📌 Project Overview
- **Project Name:** Quasar Application Launcher
- **Repository:** `https://github.com/ArchEnjoyerakazonix/quasar.git`
- **Location:** `/home/archuser/Projects/quasar`
- **Build System:** CMake (C++20 standard)
- **Tech Stack:** C++20, Qt6 (Core, Quick, Gui, DBus, Concurrent, Svg, Test), LayerShellQt, QML
- **Latest Commit:** `bb84a37`

---

## 📈 Technical Audit Trajectory (Score History)
- **v1 Audit Score:** **4.5/10** (Initial external audit - 6 critical P0 issues)
- **v2 Audit Score:** **5.8/10** (+1.3 - Fixed `exec`/`keywords` roles, QML context property `frecencyRanker`, Damerau-Levenshtein edit distance, QML `query` property binding)
- **v3 Audit Score:** **6.7/10** (+0.9 - Fixed DL 3-row transposition `prev2` array, delegate `activate()` activation, logarithmic frecency score clamping `0..900`, EN->RU layout conversion)
- **v4 Audit Score:** **8.0/10** (+1.3 - Fixed `useLayerShell()` placement BEFORE `QGuiApplication`, CPU optimization `m_corpusHasCyrillic` + `isPureLatin`, `formatHighlightedName` EN->RU support, async `IconProvider` with `QPixmapCache` + negative cache + Flatpak paths, cursor screen targeting `screenAt(QCursor::pos())`, `ё`/`Ё` layout mapping, `AppGrid` footer, QtTest `test_fuzzymatcher` test suite)

---

## 🛠 Complete Summary of Accomplishments & Code Architecture

### 1. Core Search Engine (`src/fuzzymatcher.cpp`, `src/fuzzymatcher.h`)
- **Bounded Damerau-Levenshtein Edit Distance:** 3-row matrix (`prev2`, `prev`, `curr`) supporting adjacent transpositions (`ca` ↔ `ac`, `friefox` → `firefox`, `stema` → `steam`).
- **Acronym Recognition:** High-priority matching for initials (`vsc` → `Visual Studio Code`, `tb` → `Telegram Desktop`).
- **Exec Binary Name Matching:** Direct matching for binary names (`pavucontrol` → `PulseAudio Volume Control`).
- **Bi-directional Layout & Mnemonic Transliteration:** QWERTY ↔ ЙЦУКЕН + Mnemonic (`ghbdtn` → `Привет`, `чщ` → `vsc`). Includes `~`/`` ` `` ↔ `Ё`/`ё`.
- **CPU Optimization:** `m_corpusHasCyrillic` flag computed once on model reset. `convertEnToRu` only runs if `m_corpusHasCyrillic && isPureLatin(query)`.
- **Logarithmic Frecency Clamping:** Clamped between `0` and `900` points (`std::clamp(qRound(180.0 * std::log1p(rawFrecency)), 0, 900)`), keeping history bonuses within relevance tier bounds.
- **Cache Management:** `m_scoreCache` automatically cleared on `modelReset` and `dataChanged` signals.

### 2. Qt6 Engine & System Integration (`src/main.cpp`)
- **Wayland LayerShell Placement:** `LayerShellQt::Shell::useLayerShell()` called BEFORE `QGuiApplication` constructor for proper `overlay` layer registration.
- **Async Icon Provider (`IconProvider`):** 20 MB `QPixmapCache` + negative caching (`QSet<QString> negativeCache`) + search paths including Flatpak (`~/.local/share/flatpak/exports/share/icons`, `/var/lib/flatpak/exports/share/icons`).
- **Multi-Monitor Targeting:** `QGuiApplication::screenAt(QCursor::pos())` sets window to current mouse cursor display upon show/toggle.
- **D-Bus Error Checking:** Checked return values of `registerService` and `registerObject`.

### 3. QML UI & Delegates (`qml/`)
- **`AppList.qml` & `AppListDelegate.qml`:** Clean role-independent `activate()` function on delegate. Added `reuseItems: true` and `cacheBuffer: 300`.
- **`AppGrid.qml`:** Symmetrical footer (`Run command` / `Search web`) with full keyboard navigation (Up / Down / Enter). Added `reuseItems: true` and `cacheBuffer: 300`.
- **Logo Customization:** `assets/icons/quasar-icon.svg` updated to 100% transparent background (dark rect removed, shape scaled +15%). Icon regenerated across 16x16...512x512 resolution PNGs and installed to `~/.local/share/icons/hicolor`, `~/.icons/`, and `~/.local/share/pixmaps/`. System icon cache updated.

### 4. Automated Testing (`tests/test_fuzzymatcher.cpp`, `CMakeLists.txt`)
- QtTest suite `test_fuzzymatcher` registered with `CTest` (`add_test`). Runs via `ctest --test-dir build --output-on-failure`. All tests 100% passing.

---

## 📌 Instructions for the Next Account / Session
- Read this file and `repomix.txt` to instantly catch up on the entire codebase state.
- All code is compiled, tested, and pushed to `main` branch on GitHub (`https://github.com/ArchEnjoyerakazonix/quasar.git`).
- To test the project locally at any time:
  ```bash
  cmake -B build -S . && cmake --build build -j$(nproc) && ctest --test-dir build --output-on-failure
  ```

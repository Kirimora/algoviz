<div align="center">

# 🧮 AlgoViz

### An interactive algorithm visualizer built with Qt6 & C++

*Watch sorting, pathfinding, hashing, and dynamic programming algorithms come to life — step by step, frame by frame.*

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6-41CD52?style=flat-square&logo=qt&logoColor=white)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?style=flat-square&logo=cmake&logoColor=white)](https://cmake.org/)
[![Arch Linux](https://img.shields.io/badge/Arch%20%2F%20CachyOS-1793D1?style=flat-square&logo=archlinux&logoColor=white)](https://archlinux.org/)
[![Wayland](https://img.shields.io/badge/Wayland-Niri-FFB300?style=flat-square)](https://github.com/YaLTeR/niri)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](#-license)

</div>

---

## ✨ Overview

**AlgoViz** turns abstract algorithms into something you can *see*. Every algorithm
precomputes its execution as a sequence of frames, so a shared transport bar lets you
**play, pause, step forward, step back, and scrub** to any point — forward *or* backward —
just like a video timeline.

Four categories, each in its own tab, each with tunable settings:

| Tab | Algorithms | Highlights |
|-----|-----------|------------|
| 🔢 **Sorting** | Bubble · Insertion · Selection · Quick · Merge · Heap · Shell | Adjustable array size & value range |
| 🗺️ **Pathfinding** | BFS · DFS · Dijkstra · A* | Editable walls · diagonal moves · maze generator |
| 🪣 **Hash Table** | Linear · Quadratic · Double hashing | Collision & probe-sequence visualization |
| 📊 **Dynamic Programming** | 0/1 Knapsack · LCS | Cell-by-cell fill · recurrence · solution backtracking |

---

## 🎬 Features

### 🔢 Sorting
Seven classic algorithms animated bar-by-bar. Tune the **array size** (5–200) and
**maximum value** (10–500), then shuffle and play.

> 🟦 idle · 🟥🟨 active comparison · 🟩 finalized

### 🗺️ Pathfinding
Draw walls by clicking and dragging on the grid, generate a random maze, or toggle
**diagonal movement** (8-neighbour, with a proper √2 ≈ 1.4 diagonal cost in Dijkstra & A*).

> 🟩 start · 🟥 goal · 🟦 visited · 🟨 frontier · 🟪 shortest path

### 🪣 Hash Table
Watch open-addressing insertion resolve collisions in real time. Choose your **capacity**
and a list of **keys**, pick a probing strategy, and follow each probe step.

> 🟦 home slot · 🟨 probing · 🟩 placed

### 📊 Dynamic Programming
Toggle **Animate fill** to watch the DP table populate cell-by-cell, or switch it off
to jump straight to the completed table. The **recurrence relation** is displayed above
the grid, and the **backtracked solution** is highlighted once the table is full.

> Knapsack → optimal item selection · LCS → longest common subsequence string

---

## 🚀 Quick Start

### Dependencies (Arch / CachyOS)

```bash
sudo pacman -S --needed qt6-base cmake gcc
```

### Build & Run

```bash
git clone <your-repo-url> algoviz
cd algoviz
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/algoviz
```

### Install system-wide

```bash
makepkg -si                              # using the bundled PKGBUILD
# — or —
sudo cmake --install build --prefix /usr
```

---

## 🪟 Niri / NoctaliaShell (Wayland)

AlgoViz runs natively on Wayland. Force a backend if you ever need to:

```bash
QT_QPA_PLATFORM=wayland ./build/algoviz   # native Wayland
QT_QPA_PLATFORM=xcb     ./build/algoviz   # XWayland fallback
```

The window **app id** is `algoviz` — handy for Niri window rules:

```kdl
window-rule {
    match app-id="algoviz"
    // your rules here, e.g. default-column-width, open-floating, etc.
}
```

---

## 🧩 Project Structure

```
algoviz/
├── CMakeLists.txt          # Qt6 + CMake build
├── PKGBUILD                # Arch/CachyOS packaging
├── include/
│   ├── MainWindow.h
│   ├── PlaybackBar.h       # shared transport widget
│   ├── SortView.h
│   ├── PathView.h
│   ├── HashView.h
│   └── DPView.h
└── src/
    ├── main.cpp
    ├── MainWindow.cpp       # tab container
    ├── PlaybackBar.cpp      # play/pause/step/scrub + speed
    ├── SortView.cpp
    ├── PathView.cpp
    ├── HashView.cpp
    └── DPView.cpp
```

---

## 🛠️ Architecture

The heart of AlgoViz is **frame precomputation**. Rather than animating an algorithm as
it runs, each view executes the algorithm fully and records every meaningful state as a
*frame*. The reusable `PlaybackBar` then simply indexes into that frame list:

```
algorithm  ──►  vector<Frame>  ──►  PlaybackBar.setTotalFrames(n)
                                          │
                          frameChanged(idx)  ──►  view renders frame idx
```

Because all state is precomputed, **scrubbing backward is free** — it's just an index
lookup (sorting, hashing) or a fast replay-from-snapshot (pathfinding, DP).

### Extending AlgoViz

**Add a sorting algorithm** — write a `genX()` that pushes `SortStep` snapshots, then
register it in the combo box and `buildSteps()`.

**Add a pathfinding algorithm** — write a `buildX()` that pushes `Frame`s and calls
`reconstruct()` for the final path.

---

## 🗺️ Roadmap

- [ ] Graph algorithms tab (MST, topological sort) with a dedicated node-and-edge canvas
- [ ] Side-by-side algorithm comparison
- [ ] Export animations as GIF / video
- [ ] Custom color themes

---

## 🤝 Contributing

Contributions are welcome! Feel free to open an issue or submit a pull request for new
algorithms, settings, or visual improvements.

---

## 📄 License

Released under the [MIT License](LICENSE).

<div align="center">

Built with ❤️ for **Arch Linux / CachyOS** · **NoctaliaShell** · **Niri**

</div>
# algoviz

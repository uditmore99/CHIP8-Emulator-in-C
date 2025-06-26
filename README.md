# CHIP-8 Emulator in C (with SDL2)

[![invaders-GIF.gif](https://i.postimg.cc/vZ0JNxXc/invaders-GIF.gif)](https://postimg.cc/rDtZdmBk)

This is a basic CHIP-8 emulator written in C using the SDL2 library for graphics and input. It supports running CHIP-8 games via a simple drag-and-drop interface.

## ⚙️ Installation (Windows)

You have **two options** to run the CHIP-8 Emulator on Windows:

---

### 🅰️ Option 1: Download Pre-Built Executable

The easiest way to run the emulator and almost no setup needed.

1. Go to the [Releases](https://github.com/uditmore99/CHIP8-Emulator-in-C/releases/) section of this repository.
2. Download the latest `.exe` and `SDL2.dll`.
3. Place **both files in the same folder**.
4. Drag and drop your preffered CHIP-8 ROMS into the `CHIP8-Emulator.exe` to launch the emulator.

> 💡 Make sure `SDL2.dll` is in the **same folder** as the `.exe`, or it won’t run.

---

### 🅱️ Option 2: Build From Source (Recommended for Devs)

Follow these steps to set up your environment and compile the project manually:

### 1️⃣ Install MinGW (GCC Compiler)

1. Follow this [guide](https://code.visualstudio.com/docs/cpp/config-mingw) to install GCC with MinGW on Windows.
2. Make sure to add `C:\MinGW\bin` to your **System PATH** environment variable.

To verify:

```bash
gcc --version
```

### 2️⃣ Install Make

1. Download and Install **Make** from [here](https://gnuwin32.sourceforge.net/packages/make.htm).

To verify:

```bash
make --version
```

### 3️⃣ Install SDL2 (Optional)

1. Go to the SDL2 [Github releases](https://github.com/libsdl-org/SDL/releases) page and download any 2.x.x version of SDL (don't use SDL3).
2. Download _SDL2-devel-2.x.x-mingw.zip_ (MinGW version).
3. Extract the contents.
4. Copy:

   - The **include/** folder to your project under `src/include`
   - The **lib/** folder to your project under `src/lib`
   - The **SDL2.dll** to your project root (next to `main.exe`)

---

### ✅ Build the Project

Now run:

```bash
make
```

This will compile `main.c` and produce `main.exe`.

---

## ▶️ How to Run

### 💻 Option 1: Run from Terminal

You can also run the emulator from the terminal by passing the path to a `.ch8` file:

```bash
./main.exe games/pong.ch8
```

### 🖱️ Option 2: Drag-and-Drop

- Drag a `.ch8` game file from the `games/` folder onto `main.exe`.
- The game will launch automatically.

---

## 🎮 Keypad Mapping

The CHIP-8 hex-based keypad is mapped to your standard keyboard as follows:

```
CHIP-8     KEYBOARD
1 2 3 C    1 2 3 4
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
```

Use these keys to control the games as you would on a CHIP-8 device.

---

## 📦 ROMs

Place `.ch8` ROM files in the `games/` folder. You can find many CHIP-8 games online.

---

## 📢 Sound

- The emulator plays a simple beep using `beep.wav` when the sound timer is active.
- You can add your own custom beep. Just make sure that the total play time is less than 150-200 ms and uniform since it'd be repeated multiples times to make long sounds.
- Make sure it's in `.wav` format. Keep name as `beep.wav` only.

---

## 📌 Notes

- While running, make sure the path of games is set correctly. Use **relative path** only.

---

## 📸 Screenshots

[![Screenshot-2025-06-26-222438.png](https://i.postimg.cc/Y2XCQjBq/Screenshot-2025-06-26-222438.png)](https://postimg.cc/8s6GSpCq)
[![Screenshot-2025-06-26-222238.png](https://i.postimg.cc/yxfxd8sx/Screenshot-2025-06-26-222238.png)](https://postimg.cc/30vYf7yQ)
[![Screenshot-2025-06-26-222355.png](https://i.postimg.cc/9MbF8Q4b/Screenshot-2025-06-26-222355.png)](https://postimg.cc/8JFSsDmf)
[![Screenshot-12.png](https://i.postimg.cc/K8WTfJXj/Screenshot-12.png)](https://postimg.cc/75gbY1Zr)

---

## 🧠 Credits & License

- Took help of [Cowgod's Chip8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.5)
- SDL2 licensed under [zlib License](https://www.libsdl.org/license.php)
- This was a hobby project. I tried to make it as simple as possible. If you want to discuss regarding the same, please message me on [X/Twitter](https://x.com/uditmore99)

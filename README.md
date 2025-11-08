# 📞 Ultimate Phonebook CLI — Demo-Perfect Edition  

[![Made with C](https://img.shields.io/badge/Made%20with-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg)]()
[![Status](https://img.shields.io/badge/Version-1.0.0-yellow.svg)]()

> A visually rich, secure, and modular **Phonebook system** built entirely in **C**.  
> Combines strong fundamentals, file persistence, and a modern terminal-dashboard UX.

---

## ✨ **Features**

✅ Modern **Terminal Dashboard** UI  
✅ **Dynamic memory** with `realloc()` growth  
✅ **Binary I/O** persistence (`contacts.dat`)  
✅ **Cross-platform password masking** (Windows / Linux / macOS)  
✅ **Color-coded UX** for pro-grade clarity  
✅ Add • Search • Delete • Save — All from CLI  
✅ Minimal & clean code style for GitHub display  

---

## 🖼️ **Preview**

┌──────────────────────────────────────────────┐
│ 📞 Ultimate Phonebook CLI Dashboard │
└──────────────────────────────────────────────┘
A Modern CLI Experience — by Tridib Dey
──────────────────────────────────────────────
✨ Add • Search • Authenticate • Save • Delete ✨
──────────────────────────────────────────────

yaml
Copy code

---

## ⚙️ **Build & Run**

### 🧰 Requirements
- GCC or Clang (C99 compliant)
- ANSI-supported terminal (most modern ones are)
- Optional: Windows Console with Virtual Terminal Processing enabled

### 🧪 Build
```bash
gcc phonebook_pro.c -o phonebook_pro
▶️ Run
bash
Copy code
./phonebook_pro
📂 Project Structure
scss
Copy code
phonebook_pro.c   → main program file
contacts.dat      → binary save file (auto-generated)
README.md         → documentation (you’re reading it)
🧩 Core Concepts Used
Concept	Description
Dynamic Memory	Automatic array resizing with realloc()
File Handling	Binary save/load using fwrite and fread
String Sanitization	Custom fgets_trim() helper
Terminal UI	ANSI color codes, progress bars, borders
Cross-Platform	Works on Windows, Linux, and macOS

🔐 Security Features
Masked password input

Strong password rules (configurable)

Optional encryption layer (coming soon 🔒)

🧠 Learning Outcomes
By studying or contributing to this project, you’ll:

Strengthen your C language foundations

Understand modular program design

Learn real-world file I/O & memory handling

Practice clean, readable, and professional code

🪄 Author
👨‍💻 Tridib Dey
🚀 Aspiring Full-Stack + AI Developer & Tech Entrepreneur
📅 Created: November 2025
🌍 Location: Asansol, West Bengal, India

🪪 License
This project is licensed under the MIT License.
You are free to use, modify, and distribute this code with attribution.

💬 Acknowledgments
Special thanks to:

The C programming community ❤️

The gcc & clang compiler teams

Everyone learning C the right way — by building real projects!


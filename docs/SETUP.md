# Setting up the BeastVR environment (Windows 10/11, x64)

Both of you do every step. Takes about an hour, mostly downloads.

## 1. Core build tools

Open **PowerShell** (normal, not admin) and run:

```powershell
winget install Git.Git
winget install GitHub.cli
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install Microsoft.VisualStudio.2022.Community
```

When the Visual Studio Installer opens, tick **Desktop development with C++**. Make sure these are checked on the right side:
- MSVC v143 x64/x86 build tools
- Windows 11 SDK (latest)
- C++ CMake tools for Windows

Close and reopen PowerShell so the new tools are on PATH.

## 2. VR runtime

You need an **OpenXR runtime** on the PC, same as Spidy:
- Quest 3 through **Virtual Desktop** (best quality), or
- **Meta Quest Link** (the Meta Horizon Link app), or
- **SteamVR** with Steam Link.

Whichever you use, set it as the active OpenXR runtime in its settings. Test that some other PC VR game works before blaming our code.

## 3. Reverse engineering tools

| Tool | What we use it for | Where |
| --- | --- | --- |
| **Cheat Engine** | Finding player position, camera, health values | cheatengine.org (the installer bundles adware offers, decline them all) |
| **ReClass.NET** | Mapping the layout of game objects once found | GitHub: ReClass.NET releases |
| **x64dbg** | Breakpoints, call stacks, finding who writes a value | x64dbg.com |
| **Ghidra** | Static analysis of the game exe (needs a JDK, Ghidra's docs say which) | ghidra-sre.org |
| **PIX** or **RenderDoc** | Seeing how the game renders a frame (draw order, passes) | Microsoft PIX site / renderdoc.org |

Only ever attach these to the game **offline, in single player**. See the anti-cheat question in RECON.md first.

## 4. Unity (feel lab)

Unity Hub with a recent Unity 6 LTS, plus the **XR Interaction Toolkit** and **OpenXR Plugin** packages. The `lab/` folder explains the prototype project.

## 5. Create the GitHub repo (one of you)

From inside the BeastVR folder:

```powershell
git init
git add .
git commit -m "Initial scaffold"
gh auth login
gh repo create BeastVR --public --source . --push
```

Then on GitHub: **Settings > Collaborators > Add people** and add your partner. Also **Settings > Branches**, add a rule for `main`: require a pull request before merging.

Pick a license before the first release. GitHub's **Add file > Create new file**, name it `LICENSE`, and it offers templates. GPL-3.0 means anyone who builds on BeastVR has to stay open source too (Spidy uses it, and if we ever reuse Spidy code we **must** use GPL-3.0). MIT is looser. Decide together.

## 6. The other person clones

```powershell
gh repo clone <owner>/BeastVR
cd BeastVR
```

## 7. Build

```powershell
.\tools\bootstrap.ps1
.\tools\build.ps1
```

First build downloads MinHook, OpenXR SDK and spdlog, so it takes a few minutes. Output lands in `build\bin\`:
- `beastvr.dll`
- `BeastVR Launcher.exe`
- `beastvr.ini` (copied from `config\`)

## 8. First run (Phase 0 test)

1. Set the game to **DirectX 11** in its graphics settings, restart it, then quit.
2. Edit `build\bin\beastvr.ini`: put the real exe name in `game_process`.
3. Run `BeastVR Launcher.exe`. It waits.
4. Start the game from Steam. The launcher loads `beastvr.dll` as soon as the game process appears.
5. Open `%LOCALAPPDATA%\BeastVR\logs\beastvr.log`. You want to see `Present hook live` and `OpenXR system: ...`.

If the game crashes or refuses to start with the DLL, stop and write down exactly what happened in RECON.md before trying again.

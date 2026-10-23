# BeastVR

**Free, open source VR mod for Dying Light: The Beast.**
Goal: full motion controls, physical parkour in the style of Assassin's Creed Nexus, and physics melee that keeps (and amplifies) the game's gore.

> Status: **Phase 0, environment and recon.** Nothing playable yet.

BeastVR is a fan project. It is not affiliated with or endorsed by Techland. It contains no game files or assets; you need your own copy of the game.

## Design rule

Don't fake the game, drive it. Our code decides what your head and hands are doing, then calls the game's own systems (camera, hits, dismemberment, ragdolls, climbing) so everything still looks and feels like Dying Light.

## Repo layout

| Path | What it is |
| --- | --- |
| `src/core/` | `beastvr.dll`, loaded into the running game. Hooks, logging, VR runtime. |
| `src/launcher/` | `BeastVR Launcher.exe`. Waits for the game and loads the DLL. Touches nothing in the game folder. |
| `lab/` | Unity prototype for feel (climbing, melee) with no game needed. |
| `docs/` | Roadmap, setup, recon notes, design notes. |
| `tools/` | Build and environment scripts. |

## Build

See [docs/SETUP.md](docs/SETUP.md) for the full environment. Short version:

```powershell
.\tools\bootstrap.ps1   # checks your tools
.\tools\build.ps1       # configures and builds into build\
```

## Docs

- [Roadmap](docs/ROADMAP.md): phases, exit criteria, who does what
- [Setup](docs/SETUP.md): everything to install, step by step
- [Recon notes](docs/RECON.md): what we have learned about the game's internals
- [Contributing](CONTRIBUTING.md): branches, PRs, commit style

# Contributing

## Branches

- `main` always builds. Never push straight to it.
- Work on a branch named after what you're doing: `camera-hook`, `ik-hands`, `melee-sweep`.
- Open a pull request, the other person skims it, merge.

```powershell
git switch -c camera-hook
# ...work...
git add .
git commit -m "Hook camera update, log position each frame"
git push -u origin camera-hook
gh pr create --fill
```

## Commits

Short present-tense summary of what changed. One idea per commit.

## Recon

Any address, signature or struct you find goes into `docs/RECON.md` in the same PR as the code that uses it, with the game version.

## Don't commit

- Game files, game assets, or dumps of the game exe.
- Cheat Engine tables with absolute addresses (they're useless next launch anyway). Pointer paths and signatures are fine.
- Your `beastvr.ini` with personal paths (the template lives in `config/`).

## Testing rules

- Offline, single player only, while the mod is in development.
- Back up your save folder before testing anything that touches gameplay.

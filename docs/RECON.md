# Recon notes

Everything we learn about the game's internals goes here. Always record the **game version** and **date** next to a finding, since patches move things.

## Game basics

| Question | Answer | Game version | Who / date |
| --- | --- | --- | --- |
| Exe name | ? | | |
| Install path (Steam) | `...\Dying Light The Beast\ph_ft\work\bin\x64\` (confirm) | | |
| Anti-cheat present? (EasyAntiCheat / BattlEye folders, services) | ? | | |
| How do existing mods load? (pak files in `ph_ft\mods` + merger; any DLL mods?) | | | |
| Renderer options | DX11 and DX12 both available | | |
| Which renderer we target first | **DX11** | | |
| Does Steam relaunch the exe if started directly? | ? | | |

## Camera

| Thing | Pointer path / AOB signature | Type / layout | Notes |
| --- | --- | --- | --- |
| Player position | | | |
| Camera position | | | |
| Camera rotation | | | |
| FOV | | | |
| Function that writes camera each frame | | | |

## Rendering

| Thing | Finding |
| --- | --- |
| Scene view submit function | |
| Projection matrix build | |
| Where HUD is drawn (separate pass?) | |

## Gameplay

| Thing | Finding |
| --- | --- |
| Melee hit function | |
| Dismemberment call | |
| Ragdoll impulse | |
| Player mover / controller | |
| Climb / ledge state machine | |
| First-person arm skeleton | |

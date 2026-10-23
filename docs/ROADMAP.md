# BeastVR Roadmap

Each phase has an **exit test**. Don't start the next phase until the current one passes. Two people: call them **A** (rendering and engine hooks) and **B** (input, physics, gameplay feel). Swap roles whenever you want, but keep one owner per phase.

---

## Phase 0: Environment and recon (week 1 to 2)

Goal: both of you can build, load our DLL into the game, and read a log.

1. Both: install everything in [SETUP.md](SETUP.md), clone the repo, run `tools\build.ps1`.
2. Both: answer the recon questions in [RECON.md](RECON.md):
   - Exact game exe name and path (Task Manager, Details tab).
   - Does the game ship an anti-cheat? Look for `EasyAntiCheat`, `BattlEye` folders or services in the install. **If yes, we only ever test offline, single player, and we figure out how other mods load before injecting anything.**
   - Which renderer is active. The game supports both DX11 and DX12. **We start on DX11**: it is far simpler to hook and OpenXR supports it directly. DX12 comes after the mod works.
3. A: put the exe name in `beastvr.ini`, run the launcher, start the game, confirm `beastvr.log` shows the Present hook firing and the swapchain resolution.
4. B: confirm the log shows your headset name from the OpenXR probe.

**Exit test:** log shows `Present hook live`, a resolution, and `OpenXR system: <your headset>`.

---

## Phase 1: Find the camera (week 2 to 5)

Goal: we control the game camera from our code.

1. A: with Cheat Engine, find the player position (move, scan for changed floats, narrow down). Then find the camera's position and rotation near it.
2. A: in ReClass.NET, map the camera object: position, rotation (quaternion or matrix), FOV, near plane. Find a stable pointer path or a code signature (AOB) to it, since raw addresses change every launch.
3. A: in x64dbg or Ghidra, find the function that writes the camera each frame. Hook it with MinHook.
4. B: build the debug overlay (Dear ImGui on top of Present) showing camera values live.
5. Both: write everything to RECON.md, including the game version it was found on.

**Exit test:** a hotkey that rotates the in-game camera from our code, plus a "freecam" toggle.

---

## Phase 2: Seated stereo VR (week 5 to 10)

Goal: sit in the game, look around in 3D with your head.

1. A: create an OpenXR session with `XR_KHR_D3D11_enable` using the game's D3D11 device.
2. A: first version, **alternate eye rendering** (left eye on even frames, right on odd). Ugly but proves the pipeline.
3. A: real version, **two views per frame** through the game's renderer (what Spidy does). Requires finding how the engine submits a scene view so we can call it twice with offset cameras. This is the single hardest engineering task in the project.
4. B: write head pose into the camera hook each frame. Kill head bob, camera shake, and animation-driven camera motion (vaults, climbs, landings). The head must belong to the player, always.
5. B: render HUD and menus onto a floating screen quad when the game is in a menu or cutscene.

**Exit test:** 20 minutes seated in Old Town without nausea, correct world scale (doorways look door-sized).

---

## Phase 3: Hands (week 10 to 14)

Goal: the first-person arms follow your controllers.

1. A: find the first-person arm skeleton and the bone transform buffer.
2. B: controller poses in world space, then two-bone IK so the hand bones land on the controllers. Weapon attaches to the hand bone.
3. B: one-time T-pose calibration for arm length and height (same idea as Spidy).
4. B: button mapping layer: every gamepad action still reachable (D-pad functions go to a wrist menu or gestures).

**Exit test:** look down, see your hands and weapon tracking 1:1 with no visible lag.

---

## Phase 4: Physics melee and gore (week 14 to 20)

Goal: swinging your arm is the attack, and the game does the gore.

1. A: find the melee hit function (breakpoint on zombie health decreasing, walk back the call stack) and the dismemberment call.
2. B: weapon collider swept along the controller path each frame. On contact, call the game's hit with:
   - damage scaled by swing speed (a slow tap does nothing),
   - hit position and swing direction, so the game picks the limb (neck slash, leg chop),
   - impulse along the swing for ragdolls.
3. B: stab and lodge: a thrust into a skull keeps the blade stuck until you yank it out.
4. B: two-handed grip for heavy weapons, physical throwing, haptic pulse scaled by impact.
5. Both: shove gesture against biter grabs instead of mashing.

**Exit test:** you can decapitate a biter with a deliberate swing and not with a lazy flick.

---

## Phase 5: Locomotion and Nexus-style parkour (week 20 to 30)

Goal: climb with your hands.

1. B: smooth stick locomotion, snap and smooth turn, run on stick click or arm pump.
2. A: find the player movement controller and the ledge detection / climb state machine.
3. B: our own climb solver: grip on a ledge or pipe locks that hand, pulling down moves the body up, the solver drives the game's mover against the game's collision world.
4. B: vault by planting hands on an obstacle while moving; wall launch by pulling sharply away from a grabbed wall.
5. B: comfort: vignette on falls, optional reduced fall camera, all toggleable.

**Exit test:** climb a building in Old Town hand over hand, no button-press climbs.

---

## Phase 6: The Beast stuff (week 30+)

- Beast Mode as bare-hand tearing (two-hand grab and pull to dismember).
- Kicks (button or stomp gesture), grapple if the game gives it to you.
- Throwables, firearms aiming down a real sight.

## Phase 7: Release polish (ongoing from Phase 2)

- In-game VR settings tab (ImGui first, native menu later).
- Launcher: checks game version, OpenXR runtime, memory, writes session reports.
- GitHub Actions release zips, changelog, players' guide.
- **Version lock:** every game patch can break hooks. Prefer AOB signatures over hard offsets, and the launcher refuses unknown game versions with a clear message.

---

## Feel prototyping in parallel

From Phase 0 on, B builds climbing and melee feel in the Unity `lab/` on Quest 3. When the game hooks are ready, the tuned numbers and gesture logic port straight into C++. This way design work never waits on reverse engineering.

# BeastVR Lab (Unity)

A small Unity scene for tuning how things **feel**, with no game and no reverse engineering. Same idea as Spidy's standalone swinging lab.

## Set up

1. Unity Hub > New project > **3D (URP)**, Unity 6 LTS, create it inside this `lab/` folder as `BeastVRLab`.
2. Package Manager: **XR Interaction Toolkit** and **OpenXR Plugin**. Project Settings > XR Plug-in Management > OpenXR, add the Meta Quest Touch controller profile.
3. Build a grey-box rooftop course: ledges at a few heights, pipes, a gap to jump, a wall to launch off.
4. Add ragdoll dummies (Unity's Ragdoll Wizard on any humanoid) for melee.

## What to prototype (in this order)

1. **Melee feel.** Swept weapon collider, damage from swing speed, a minimum speed threshold, impulse along the swing direction. Log the numbers that feel right.
2. **Stab and lodge.** Blade sticks in the dummy past a depth, pulls free past a yank speed.
3. **Hand-over-hand climbing.** Grip locks a hand to a ledge, body moves opposite to hand motion, release and re-grab higher.
4. **Vault and wall launch.** Hands planted on an obstacle while moving, sharp pull away from a wall.
5. **Comfort.** Vignette strength on falls, snap vs smooth turn.

## Rule

Every tuned number goes into `lab/TUNING.md` with a sentence on why. That file is what gets ported into the C++ mod later, so design never waits on the reverse engineering.

# SecureVoice PC Simulation

This folder lets you run the firmware `src/main.c` on a PC without editing the original source files.

The simulator links `src/main.c` with `pc_stubs.c` instead of the STM32 HAL transport/audio/crypto modules. It runs five loopback frames, prints each step, then exits automatically.

## Run

From the project root:

```powershell
.\pc_sim\build_and_run.ps1
```

If PowerShell blocks scripts, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\pc_sim\build_and_run.ps1
```

## Compiler Needed

The script looks for `gcc` first, then `clang`.

If neither exists, install one of these and make sure it is in PATH:

- MinGW-w64 GCC
- LLVM Clang

## What Is Simulated

- Audio capture generates deterministic test bytes.
- Crypto uses a reversible XOR transform.
- Transport sends packets through an in-memory loopback.
- Playback prints the first few decoded bytes.
- System health functions print task activity.

No files in `src` or `inc` are modified by this simulator.

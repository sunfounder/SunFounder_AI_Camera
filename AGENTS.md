# AGENTS.md

Guidance for AI coding agents working in this repository (SunFounder AI Camera, an Arduino
library). Humans are welcome to read it too.

## What this is

An Arduino library for the **ESP32-CAM module used in SunFounder kits** (GalaxyRVR, PiCar-X,
Pico 4WD, ...). The board talks to the camera over UART and the camera answers with a small
line protocol; the library wraps that so a sketch can configure the camera, read the SunFounder
Controller app's widgets and drive the camera from the board.

## Layout

```
src/SunFounder_AI_Camera.h    class declaration, protocol keywords, version macros
src/SunFounder_AI_Camera.cpp  serial protocol, widgets, camera control
library.properties            Arduino Library Manager metadata - bump "version" on release
CHANGELOG.md                  release notes (English)
README.md                     user-facing API documentation
```

There is no test folder: the library only runs against real hardware, so verification is a
compile check plus a board test (see "Verifying a change").

## Protocol in one screen

- Host sends ASCII lines: `SET+<CMD><value>` (for example `SET+START`), the camera answers
  `[OK] <payload>` or `[ERROR] ...`.
- `SET+RESET` returns the camera firmware version, `SET+START` starts the servers and returns
  the IP address.
- Widget data arrives asynchronously as `WS+{json}` lines, binary frames start with `WSB+`.
- Debug output from the camera is prefixed `[CAM_A]` / `[CAM_E]` / `[CAM_I]` / `[CAM_D]`.

Two serial ports are used (`Arduino.h` branch `ARDUINO_MINIMA`): `DataSerial` for the camera,
`DebugSerial` for the user. On most boards both map to `Serial`.

Camera firmware generations speak different commands - see the compatibility table in
`README.md`. 1.4.x must be configured with the legacy command set (`SSID` / `PSK` / `MODE`)
and gets `cameraFirmwareLegacy = true`; 1.5.x+ uses `APSSID` / `APPSK` / `PORT` / `START`.

## Rules that keep this library working

1. **Serial replies must be read line by line.** The camera sends at 115200 and a reply such as
   `[OK] 192.168.4.1\n` often arrives in several chunks (it is busy starting its servers right
   after `SET+START`). `readInto()` therefore accumulates into `recvBuffer` / `recvLen` and
   returns `true` only once the line's `\n` arrived; `command()` and `loop()` must ignore a
   partial line instead of parsing it. Returning early with half a line is what made boards
   print `http://192.168.:9000/mjpg`.
2. **Every copy into a caller buffer stays bounded.** Replies can be longer than the caller's
   buffer (`[OK] NAME already set`), which used to overflow a 10-byte stack buffer on every
   boot. Use `AI_CAM_REPLY_MAX` and keep it <= the smallest buffer a caller passes.
3. **`recvBuffer` is `uint8_t[]`** because binary frames use it too. When it is handled as a
   string, go through the `RECV_STR` macro - the ESP32 core 3.x toolchain rejects the implicit
   `uint8_t* -> char*` conversion that the AVR build accepted.
4. **Respect `WS_BUFFER_SIZE` (200).** A longer line must be truncated with a warning, never
   written past the end of the buffer.
5. **Keep the two camera generations working.** `isLegacyFirmware()` / `cameraFirmwareLegacy`
   exist so a 1.4.x camera still comes up as an access point and can be updated over the air;
   never make `command()` hang or `while (1)` on an unexpected reply.

## Versioning and release

`AI_CAMERA_VERSION_CODE` is `major * 10000 + minor * 100 + patch`, so sketches can require a
minimum version at compile time (GalaxyRVR does). On a release, bump both:

- `library.properties` -> `version`
- `src/SunFounder_AI_Camera.h` -> `AI_CAMERA_VERSION_MAJOR` / `_MINOR` / `_PATCH` and
  `AI_CAMERA_VERSION_CODE`

then add a CHANGELOG entry, commit to `main`, and **tag + publish a release with the same
number**. Arduino's Library Manager indexes a new version only from that tag, so a merged PR
without a tag does not reach users.

```bash
git tag 1.2.3 && git push origin 1.2.3
gh release create 1.2.3 --repo sunfounder/SunFounder_AI_Camera --title "1.2.3" --notes-file notes.md
```

## Verifying a change

There is no native test setup in this repo, and the MSYS2 g++ on our Windows box does not run,
so the base check is a real compile against the ESP32 toolchain:

```bash
# flags come from any firmware project that uses this library, e.g.
#   <firmware-repo>/compile_commands.json  (PlatformIO) -> the entry for main.cpp
xtensa-esp32-elf-g++ -fsyntax-only <those -I flags> src/SunFounder_AI_Camera.cpp
```

This catches real problems: it is what surfaced the `uint8_t*` / `char*` conversion above.
For behaviour changes, the minimum bar is a board test - flash the firmware that uses the
library and watch the serial log (for example GalaxyRVR must print a complete
`http://192.168.4.1:9000/mjpg`).

## Writing rules

- **English everywhere in the repository**: code comments, commit messages, PR titles and
  bodies, CHANGELOG entries, release notes. Only conversations with the SunFounder team are in
  Chinese.
- Commit subjects follow `fix(scope): ...` / `feat(scope): ...`; keep the body explaining the
  root cause, not just the change.
- Do not reformat unrelated code; keep diffs reviewable.

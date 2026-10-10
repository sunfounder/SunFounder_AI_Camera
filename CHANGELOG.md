# Changelog

## [1.2.1] - 2026-10-10

### Fixed
- **Stack corruption on camera firmware 1.5.x.** `command()` copied the camera reply with an
  unbounded `strcpy()` into the 10-byte buffer the `set()` helpers pass in, while 1.5.x
  answers things like `[OK] NAME already set` (15 chars), `[OK] AP already set` (13) or
  `[OK] SSID already set`. Those replies come back on every boot once the camera has the
  setting stored, so `NAME` / `APSSID` / `APPSK` overflowed a 10-byte stack buffer by
  5-6 bytes. Depending on the build the corruption was harmless or fatal - on GalaxyRVR
  2.0.1 it reset the board in a loop right after the `NAME` reply. The copy is now bounded
  (`AI_CAM_REPLY_MAX`), and the `set()` buffers are that size.

## [1.2.0] - 2026-10-10

### Added
- Compatibility mode for 1.4.x ESP32-CAM firmware. `begin(ssid, password, wsPort, autoSend)`
  now configures those cameras with the legacy command set (`SSID` / `PSK` / `MODE`), which
  starts the camera access point and settings page so the camera firmware can be updated
  over the air. Previously a 1.4.x camera made `command()` fail and the library hung in
  `while (1)`, leaving the board stuck in `setup()`.
- Public members `cameraFirmwareVersion` and `cameraFirmwareLegacy`.
- `AI_CAMERA_VERSION_CODE` macro for compile-time version checks.

### Changed
- Cameras >= 1.5.0 keep the exact previous code path; cameras < 1.4.0 keep the previous
  "version not match" early return.

## [1.1.1] - 2024-12-30

- Previous release. No changelog was kept before this version.

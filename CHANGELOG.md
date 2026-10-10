# Changelog

## [1.2.0] - 2026-10-10

### Added
- Compatibility mode for 1.4.x ESP32-CAM firmware. `begin(ssid, password, wsPort, autoSend)`
  now configures those cameras with the legacy command set (`SSID` / `PSK` / `MODE`), which
  starts the camera access point and settings page so the camera firmware can be updated
  over the air. Previously a 1.4.x camera made `command()` fail and the library hung in
  `while (1)`, leaving the board stuck in `setup()`.
- Public members `cameraFirmwareVersion` and `cameraFirmwareTooOld`.
- `AI_CAMERA_VERSION_CODE` macro for compile-time version checks.

### Changed
- Cameras >= 1.5.0 keep the exact previous code path; cameras < 1.4.0 keep the previous
  "version not match" early return.

## [1.1.1] - 2024-12-30

- Previous release. No changelog was kept before this version.

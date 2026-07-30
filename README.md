# LXS Touch Controller Firmware Updater

A firmware updater tool for LXS touch controllers on ChromeOS.

## Description

`lxs_touch_updater` is a command-line utility for querying and updating
firmware on LXS touch controllers via HID raw interface. It is designed
to be used with the ChromeOS `platform/touch_updater` framework, running
under minijail with seccomp policies for sandboxing.

## Supported Devices

| VID    | PID    | Mode    |
|--------|--------|---------|
| 0x1FD2 | 0x5008 | Normal  |
| 0x1FD2 | 0x5007 | Normal  |
| 0x29BD | 0x5357 | DFU     |

## Usage

```bash
# Query current firmware version (must complete in <40ms)
lxs_touch_updater --get_current_version

# Update firmware with specified image
lxs_touch_updater --update /lib/firmware/lxs/Touch_Binary.img

# Get product ID
lxs_touch_updater --get_product_id

# Help
lxs_touch_updater --help
```

## Build

```bash
make
```

## License

BSD-Google — See LICENSE file.

# Fork Quick Start

1. Follow the instructions from [Setting Up Your QMK Environment](https://docs.qmk.fm/newbs_getting_started), but
   instead of running `qmk setup` run:
   ```bash
   qmk setup -H D:\desired_home_dir\qmk_firmware -b wireless_playground_jrs jacobrshields/keychron_qmk_firmware
   ```

2. To compile the Keychron V3 Max Dualshot 2 keymap, run:
   ```bash
   qmk compile -kb keychron/v3_max/ansi_encoder -km dualshot_2
   ```

3. To flash the keyboard, use [QMK Toolbox](https://qmk.fm/toolbox) with the compiled
   `keychron_v3_max_ansi_encoder_dualshot_2.bin`. You can leave the MCU option alone.

4. To use VIA, install it (see [releases](https://github.com/the-via/releases/releases)); go to the Settings tab; set
   Render Mode to 3D (otherwise VIA can't detect the keyboard
   [for some reason](https://github.com/the-via/app/issues/202)); enable Show Design tab; go to the Design tab; and
   load the definition from
   `qmk_firmware\keyboards\keychron\v3_max\ansi_encoder\keymaps\dualshot_2\v3_max_ansi_encoder_dualshot_2_via.json`.

# Quantum Mechanical Keyboard Firmware

[![Current Version](https://img.shields.io/github/tag/qmk/qmk_firmware.svg)](https://github.com/qmk/qmk_firmware/tags)
[![Discord](https://img.shields.io/discord/440868230475677696.svg)](https://discord.gg/Uq7gcHh)
[![Docs Status](https://img.shields.io/badge/docs-ready-orange.svg)](https://docs.qmk.fm)
[![GitHub contributors](https://img.shields.io/github/contributors/qmk/qmk_firmware.svg)](https://github.com/qmk/qmk_firmware/pulse/monthly)
[![GitHub forks](https://img.shields.io/github/forks/qmk/qmk_firmware.svg?style=social&label=Fork)](https://github.com/qmk/qmk_firmware/)

This is a keyboard firmware based on the [tmk\_keyboard firmware](https://github.com/tmk/tmk_keyboard) with some useful features for Atmel AVR and ARM controllers, and more specifically, the [OLKB product line](https://olkb.com), the [ErgoDox EZ](https://ergodox-ez.com) keyboard, and the Clueboard product line.

## Documentation

* [See the official documentation on docs.qmk.fm](https://docs.qmk.fm)

The docs are powered by [Docsify](https://docsify.js.org/) and hosted on [GitHub](/docs/). They are also viewable offline; see [Previewing the Documentation](https://docs.qmk.fm/#/contributing?id=previewing-the-documentation) for more details.

You can request changes by making a fork and opening a [pull request](https://github.com/qmk/qmk_firmware/pulls), or by clicking the "Edit this page" link at the bottom of any page.

## Supported Keyboards

* [Planck](/keyboards/planck/)
* [Preonic](/keyboards/preonic/)
* [ErgoDox EZ](/keyboards/ergodox_ez/)
* [Clueboard](/keyboards/clueboard/)
* [Cluepad](/keyboards/clueboard/17/)
* [Atreus](/keyboards/atreus/)

The project also includes community support for [lots of other keyboards](/keyboards/).

## Maintainers

QMK is developed and maintained by Jack Humbert of OLKB with contributions from the community, and of course, [Hasu](https://github.com/tmk). The OLKB product firmwares are maintained by [Jack Humbert](https://github.com/jackhumbert), the Ergodox EZ by [ZSA Technology Labs](https://github.com/zsa), the Clueboard by [Zach White](https://github.com/skullydazed), and the Atreus by [Phil Hagelberg](https://github.com/technomancy).

## Official Website

[qmk.fm](https://qmk.fm) is the official website of QMK, where you can find links to this page, the documentation, and the keyboards supported by QMK.

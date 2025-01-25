
# Flipper Zero Game of Life

## Description

A very basic implementation for Conway's "Game of Life" on Flipper Zero.

## Screenshot

![Screenshot](./Screenshot.png)

## Status

ALPHA - WORK IN PROGRESS WITH OPEN BUGS

Roadmap:

- Show current count of living cells
- Show current number of cycles
- Switch between fullscreen and Info-Line (with ok button)
- Detect end
- Init with different modes (with right and left keys)
  - Random
  - Blinker
  - Glider
  - Glider gun
  - Pentomino
  - Diehard
  - Acorn
- Adjustable speed (with up and down keys)
- Add startup screen

## Usage

Open Program via "Apps" -> "Games"

![Screenshot](./ScreenshotApp.png)

The return button ends the program.
All other buttons reset the cells to a random pattern.

## Background

<https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life>

## License

MIT License, see "License.txt".

## Installation

- Install [ufbt](https://github.com/flipperdevices/flipperzero-ufbt)
- Clone Project into local Projectfolder
- Attach Flipper Zero via USB
- Run "ufbt launch" within local Projectfolder

## Author

By domo

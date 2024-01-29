# interactive-film-engine
A game engine that allows people to make and export interactive films!

## Getting Started

### Stable (Released)
Download the latest version, for your operating system, from the releases section and then run it!

### Development (Source)

*Note: I recommend you do not use this version unless you need to or want to. It's usually better to use a stable version unless you're wanting an experimental feature.*

#### Linux/Unix

*Note: Unix has not been tested yet, however, I assume it will be working as similar.*

Download the git repo (via `git clone` or downloading as a zip) and put it somewhere! From there, run `cmake -B build` in the folder and then `cmake --build build -j4`.

#### Windows

*Note: Whilst MinGW may be supported by the other libraries used, we do not directly support it and will not offer support for it. If it works for you, great! However, we will not be directly supporting that as it has far too many issues. You should either use MSVC or clang-cl.*

Coming soon...

## Dependencies

- `cmake` (version 3.25 or higher)
- `ffmpeg`
- `imgui` (will auto download upon `cmake -B build`)
- `glfw` (will auto download upon `cmake -B build`)
- `glad` (will auto download upon `cmake -B build`)
- `glm` (will auto download upon `cmake -B build`)

### Linux Only:

- `libavcodec-dev`
- `libavformat-dev`
- `libswscale-dev`
- `xorg-dev`
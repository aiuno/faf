# faf

### A tool to easily download fonts on Linux 🐧 and MacOS 🍏

Even though most if not all Linux distros have font installers.
None of the ones i have seen download them for you.

This tool makes is as easy as finding an open source font you want and typing:
`faf -S [font name]`

#### Currently supports Google Fonts

With planned support for [FontSquirrel](https://fontsquirrel.com), as well as any other relevant font providers with relatively decent API's.

#### All fonts downloaded with faf are FREE

Google fonts, as well as FontSquirrel both provide only fonts which are free for commercial use.

#### How do i use this?

Get build dependencies:

- Arch/Manjaro:`# pacman -S gcc cmake`
- Debian/Ubuntu:`# apt-get install cmake build-essential`
- MacOS: `$ brew install cmake`

First clone the repository:
`git clone https://github.com/aiuno/faf.git`

Create and go to the source code build directory:
`mkdir faf/build && cd faf/build`

Build the thing:
`cmake -DCMAKE_BUILD_TYPE=Release .. && make`

Finally, install it:
`sudo make install`

Now you can use faf.

For convenience, here is the output of `faf -h`:

```
Usage:
    faf <operation> <options> [...]

operations:
    faf -S [fonts]                   Download font(s)
    faf -R [fonts]                   Remove installed font(s)
    faf -Q [fonts]                   Search for font(s)

options:
    --system                         Install fonts for all user
    --ignore <variant>(,variant)     Ignore a font variant
    --attend <weight>(,<weight>)     Download "extra" font weights

    extra weights:
        thin                         (100)
        extralight                   (200)
        light                        (300)
        medium                       (500)
        semibold                     (600)
        extrabold                    (800)
        heavy                        (900)
    variants:
        regular
        italic
        bold
```


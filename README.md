# NOCC
nocc is not a c compiler although it is // Lol

## Build
The cool thing about the build system i use here is that once you build make.c even if you make changes to make.c it will rebuild itself and then build main.c

### Deps
- You need [STC](https://github.com/secretval/stc) in your path

### Non debug build
```sh
gcc -o make make.c
./make build
```

### Debug build
```sh
gcc -o make make.c
./make build debug
```

## Nix
If you use nix to install this project it's as easy as: `nix profile install`

And if you want to develop on this project you can either use nix-direnv or just do: `nix develop`

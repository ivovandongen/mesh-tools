# Compiling

## Preparation
```shell
git submodule update --init --recursive
```

## Installing pre-requisites

```shell
brew install embree
```

## Building

```shell
mkdir build && cd build
cmake ..
make
```

# Usage

```shell
Usage:
  ao-cli [OPTION...]

  -i, --input arg           Input model file
  -o, --output arg          Output model file
  -r, --resolution arg      Output texture resolution (default: 0)
      --output-texture arg  Output texture file separately (default: "")
  -b, --blur arg            Blur kernel size (default: 5)
  -v, --verbose             Speak up!
  -h, --help                Print usage
```
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
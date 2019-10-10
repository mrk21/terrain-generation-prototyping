# Terrain generation prototyping

## Dependencies

- [OpenCV](https://opencv.org/): 4.x
- [libnoise](http://libnoise.sourceforge.net/): 1.0.0

## Setup

```sh
# install dependencies
brew install opencv

# build
mkdir gen
cd gen
cmake ..
make
./src/perlin_worms
```

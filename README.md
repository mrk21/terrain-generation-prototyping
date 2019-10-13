# Terrain generation prototyping

## Dependencies

- [libnoise](http://libnoise.sourceforge.net/): 1.0.x
- [OpenCV](https://opencv.org/): 4.x
- [OpenGL](https://opengl.org/)
- [GLFW](https://www.glfw.org/): 3.x

## Setup

```sh
# install dependencies
brew install opencv
brew install glfw

# build
mkdir gen
cd gen
cmake ..
make
./src/perlin_worms
```

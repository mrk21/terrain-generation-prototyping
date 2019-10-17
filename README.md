# Terrain generation prototyping

## Dependencies

- [Boost](https://www.boost.org/): 1.71.x
- [libnoise](http://libnoise.sourceforge.net/): 1.0.x
- [OpenCV](https://opencv.org/): 4.x
- [OpenGL](https://opengl.org/): 4.1
- [GLFW](https://www.glfw.org/): 3.x
- [glm](https://glm.g-truc.net/0.9.9/index.html)
  - [g-truc/glm: OpenGL Mathematics (GLM)](https://github.com/g-truc/glm)

## Setup

```sh
# install dependencies
brew install boost
brew install opencv
brew install glfw
brew install glm

# build
mkdir gen
cd gen
cmake ..
make
./src/perlin_worms
```

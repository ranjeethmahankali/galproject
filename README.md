# galproject

The goal of this project is to support [my YouTube
channel](https://www.youtube.com/channel/UCjkfxwk0EQI-ovUh8tXdX5A).

`galcore` contains all the algorithms, mostly computational geometry.

`galfunc` and `galview` together provide a means to use functional
programming to compose interactive demos using the algorithms
available in `galcore`. A "demo file" is a python file that defines
how the algorithms are composed for that demo. These demos can have
sliders, and other input fields that help you run and test the
algorithms interactively, while seeing the results in the 3d viewer.

### dependencies

vcpkg dependencies:
* boost, boost-python, boost-program-options
* python3
* glm
* glfw3
* GLEW
* imgui
* gtest
* tinyobjloader
* efsw
* freetype
* tbb
* pngpp

EFSW depends on `inotify`. It can be installed as follows:
```
sudo apt-get install inotify-tools
```

### building on linux

if all the dependencies are setup correctly, you can just run:

```
sh build.sh
```


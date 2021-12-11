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

## Building on Linux

You need to have python3.9 and a few other dependencies that you can install like this:
```
sudo apt update
sudo apt install -y inotify-tools libxmu-dev libxi-dev libgl-dev
sudo apt install -y libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev
pip3 install pytest-xdist
```

Now setup Vcpkg
```
git clone https://github.com/microsoft/vcpkg.git
sh vcpkg/booststrap-vcpkg.sh
```
Set the environment variable VCPKG_PATH to the path where vcpkg is cloned.

Building:

```
./build.sh Release
```

Runnning unit tests:
```
Release/galtest
```

Running script tests:
```
pytest scripts/
```



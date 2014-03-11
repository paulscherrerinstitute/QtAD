QtAD
====

areaDetector Viewer in Qt/OpenGL

Compile
-------
```
$ export EPICS_BASE=/your-epics/base/
$ export EPICS_HOST_ARCH=linux-x86_64
$ qmake 
$ make
```

Options
-------
```
   --prefix=<areaDetector NDArray>: channel prefix of NDStdArray
   --rate=<frame rate>: refresh with fixed frame rate or monitor for updates (<=0)
   --simple: show only the viewer window
   --fullscreen: fullscreen mode
   --dg=<[width][xheight][xoffset][yoffset]>: window geometry
```
* show viewer only 

  ```
  $ QtAD --prefix=13PS1:image1: --simple
  ```

Limitations
-----------
* data type can only be Int8 or Int16
* color type can only be RGB or mono
* image width and height must be even number. odd width or height produces slanted image.

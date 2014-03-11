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
--prefix=<areaDetector NDArray>
--rate=<refresh rate>
--simple
--dg=widthxheight+xpos+ypos
--fullscreen
```

Limitations
-----------
* data type can only be Int8 or Int16
* color type can only be RGB or mono
* image width and height must be even number. odd width or height produces slanted image.

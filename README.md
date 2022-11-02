# cppuprofile

This project provides a tiny Linux C++ profiling library for monitoring:
* execution time
* CPU usage
* memory usage

This library aims at collecting metrics on embedded devices to monitor device
performance while operating heavy tasks or booting for example. Those metrics can 
be useful to check that load is properly spread onto all CPU cores or 
that memory is not starved.

Metrics are stored in a CSV file (the path is configurable).

The library is lightweight, simple and easy to use. It can be easily used from an existing
application or integrated in a dedicated monitoring application.

## Tools

The project also brings a tool for displaying the different metrics in
a single view:

[IMG]

This tool is written in Python. It requires the following packages:
* numpy
* plotly
* pandas

```
$ pip3 install numpy plotly pandas
```

## Sample

The project provides a C++ sample application called `uprof-sample` 
that shows how to use the `cppuprofile` library.

## Build

The build process is based on CMake. Minimum version is 2.8.

```commandline
$ cmake --configure . -B ../build-cppuprofile
$ cmake --build ../build-cppuprofile
```

## License

This project is licensed under BSD-3-Clause license. See LICENSE file for any further information.

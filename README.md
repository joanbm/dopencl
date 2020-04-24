# dOpenCL

[![Build Status](https://travis-ci.org/joanbm/dopencl.svg?branch=master)](https://travis-ci.org/joanbm/dopencl)

This repository contains an old version of dOpenCL. dOpenCL has been created by a research group at the Uni Münster, Germany. Therefore all credit for the initial project goes to them. If you want to find out more about their awesome project visit their page at http://www.uni-muenster.de/PVS/en/research/dopencl/ or read the paper at http://ieeexplore.ieee.org/document/6270637/. Regarding the paper there are also sources that provide it for free. Basically dOpenCL allows programmers to access remote OpenCL devices that are connected to the host machine via network. As dOpenCL relies on API forwarding the OpenCL code doesn't have to be changed in order to function properly. Instead dOpenCL handles all the network transfers. This version of dOpenCL is heavily based on the C++ library Asio and has been abandoned by the research group as they switched to an internal solution for the network transfers, called 'The Real-Time Framework' (http://www.uni-muenster.de/PVS/en/research/rtf/index.html). Unfortunately, this dependency is not openly accessible, which is why this older version was utilized.

This version of dOpenCL has been modified to function with Aparapi. Aparapi allows OpenCL programmers to write and execute their Kernels in pure Java, which allows much faster progression and especially aids beginners to execute their first OpenCL programs on the CPU and GPU without knowledge about internals. If you want to find out more about Aparapi visit their github repository at https://github.com/aparapi/aparapi. In order to connect Aparapi and dOpenCL with each other a specialized fork was created at https://github.com/florianroesler/aparapi. It fixes some issues that arise when combining both libraries and enables to use several dynamic features like adding devices to the cluster at runtime.

For further information, see the following files:

* `README_Linux.txt`: Basic information about dOpenCL

* `INSTALL_Linux.txt`: How to build dOpenCL

* `COPYING` and `COPYING.academic`: Distribution licenses

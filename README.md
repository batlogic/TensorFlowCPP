Installing TensorFlow on existing C++ project inside a docker environment
====================================

In this document I briefly summarize the operations needed to install TensorFlow and some additional common widely used libraries inside a docker container, in order to integrate them into an existing C++ project.
The audience should have a good understanding of the C++ project building flow: compilation, linking, setting include search path, setting dynamic library search path, difference between dynamic linking and static linking.
The audience should also have a basic understanding of the Linux commands.


0. Assuming docker is already installed in your host machine type the following in order to enter the docker's virtual machine:
------------------------------------------------------------------------

0.a. sudo dockerd
0.b. docker run -d --name <**container name**> -p 20000:20000 < image name>   ⇒ create your docker container from an existing docker image
0.c. docker start <**container name**>
0.d. docker ps  ⇒ inspect that the container is running
0.e. docker exec -it <**container name**> bash  ⇒ enter your container's command line prompt

1. installing compilation and additional working tools inside the docker container:
--------------------------------------------
At this phase you are inside the docker container's command line prompt.
My container started without the latest revision of the Ubuntu's tools, and does not have the necessary compilation tools that are needed to build the libraries.

1.a. apt-get update && apt-get upgrade **⇒** updating your environment
1.b. apt-get install build-essential **⇒**  this will install gcc version 4.9 in my container
1.c. apt-get install gfortran **⇒** this will install the fortran compiler needed for Blas (Intel MKL and IPP) libraries

1.d. **Installing git:**
1.d.1. apt-get install git-core
1.d.2. git config --global user.name "Dror Meirovich"
1.d.3. git config --global user.email "dror@email.com"

1.e. **installing cmake:**
1.e.1. Note: "apt-get install cmake" gives you version 3.0.2 and I decided that it is too old for my purpose, so I will install it manually.
1.e.2. search for "cmake" in google and download latest version of the cmake tool (in my case it was ver. 3.8.2). https://cmake.org/download/
1e.2.a. If it is downloaded to your host machine, copy the zip file into the docker container: "docker cp <**host file name**>  <**container name**>:<**target file name**>
1.e.3. extract the cmake enter into its directory
1.e.4. ./configure **⇒** this script will check your current compiler and will make the necessary changes to the source code of cmake before its compilation
1.e.5. make **⇒** compilation of the cmake tool
1.e.6. ln -s  $PWD/bin/cmake /usr/bin/cmake **⇒** creating a soft link for cmake that could be called from anywhere inside the system

1.f. **installing cpio ⇒ needed for Intel MKL**
1.f.1. apt-get install cpio **⇒** will give you version 2.11
1.f.2. version 2.12 can be found in the Internet, but I don't think it is worth the time to install it manually when you can get 2.11 using apt-get https://www.gnu.org/software/cpio/

1.g. **Installing python 2.7:** Although I don't work with python at all, TensorFlow demands python installation in order to copy into it some binaries during its build process. So I decided to use "apt-get" which gives me an older version (2.7.9) instead of manual installation of a newer version (2.7.13) https://www.python.org/downloads/
1.g.1. apt-get install python python-pip python-setuptools python-numpy swig python-dev python-wheel **⇒** TensorFlow Dependencies
1.g.2. In case you decided to manually install version 2.7.13, it can be found in the Internet and the installation of this product is beyond the scope of this document.
1.g.2.a Note: In case library.so was not found during the installation of python... find the file: ld.so.conf and add into it the path to the missing library.so file, as describe in this document: https://stackoverflow.com/questions/20842732/libpython2-7-so-1-0-cannot-open-shared-object-file-no-such-file-or-directory

1.h. **installing bazel:** (I could not use "apt-get" because it collide with my already existing Java installation, I discovered that only when I stopped and tried to restart my docker container, it became unusable)
1.h.1. apt-get install pkg-config zip g++ zlib1g-dev unzip **⇒**  bazel dependencies
1.h.2. Download the binary installer with embedded JDK: bazel-0.5.1-installer-linux-x86_64.sh (or later) from the github https://github.com/bazelbuild/bazel/releases
1.h.3. chmod +x bazel-0.5.1-installer-linux-x86_64.sh
1.h.4. ./bazel-0.5.1-installer-linux-x86_64.sh --user
1.h.5.  ln -s /root/.bazel/bin/bazel /usr/bin/bazel

2. Install the C++ project's prerequisites libraries:
=====================================================

2.a. Create your projects main directory, it will contain all the source codes of your projects and its libraries. In my case I'm the root user and my existing home directory is: /root/
2.a.1. mkdir /root/projects
2.a.2. cd /root/projects

2.b. **Installing Intel MKL & IPP:**
2.b.1. Find and install Intel MKL and IPP: https://registrationcenter.intel.com/en/products/
2.b.2. After following the installation instructions, the source code of these libraries would be installed into: /opt/intel/mkl/include and /opt/intel/ipp/include respectively.
2.b.3. Don't forget to add these paths into the include path of your project.

2.c. **Installing boost:** I found that using "apt-get" gives boost version 1.55 and it is suffice for my project.
2.c.1. apt-get install libboost-all-dev **⇒** boost library will be installed in the library search path
2.c.2. Inspecting the boost library can be done with:
2.c.2.a. apt-get install aptitude
2.c.2.b. aptitude search boost
2.c.3. If you wish to use the latest version of boost, you can find it at: http://www.boost.org/users/download/#live
2.c.3.a. extract the archive, and enter into the extracted boost directory.
2.c.3.b. cd tools/build
2.c.3.c. ./bootstrap.sh
2.c.3.d. ./b2 install link=static runtime-link=static variant=release threading=multi **⇒** I decided to use the static libraries of boost in my project.
2.c.3.e. Add a symbolic link to the boost directory containing all the .hpp files inside /usr/include/

2.d. **Installing log4cplus:** The latest stable release (not RC) is version 1.2.0
2.d.1. download and extract the library from: https://github.com/log4cplus/log4cplus/releases
2.d.2. cd log4cplus_directory
2.d.3. export CXXFLAGS="-O3 -DNDEBUG -fopenmp -fPIC -m64 -v" **⇒** Setting compiler flags before building the library
2.d.4. ./configure --enable-static=yes
2.d.5. make install

2.e. **Installing vlfeat: **
2.e.1. The vlfeat library can be found at: http://www.vlfeat.org/
2.e.2. make
3.e.2. ar  rcs <**VLROOT**>/bin/glnxa64/libvl.a  <**VLROOT**>/bin/glnxa64/obj/*.o **⇒** The Makefile of this library doesn't create a static library artifact so I had to create it by myself

2.f. **Installing flann**
2.f.1. Download and extract the flann library from: https://github.com/mariusmuja/flann/releases
2.f.2. apt-get install liblz4-dev **⇒** flann dependency
2.f.3. cd flann_directory
2.f.4. cmake  -DCMAKE_BUILD_TYPE=Release --build . 
2.f.5. make
2.f.6. make install

2.g. **Installing Eigen3.3** (depending on boost and on MKL)
2.g.1. Download and extract the library from: http://eigen.tuxfamily.org/index.php?title=Main_Page
2.g.2. cd eigen3_directory
2.g.3. cat INSTALL **⇒** read the installation manual, the rest of this section was copied from this manual
2.g.4. mkdir build_dir
2.g.5. cd build_dir
2.g.6. cmake  -DCMAKE_BUILD_TYPE=Release --build ..
2.g.7. make install

3. Installing TensorFlow
========================
3.a. apt-get install autoconf autogen automake libtool curl make g++ unzip **⇒**  Protobuf Dependencies (a module inside TensorFlow)
3.b. git clone https://github.com/tensorflow/tensorflow.git
3.c. cd tensorflow
3.d. git checkout tags/v1.1.0 -b r1.1 **⇒** I had to use the latest stable revision of TF, in my case it was v1.1
3.e. export CXXFLAGS="-O3 -DNDEBUG -fopenmp -fPIC -m64 -v -I/usr/local/include/eigen3/ -I/opt/intel/mkl/include/ -I/opt/intel/ipp/include/" **⇒** setting compilation flags and include path before the compilation of TF
3.f. ./configure
3.g. bazel build -c opt --copt=-mavx --copt=-mavx2 --copt=-mfma --copt=-mfpmath=both --copt=-msse4.2  --copt=-msse4.1 --copt=-march=native tensorflow:libtensorflow_cc.so **⇒** I could not find a way to use a static library of TF
3.h. Copy the shared object into the library include path:
3.h.1. find . | grep libtensorflow_cc.so **⇒** Find the output of bazel
3.h.2. cp /root/.cache/bazel/_bazel_root/f44a66ea342b732c36ce6e6074c4ff93/execroot/tensorflow/bazel-out/local-opt/bin/tensorflow/libtensorflow_cc.so /usr/local/lib/libtensorflow_cc.so
3.h.3. chmod 644 /usr/local/lib/libtensorflow_cc.so 

4. Install your project...
==========================

4.a. git clone your_project
4.b. Compile it and fix all the dependency problems using the examples in this document...

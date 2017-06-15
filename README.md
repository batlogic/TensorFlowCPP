Installing TensorFlow on existing C++ project inside a docker environment
=

In this document I briefly summarize the operations needed to install TensorFlow and some additional common widely used libraries inside a docker container, in order to integrate them into an existing C++ project.
The audience should have a good understanding of the C++ project building flow: compilation, linking, setting include search path, setting dynamic library search path, difference between dynamic linking and static linking.
The audience should also have a basic understanding of the Linux commands.

A. Assuming docker is already installed in your host machine type the following in order to enter the docker's virtual machine:
-
  A.1. sudo dockerd

  A.2. docker run -d --name <**container name**> -p 20000:20000 < image name>   ⇒ create your docker container from an existing docker image

  A.3. docker start <**container name**>

  A.4. docker ps  ⇒ inspect that the container is running

  A.5. docker exec -it <**container name**> bash  ⇒ enter your container's command line prompt

B. installing compilation and additional working tools inside the docker container:
-
At this phase you are inside the docker container's command line prompt.
My container started without the latest revision of the Ubuntu's tools, and does not have the necessary compilation tools that are needed to build the libraries.

  B.1. apt-get update && apt-get upgrade **⇒** updating your environment
  
  B.2. apt-get install build-essential **⇒**  this will install gcc version 4.9 in my container
  
  B.3. apt-get install gfortran **⇒** this will install the fortran compiler needed for Blas (Intel MKL and IPP) libraries

  B.4. **Installing git:**
  
  B.4.a. apt-get install git-core
  
  B.4.b. git config --global user.name "Dror Meirovich"
  
  B.4.c. git config --global user.email "dror@email.com"

  B.5. **installing cmake:**
  
  B.5.a. Note: "apt-get install cmake" gives you version 3.0.2 and I decided that it is too old for my purpose, so I will install it manually.
  
  B.5.b. search for "cmake" in google and download latest version of the cmake tool (in my case it was ver. 3.8.2). https://cmake.org/download/
  
  B.5.b.1. If it is downloaded to your host machine, copy the zip file into the docker container: "docker cp <**host file name**>  <**container name**>:<**target file name**>
  
  B.5.c. extract the cmake enter into its directory
  
  B.5.d. ./configure **⇒** this script will check your current compiler and will make the necessary changes to the source code of cmake before its compilation
  
  B.5.e. make **⇒** compilation of the cmake tool
  
  B.5.f. ln -s  $PWD/bin/cmake /usr/bin/cmake **⇒** creating a soft link for cmake that could be called from anywhere inside the system

  B.6. **installing cpio ⇒ needed for Intel MKL**
  
  B.6.a. apt-get install cpio **⇒** will give you version 2.11
  
  B.6.b. version 2.12 can be found in the Internet, but I don't think it is worth the time to install it manually when you can get 2.11 using apt-get https://www.gnu.org/software/cpio/

  B.7. **Installing python 2.7:** Although I don't work with python at all, TensorFlow demands python installation in order to copy into it some binaries during its build process. So I decided to use "apt-get" which gives me an older version (2.7.9) instead of manual installation of a newer version (2.7.13) https://www.python.org/downloads/
  
  B.7.a. apt-get install python python-pip python-setuptools python-numpy swig python-dev python-wheel **⇒** TensorFlow Dependencies
  
  B.7.b. In case you decided to manually install version 2.7.13, it can be found in the Internet and the installation of this product is beyond the scope of this document.
  
  B.7.b.1. Note: In case library.so was not found during the installation of python... find the file: ld.so.conf and add into it the path to the missing library.so file, as describe in this document: https://stackoverflow.com/questions/20842732/libpython2-7-so-1-0-cannot-open-shared-object-file-no-such-file-or-directory

  B.8. **installing bazel:** (I could not use "apt-get" because it collide with my already existing Java installation, I discovered that only when I stopped and tried to restart my docker container, it became unusable)
  
  B.8.a. apt-get install pkg-config zip g++ zlib1g-dev unzip **⇒**  bazel dependencies
  
  B.8.b. Download the binary installer with embedded JDK: bazel-0.5.1-installer-linux-x86_64.sh (or later) from the github https://github.com/bazelbuild/bazel/releases
  
  B.8.c. chmod +x bazel-0.5.1-installer-linux-x86_64.sh
  
  B.8.d. ./bazel-0.5.1-installer-linux-x86_64.sh --user
  
  B.8.e.  ln -s $HOME/.bazel/bin/bazel /usr/bin/bazel
  

C. Install the C++ project's prerequisites libraries:
-

  C.1. Create your projects main directory, it will contain all the source codes of your projects and its libraries.
  
  C.1.a. mkdir $HOME/projects
  
  C.1.b. cd $HOME/projects

  C.2. **Installing Intel MKL & IPP:**
  
  C.2.a. Find and install Intel MKL and IPP: https://registrationcenter.intel.com/en/products/
  
  C.2.b. After following the installation instructions, the source code of these libraries would be installed into: /opt/intel/mkl/include and /opt/intel/ipp/include respectively.
  
  C.2.c. Don't forget to add these paths into the include path of your project.

  C.3. **Installing boost:** I found that using "apt-get" gives boost version 1.55 and it is suffice for my project.
  
  C.3.a. apt-get install libboost-all-dev **⇒** boost library will be installed in the library search path
  
  C.3.b. Inspecting the boost library can be done with:
  
  C.3.b.1. apt-get install aptitude
  
  C.3.b.2. aptitude search boost
  
  C.3.c. If you wish to use the latest version of boost, you can find it at: http://www.boost.org/users/download/#live
  
  C.3.c.1. extract the archive, and enter into the extracted boost directory.
  
  C.3.c.2. cd tools/build
  
  C.3.c.3. ./bootstrap.sh
  
  C.3.c.4. ./b2 install link=static runtime-link=static variant=release threading=multi **⇒** I decided to use the static libraries of boost in my project.
  
  C.3.c.5. Add a symbolic link to the boost directory containing all the .hpp files inside /usr/include/

  C.4. **Installing log4cplus:** The latest stable release (not RC) is version 1.2.0
  
  C.4.a. download and extract the library from: https://github.com/log4cplus/log4cplus/releases
  
  C.4.b. cd log4cplus_directory
  
  C.4.c. export CXXFLAGS="-O3 -DNDEBUG -fopenmp -fPIC -m64 -v" **⇒** Setting compiler flags before building the library
  
  C.4.d. ./configure --enable-static=yes
  
  C.4.e. make install

  C.5. **Installing vlfeat:**
  
  C.5.a. The vlfeat library can be found at: http://www.vlfeat.org/
  
  C.5.b. make
  
  C.5.c. ar  rcs <**VLROOT**>/bin/glnxa64/libvl.a  <**VLROOT**>/bin/glnxa64/obj/*.o **⇒** The Makefile of this library doesn't create a static library artifact so I had to create it by myself

  C.6. **Installing flann**
  
  C.6.a. Download and extract the flann library from: https://github.com/mariusmuja/flann/releases
  
  C.6.b. apt-get install liblz4-dev **⇒** flann dependency
  
  C.6.c. cd flann_directory
  
  C.6.d. cmake  -DCMAKE_BUILD_TYPE=Release --build . 
  
  C.6.e. make
  
  C.6.f. make install

  C.7. **Installing Eigen3.3** (depending on boost and on MKL)
  
  C.7.a. Download and extract the library from: http://eigen.tuxfamily.org/index.php?title=Main_Page
  
  C.7.b. cd eigen3_directory
  
  C.7.c. cat INSTALL **⇒** read the installation manual, the rest of this section was copied from this manual
  
  C.7.d. mkdir build_dir
  
  C.7.e. cd build_dir
  
  C.7.f. cmake  -DCMAKE_BUILD_TYPE=Release --build ..
  
  C.7.g. make install

D. Installing TensorFlow
  -
  D.1. apt-get install autoconf autogen automake libtool curl make g++ unzip **⇒**  Protobuf Dependencies (a module inside TensorFlow)
  
  D.2. git clone https://github.com/tensorflow/tensorflow.git
  
  D.3. cd tensorflow
  
  D.4. git checkout tags/v1.1.0 -b r1.1 **⇒** I had to use the latest stable revision of TF, in my case it was v1.1
  
  D.5. export CXXFLAGS="-O3 -DNDEBUG -fopenmp -fPIC -m64 -v -I/usr/local/include/eigen3/ -I/opt/intel/mkl/include/ -I/opt/intel/ipp/include/" **⇒** setting compilation flags and include path before the compilation of TF
  
  D.6. ./configure
  
  D.7. bazel build -c opt --copt=-mavx --copt=-mavx2 --copt=-mfma --copt=-mfpmath=both --copt=-msse4.2  --copt=-msse4.1 --copt=-march=native tensorflow:libtensorflow_cc.so **⇒** I could not find a way to use a static library of TF
  
  D.8. **Copy the shared object into the library include path:**
  
  D.8.a. cd $HOME
  
  D.8.b. find . | grep libtensorflow_cc.so **⇒** Find the (latest) output of bazel
  
  D.8.c. cp $HOME/.cache/bazel/_bazel_$USER/f44a66ea342b732c36ce6e6074c4ff93/execroot/tensorflow/bazel-out/local-opt/bin/tensorflow/libtensorflow_cc.so /usr/local/lib/libtensorflow_cc.so
  
  D.8.d. chmod 644 /usr/local/lib/libtensorflow_cc.so 

E. Install your project...
-

  E.1. git clone your_project
  
  E.2. Compile it and fix all the dependency problems using the examples in this document...

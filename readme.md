# gRPC image server
# About

This project was built from a prompt, where I was given an image.proto file and told to create a gRPC server from it that could modify images.

I ran and compiled this using Ubuntu 18.04 and uses the packages gRPC, Protoc, and OpenCV. All the code is
written in C++.

It's a relatively simple server that sends a image Mat file over a protobuf, modifies the image at the server, and then responds with the image to the client. The image manipulation you can do is mostly simple stuff like rotations and for fun I added an image to ascii generator. Compilation can't be done using CMake since there's an issue with the gRPC CMake installation (it's a known issue), so I just did it with a shell script which is working well for now so I haven't replaced it.

# Setup
## Install gRPC and Protobuf

First you'll want to install gRPC. I did this by downloading the source and compiling it onto my machine. You'll have to run the commands. Here we additionally install protoc as well, since it's a submodule of the gRPC repo.

```
sudo apt-get install build-essential autoconf libtool pkg-config
sudo apt-get install libgflags-dev libgtest-dev
sudo apt-get install clang libc++-dev

git clone https://github.com/grpc/grpc.git
git submodules --init

# install protoc
cd grpc/third_party/protobuf
sudo make install

# install gRPC
grpc/$ make
sudo make install
```

## Install OpenCV

I also installed opencv using the binaries. First you'll want to apt-get it's libraries, then you'll want to make the files and `sudo make install` so it'll be reachable in usr/lib. I basically just followed this [guide](https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html)

```
[compiler] sudo apt-get install build-essential
[required] sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
[optional] sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

git clone
cd OpenCV
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DOPENCV_GENERATE_PKGCONFIG=ON -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j4
sudo make install
```

I additionally had a a number of issues with the linker when building this project.
I'm not sure why that was however if you have linker issues you can try adding this file
and running this command.

```
cd /etc/ld.so.conf.d/
sudo touch opencv.conf
sudo vi opencv.conf
```

Then add the file path `/usr/local/lib/` to that file (which should be the path to your
.so opencv files). Now you'll want to leave the file and run the command

```
sudo ldconfig -v
```

This fixed my opencv linker issues, though if you don't have those issues you should be fine.

## Compile protobuf .pb and grpc.pb files

I saved the protobuf c files in a folder called compiled. Although there is a make file I ended up not using that because of linker issues and I just ran the commands in the terminal. To get the .pb.cc files, you can run the following commands:

```
mkdir compiled
protoc image.proto --cpp_out=compiled/
```

Which should generate two files; image.pb.cc and image.pb.h

You must additionally build the grpc files, which can be done by getting the path and then running this command

```
protoc --grpc_out=compiled/ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` image.proto
```

Or you can manually get the path, which should give you the same results.

```
which GRPC_CPP_PLUGIN
/usr/local/bin/GRPC_CPP_PLUGIN <- use this path

protoc --grpc_out=compiled/ --plugin=protoc-gen-grpc=/usr/local/bin/GRPC_CPP_PLUGIN image.proto
```

## Building the project

Finally you can build the actual project. I went a more unorthodox route and created a
build.sh file, which you can run to compile the server and client executables. Just run

```
cd proto
chmod +x build.sh
./build.sh
```

And it should compile the server.cpp and client.cpp code. Additionally if you haven't
already created the .pb.cc files you can the following to build them as well.

```
cd compiled/
chmod +x build.sh
./build.sh
```

And that should be it! You should now be able to start up the server, run the client, and pass
images between the gRPC server/client.

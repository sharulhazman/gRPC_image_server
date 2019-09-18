echo "building server..."
g++ -std=c++11 `pkg-config --cflags opencv4 protobuf grpc` server.cpp compiled/image.grpc.pb.cc compiled/image.pb.cc -L/usr/local/lib `pkg-config --libs opencv4 protobuf grpc++` -o server
echo "building client..."
g++ -std=c++11 `pkg-config --cflags opencv4 protobuf grpc` client.cpp compiled/image.grpc.pb.cc compiled/image.pb.cc -L/usr/local/lib `pkg-config --libs opencv4 protobuf grpc++` -o client
echo "build finished"

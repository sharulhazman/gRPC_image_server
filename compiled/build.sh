echo "Build the pb.cc files"
protoc image.proto --cpp_out=.
echo "Build the grpc.pb.cc files"
protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` image.proto

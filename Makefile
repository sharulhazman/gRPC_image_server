 LDFLAGS += -L/usr/local/lib `pkg-config --libs opencv4 protobuf grpc++`\
            -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
            -ldl

CXX = g++
CPPFLAGS += `pkg-config --cflags opencv4 protobuf grpc`
CXXFLAGS += -std=c++11

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

all: client server

client: image.pb.o image.grpc.pb.o client.o
	$(CXX) $^ $(LDFLAGS) -o $@

server: image.pb.o image.grpc.pb.o server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	protoc --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client server rotated_image.png fixed_image.png

all:
	g++ -Wall -o client.o client.cpp -I../src/ ../src/TCPServer.cpp ../src/TCPClient.cpp \
	../../protobuf/rem_nanopb_pb/mesh-packet.pb.c \
	../../../3rd-party/nanopb_protobuf/pb_encode.c \
	../../../3rd-party/nanopb_protobuf/pb_decode.c \
	../../../3rd-party/nanopb_protobuf/pb_common.c \
	../../RespirMeshClient/examples/RemHeaderTypes.cpp \
	-I../../../3rd-party/nanopb_protobuf/ \
	-I../../../3rd-party/nanopb_protobuf/extra/ \
	-I../../protobuf/rem_nanopb_pb/ \
	-I../../RespirMeshClient/examples/ \
	-std=c++11 -lpthread

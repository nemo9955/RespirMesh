all:
	g++ -Wall -o client_r1.o client_r1.cpp \
	\
	-I../../SimpleNetwork/src/ \
	../../SimpleNetwork/src/TCPServer.cpp \
	../../SimpleNetwork/src/TCPClient.cpp \
	\
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

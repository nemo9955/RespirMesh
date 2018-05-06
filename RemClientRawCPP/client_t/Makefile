all:
	g++ -Wall -o client_t.o client_t.cpp \
	\
	-I../../RemCppCommon/ \
	../../RemCppCommon/RemChannel.cpp \
	\
	-I../../protobuf/rem_nanopb_pb/ \
	../../protobuf/rem_nanopb_pb/mesh-packet.pb.c \
	\
	-I../../../3rd-party/nanopb_protobuf/ \
	-I../../../3rd-party/nanopb_protobuf/extra/ \
	../../../3rd-party/nanopb_protobuf/pb_encode.c \
	../../../3rd-party/nanopb_protobuf/pb_decode.c \
	../../../3rd-party/nanopb_protobuf/pb_common.c \
	\
	-I../../SimpleNetwork/src/ \
	../../SimpleNetwork/src/TCPServer.cpp \
	../../SimpleNetwork/src/TCPClient.cpp \
	\
	-std=c++11 -lpthread

	# -I../../RespirMeshClient/examples/ \
	# ../../RespirMeshClient/examples/RemHeaderTypes.cpp \
all:
	g++ -g -o0 -Wall -o client_libsocket.o client_libsocket.cpp \
	\
	-I../../RemCppCommon/ \
	../../RemCppCommon/RespirMesh.cpp \
	../../RemCppCommon/RespirMeshCommonApi.cpp \
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
	-I/usr/lib -L/usr/include/libsocket -std=c++11 -DVERBOSE -lsocket++ -lpthread


	# -I../../SimpleNetwork/src/ \
	# ../../SimpleNetwork/src/TCPServer.cpp \
	# ../../SimpleNetwork/src/TCPClient.cpp \
	# \

	# -std=c++11 -lpthread
	# -I../../RespirMeshClient/examples/ \
	# ../../RespirMeshClient/examples/RemHeaderTypes.cpp \
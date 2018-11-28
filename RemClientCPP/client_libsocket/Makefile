
all:
	g++ -g -o0 -Wall -o client_libsocket.o client_libsocket.cpp \
	\
	-I../../RemCppCommon/ \
	$(wildcard ../../RemCppCommon/**.cpp)  \
	$(wildcard ../../RemCppCommon/**.c)  \
	$(wildcard ../../RemCppCommon/X86_64/**.cpp)  \
	$(wildcard ../../RemCppCommon/X86_64/**.c)  \
	\
	-I../../protobuf/rem_nanopb_pb/ \
	$(wildcard ../../protobuf/rem_nanopb_pb/**.c)  \
	\
	-I../../../3rd-party/nanopb_protobuf/ \
	-I../../../3rd-party/nanopb_protobuf/extra/ \
	../../../3rd-party/nanopb_protobuf/pb_encode.c \
	../../../3rd-party/nanopb_protobuf/pb_decode.c \
	../../../3rd-party/nanopb_protobuf/pb_common.c \
	\
	-I/usr/lib -L/usr/include/libsocket -std=c++11 -DVERBOSE -lsocket++ -lpthread

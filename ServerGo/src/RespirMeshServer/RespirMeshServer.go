package remserver

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"math/rand"
	"strconv"
	"sync"
	"time"
	"unsafe"

	"../../../protobuf/rem_go_pb"
	"../RemHeaderTypes"
	"../RemLogs"
	"../RemTopology"
	"../tcp_server"

	"github.com/golang/protobuf/proto"
)

type RmServer struct {
	address          string // Address to open connection: localhost:9999
	timeoutDuration  time.Duration
	tcpServer        *tcp_server.TcpServer
	RemTopo          *remtopo.RemTopology
	RemLogs          *remlogs.RemLogs
	ServerSesionUUID uint32
	upTimeMils       int64
	RootUpdateFreq   time.Duration
	WGr              sync.WaitGroup
}

func (s *RmServer) getUpTimeDelta() uint32 {
	return uint32(time.Now().Unix()/int64(time.Microsecond) - s.upTimeMils)
}

func headerToBytesBasic(header *remheadertypes.RemBasicHeader) []byte {
	buf := bytes.Buffer{}
	binary.Write(&buf, binary.LittleEndian, header)
	return buf.Bytes()
}

func assembleStructBasic(header *remheadertypes.RemBasicHeader, dataStruct proto.Message) []byte {
	dataBytes, _ := proto.Marshal(dataStruct)
	headerBytes := headerToBytesBasic(header)
	packetBytes := append(headerBytes, dataBytes...)
	return packetBytes
}

func headerToBytesByte(header *remheadertypes.RemBasicHeader) []byte {
	buf := bytes.Buffer{}
	binary.Write(&buf, binary.LittleEndian, header)
	return buf.Bytes()
}

func assembleStructByte(header *remheadertypes.RemBasicHeader, dataStruct proto.Message) []byte {
	dataBytes, _ := proto.Marshal(dataStruct)
	headerBytes := headerToBytesByte(header)
	packetBytes := append(headerBytes, dataBytes...)
	return packetBytes
}

// func (s *RmServer) handleTimeSync(allData []byte, header *remheadertypes.RemBasicHeader) {
func (s *RmServer) handleTimeSync(allData []byte, c *tcp_server.Client) {
	header := remheadertypes.RemBasicHeader{}
	buf := bytes.NewReader(allData)
	err := binary.Read(buf, binary.LittleEndian, &header)
	if err != nil {
		fmt.Println(err)
		return
	}

	offsetHeader := unsafe.Sizeof(header)
	packetData := allData[offsetHeader:]

	timesync := &rem.RespirMeshTimeSync{}
	err = proto.Unmarshal(packetData, timesync)
	if err != nil {
		fmt.Println("unmarshaling error on TimeSync: ", err)
		return
	}
	timesync.ResponseArriveTime = s.getUpTimeDelta()
	timesync.ServerTimestampUnix = uint32(time.Now().Unix())
	timesync.ResponseSentTime = s.getUpTimeDelta()
	// header.ForwardingType = uint8(rem.RemHeaderForwardingType_TO_NODE)
	// header.ProtobufType = uint8(rem.RemHeaderProtobufType_REM_TIMESYNC)

	// respPacket := assembleStructBasic(header, timesync)
	// c.SendBytes(respPacket)

	fmt.Println("Time    !!!            ", s.getUpTimeDelta())
	fmt.Println("      RequestSentTime: ", timesync.GetRequestSentTime())
	fmt.Println("    RequestArriveTime: ", timesync.GetRequestArriveTime())
	fmt.Println("   ResponseArriveTime: ", timesync.GetResponseArriveTime())
	fmt.Println("  ServerTimestampUnix: ", timesync.GetServerTimestampUnix())
	fmt.Println("            source_id: ", "0x"+strconv.FormatInt(int64(timesync.GetInfo().GetSourceId()), 16))
	fmt.Println("         message_type: ", timesync.GetInfo().GetType())
	fmt.Println("     ResponseSentTime: ", timesync.GetResponseSentTime())
}

func (s *RmServer) handlePingPong(allData []byte, c *tcp_server.Client) {
	header := remheadertypes.RemBasicHeader{}
	buf := bytes.NewReader(allData)
	err := binary.Read(buf, binary.LittleEndian, &header)
	if err != nil {
		fmt.Println(err)
		return
	}

	offsetHeader := unsafe.Sizeof(header)
	packetData := allData[offsetHeader:]

	pingpong := &rem.RespirMeshInfo{}
	err = proto.Unmarshal(packetData, pingpong)
	if err != nil {
		fmt.Println("unmarshaling error on PingPong: ", err)
		return
	}

	fmt.Println("         Type: ", pingpong.GetType())
	fmt.Println("     TargetId: ", "0x"+strconv.FormatInt(int64(pingpong.GetTargetId()), 16))
	fmt.Println("     SourceId: ", "0x"+strconv.FormatInt(int64(pingpong.GetSourceId()), 16))
	fmt.Println("     TargetId: ", strconv.FormatInt(int64(pingpong.GetTargetId()), 10))
	fmt.Println("     SourceId: ", strconv.FormatInt(int64(pingpong.GetSourceId()), 10))

	header.ForwardingType = uint8(rem.ForwardingType_TO_NODE)

	pingpong.Type = rem.ProtobufType_PONG
	pingpong.TargetId = pingpong.GetSourceId()

	// respPacket := assembleStructByte(&header, pingpong)
	// c.SendBytes(respPacket)
}

func (s *RmServer) onNewBytes(c *tcp_server.Client, allData []byte) {
	// s.WGr.Wait()
	// s.WGr.Add(1)

	fmt.Print(" \n\n\n\n")
	fmt.Print("\n =============================================================== \n")
	fmt.Printf("\t onNewBytes   \n[% d]\n\n", allData)
	fmt.Print(" =============================================================== \n\n")

	firstChunk := remheadertypes.GetRemBasicHeader(allData)
	headerSize := (int)(unsafe.Sizeof(firstChunk))
	chBeg := 0
	chEnd := 0

	// fmt.Print("\n\n /////////////////////// \n")
	// for chEnd < len(allData) {
	// 	headerChunk := remheadertypes.GetRemBasicHeader(allData[chBeg:(chBeg + headerSize)])
	// 	chunkSize := headerChunk.PacketSize()
	// 	chEnd = chBeg + chunkSize
	// 	fmt.Println(" ... chBeg     ", chBeg)
	// 	fmt.Println(" ... chEnd     ", chEnd)
	// 	fmt.Println(" ... chunkSize ", chunkSize)
	// 	fmt.Printf("% d\n", allData[chBeg:chEnd])
	// 	chBeg = chEnd
	// }
	// fmt.Print("\n /////////////////////// \n")

	chBeg = 0
	for chEnd < len(allData) {
		headerChunk := remheadertypes.GetRemBasicHeader(allData[chBeg:(chBeg + headerSize)])
		chunkSize := headerChunk.PacketSize()
		chEnd = chBeg + chunkSize

		fmt.Print("\n\n")
		fmt.Println(" ... chBeg     ", chBeg)
		fmt.Println(" ... chEnd     ", chEnd)
		fmt.Println(" ... chunkSize ", chunkSize)
		s.onNewPacket(c, allData[chBeg:chEnd])
		chBeg = chEnd
	}
	// s.WGr.Done()
	// s.WGr.Wait()
}

func (s *RmServer) onNewPacket(c *tcp_server.Client, allData []byte) {
	fmt.Print("\n --------------------------------------------- \n")
	fmt.Printf("\t onNewPacket   \n[% d]\n\n", allData)
	fmt.Print(" --------------------------------------------- \n\n")
	header := remheadertypes.GetRemBasicHeader(allData)

	fmt.Println("PacketSize      ", header.PacketSize())
	fmt.Println("ForwardingType  ", header.ForwardingType, rem.ForwardingType(header.ForwardingType))
	fmt.Println("HeaderType      ", header.HeaderType, rem.HeaderType(header.HeaderType))
	fmt.Println("ProtobufType    ", header.ProtobufType, rem.ProtobufType(header.ProtobufType))
	// fmt.Printf("%+v\n", header)
	// utils.PrettyPrint("RemBasicHeader", header)
	// fmt.Println("ForwardingType HeaderType ProtobufType ", rem.ForwardingType(header.ForwardingType), rem.HeaderType(header.HeaderType), rem.ProtobufType(header.ProtobufType))
	// fmt.Println("Testing16Number    ", header.Testing16Number)
	// fmt.Println("Testing32Number    ", header.Testing32Number)

	// fmt.Printf(">>> [% d]\n", allData)

	// offsetHeader := unsafe.Sizeof(header)
	// packetData := allData[offsetHeader:]

	switch rem.ForwardingType(header.ForwardingType) {
	case rem.ForwardingType_TO_PARENT:
		fallthrough
	case rem.ForwardingType_TO_PARENT_TO_ROOT:
		fallthrough
	case rem.ForwardingType_TO_ROOT:
		switch rem.ProtobufType(header.ProtobufType) {
		case rem.ProtobufType_TIMESYNC:
			s.handleTimeSync(allData, c)
		case rem.ProtobufType_PING:
			header := remheadertypes.RemBasicHeader{}
			buf := bytes.NewReader(allData)
			// err := binary.Read(buf, binary.BigEndian, &header)
			err := binary.Read(buf, binary.LittleEndian, &header)
			if err != nil {
				fmt.Println(err)
				return
			}
			s.handlePingPong(allData, c)
		case rem.ProtobufType_MESH_TOPOLOGY:
			s.RemTopo.Handle(allData, s.ServerSesionUUID, c)
		case rem.ProtobufType_LOG:
			s.RemLogs.Handle(allData, c)
		}
	}

}

func (s *RmServer) onNewClient(c *tcp_server.Client) {
	fmt.Println("NEW CLIENT :")
	fmt.Println("             ", c.GetConn().RemoteAddr().Network())
	fmt.Println("             ", c.GetConn().RemoteAddr())
	fmt.Println("             ", c.GetConn().RemoteAddr().String())
	fmt.Println("             ", c.GetConn().LocalAddr())
	// c.Send("Hello")
}

func (s *RmServer) onClientConnectionClosed(c *tcp_server.Client, err error) {
	fmt.Println("client Disconnected !")
}

// StartRemServer hold comment
func (s *RmServer) StartRemServer() {
	go s.tcpServer.Listen()
	fmt.Println("TCP server started", s.tcpServer)
}

// UpdateRootNodePeriodically hold comment
func (s *RmServer) UpdateRootNodePeriodically() {
	go func() {
		for {
			s.RemTopo.CreateRoot(s.ServerSesionUUID)
			time.Sleep(s.RootUpdateFreq)
		}
	}()
}

// New Create and run (in the background) a RespirMeshServer
func New(address string, timeoutDuration time.Duration) *RmServer {

	// log.Println("server Started !!")
	// tcpServer := tcp_server.New(address, timeoutDuration)

	rmServer := RmServer{
		address:          address,
		timeoutDuration:  timeoutDuration,
		tcpServer:        tcp_server.New(address, timeoutDuration),
		ServerSesionUUID: uint32(rand.Int31()),
		upTimeMils:       time.Now().Unix() / int64(time.Microsecond),
		RemTopo:          remtopo.New(),
		RootUpdateFreq:   3 * time.Second,
		RemLogs:          remlogs.New(),
	}

	log.Println("DONE creating rmServer", rmServer)

	rmServer.UpdateRootNodePeriodically()

	// rmServer.RemTopo.MakeEdgeFromIDs(12345, rmServer.ServerSesionUUID)
	// rmServer.RemTopo.MakeEdgeFromIDs(12345, 7896)
	// rmServer.RemTopo.MakeEdgeFromIDs(12345, 484654)
	// rmServer.RemTopo.MakeEdgeFromIDs(7896, 854)

	rmServer.tcpServer.OnNewClient(rmServer.onNewClient)
	rmServer.tcpServer.OnNewBytes(rmServer.onNewBytes)
	rmServer.tcpServer.OnClientConnectionClosed(rmServer.onClientConnectionClosed)

	return &rmServer
}

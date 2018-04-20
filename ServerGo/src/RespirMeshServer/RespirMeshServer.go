package respirmeshserver

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"math/rand"
	"strconv"
	"time"
	"unsafe"

	"../../../protobuf/rem_go_pb"
	"../HeaderTypes"
	"../RemTopology"
	"../tcp_server"

	"github.com/golang/protobuf/proto"
)

type RmServer struct {
	address          string // Address to open connection: localhost:9999
	timeoutDuration  time.Duration
	tcpServer        *tcp_server.TcpServer
	RemTopo          *remtopo.RemTopology
	ServerSesionUUID uint32
	upTimeMils       int64
	RootUpdateFreq   time.Duration
}

func (s *RmServer) getUpTimeDelta() uint32 {
	return uint32(time.Now().Unix()/int64(time.Microsecond) - s.upTimeMils)
}

func assembleStruct(header *headertypes.RemBasicHeader, dataStruct proto.Message) []byte {

	dataBytes, _ := proto.Marshal(dataStruct)
	headerBytes := headerToBytes(header)
	packetBytes := append(headerBytes, dataBytes...)
	return packetBytes
}

func headerToBytes(header *headertypes.RemBasicHeader) []byte {
	buf := bytes.Buffer{}
	binary.Write(&buf, binary.LittleEndian, header)
	return buf.Bytes()
}

func (s *RmServer) handlePingPong(allData []byte, header *headertypes.RemBasicHeader) {
	pingpong := &rem.RespirMeshInfo{}
	err := proto.Unmarshal(allData, pingpong)

	if err != nil {
		fmt.Println("unmarshaling error on PingPong: ", err)
		return
	}

	// fmt.Println("         Type: ", pingpong.GetType())
	// fmt.Println("     TargetId: ", "0x"+strconv.FormatInt(int64(pingpong.GetTargetId()), 16))
	// fmt.Println("     SourceId: ", "0x"+strconv.FormatInt(int64(pingpong.GetSourceId()), 16))

	// pingpong.Type = rem.ProtobufType_PONG
	// pingpong.TargetId = pingpong.GetSourceId()

	// header.ForwardingType = uint8(rem.RemHeaderForwardingType_TO_NODE)
	// header.ProtobufType = uint8(rem.RemHeaderProtobufType_REM_PINGPONG)

	// pingpong.SourceId = ServerSesionUUID

	// respPacket := assembleStruct(header, pingpong)
	// c.SendBytes(respPacket)

}

func (s *RmServer) handleTimeSync(allData []byte, header *headertypes.RemBasicHeader) {

	timesync := &rem.RespirMeshTimeSync{}
	err := proto.Unmarshal(allData, timesync)

	if err != nil {
		fmt.Println("unmarshaling error on TimeSync: ", err)
		return
	}
	timesync.ResponseArriveTime = s.getUpTimeDelta()
	timesync.ServerTimestampUnix = uint32(time.Now().Unix())
	timesync.ResponseSentTime = s.getUpTimeDelta()
	// header.ForwardingType = uint8(rem.RemHeaderForwardingType_TO_NODE)
	// header.ProtobufType = uint8(rem.RemHeaderProtobufType_REM_TIMESYNC)

	// respPacket := assembleStruct(header, timesync)
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

func (s *RmServer) handleMeshTopology(meshTopo *rem.RespirMeshInfo, header *headertypes.RemBasicHeader) {

	fmt.Println("  MESH TOPOLOGY: ", meshTopo.GetType())
	fmt.Println("       TargetId: ", "0x"+strconv.FormatInt(int64(meshTopo.GetTargetId()), 16))
	fmt.Println("       SourceId: ", "0x"+strconv.FormatInt(int64(meshTopo.GetSourceId()), 16))

	targetNode := s.RemTopo.ObtainNode(meshTopo.GetSourceId())
	sourceNode := s.RemTopo.ObtainNode(meshTopo.GetTargetId())
	s.RemTopo.MakeEdgeFromIDs(targetNode.ID, sourceNode.ID)

}

func (s *RmServer) onNewBytes(c *tcp_server.Client, allData []byte) {
	fmt.Print("\n")
	header := headertypes.RemBasicHeader{}
	offsetHeader := unsafe.Sizeof(header)
	buf := bytes.NewReader(allData)
	// err := binary.Read(buf, binary.BigEndian, &header)
	err := binary.Read(buf, binary.LittleEndian, &header)
	if err != nil {
		fmt.Println(err)
		return
	}

	fmt.Println("ForwardingType HeaderType ProtobufType ", rem.ForwardingType(header.ForwardingType), rem.HeaderType(header.HeaderType), rem.ProtobufType(header.ProtobufType))
	// fmt.Println("ForwardingType  ", header.ForwardingType, rem.ForwardingType(header.ForwardingType))
	// fmt.Println("HeaderType      ", header.HeaderType, rem.HeaderType(header.HeaderType))
	// fmt.Println("ProtobufType    ", header.ProtobufType, rem.ProtobufType(header.ProtobufType))
	// fmt.Println("Testing16Number    ", header.Testing16Number)
	// fmt.Println("Testing32Number    ", header.Testing32Number)
	packetData := allData[offsetHeader:]

	switch rem.ForwardingType(header.ForwardingType) {
	case rem.ForwardingType_TO_ROOT:

		switch rem.ProtobufType(header.ProtobufType) {
		case rem.ProtobufType_TIMESYNC:
			s.handleTimeSync(packetData, &header)
		case rem.ProtobufType_PING:
			s.handlePingPong(packetData, &header)
		case rem.ProtobufType_MESH_TOPOLOGY:
			meshTopo := &rem.RespirMeshInfo{}
			err := proto.Unmarshal(packetData, meshTopo)

			if err != nil {
				fmt.Println("unmarshaling error on PingPong: ", err)
				return
			}
			s.handleMeshTopology(meshTopo, &header)
		}

	case rem.ForwardingType_PARENT_TO_ROOT:
		switch rem.ProtobufType(header.ProtobufType) {
		case rem.ProtobufType_MESH_TOPOLOGY:
			meshTopo := &rem.RespirMeshInfo{}
			err := proto.Unmarshal(packetData, meshTopo)
			// fmt.Println("direct TargetId    ", meshTopo.TargetId)
			if meshTopo.GetTargetId() == 0 {
				meshTopo.TargetId = s.ServerSesionUUID
			}

			if err != nil {
				fmt.Println("unmarshaling error on PingPong: ", err)
				return
			}
			s.handleMeshTopology(meshTopo, &header)
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

func (s *RmServer) StartRemServer() {
	go s.tcpServer.Listen()
	fmt.Println("TCP server started", s.tcpServer)
}

func (s *RmServer) UpdateRootNodePeriodically() {
	go func() {
		for {
			s.RemTopo.CreateRoot(s.ServerSesionUUID)
			time.Sleep(s.RootUpdateFreq)
		}
	}()
}

// Create and run (in the background) a RespirMeshServer
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

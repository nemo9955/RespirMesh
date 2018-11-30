package remlogs

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"unsafe"

	"./LogsWriteInfluxDB"

	"../../../protobuf/rem_go_pb"
	"../RemHeaderTypes"
	"../Utils"
	"../tcp_server"

	"github.com/golang/protobuf/proto"
	// "github.com/influxdata/influxdb/client/v2"
)

// RemLogs hold comment
type RemLogs struct {
}

// Handle hold comment
func (l *RemLogs) Handle(allData []byte, c *tcp_server.Client) {
	header := remheadertypes.RemBasicHeader{}
	buf := bytes.NewReader(allData)
	err := binary.Read(buf, binary.LittleEndian, &header)
	if err != nil {
		fmt.Println(err)
		return
	}

	offsetHeader := unsafe.Sizeof(header)
	packetData := allData[offsetHeader:]
	// packetDataSize := unsafe.Sizeof(packetData)

	remLog := &rem.RespirMeshLog{}
	err = proto.Unmarshal(packetData, remLog)
	if err != nil {
		fmt.Println("unmarshaling error on RemLog: ", err)
		return
	}

	utils.PrettyPrint("RespirMeshLog", remLog)
	logswriteinfluxdb.WriteLog(remLog)
}

// New hold comment
func New() *RemLogs {
	remLogs := &RemLogs{}

	logswriteinfluxdb.CreateDatabase()
	influxDbStatus := logswriteinfluxdb.ClientPing()
	if influxDbStatus {
		fmt.Println("!!!!!!!!!!!!!! SUCCESS pinging InfluxDB Cluster: ")
	} else {
		fmt.Println("!!!!!!!!!!!!!! ERROR pinging InfluxDB Cluster: ")
	}

	return remLogs
}

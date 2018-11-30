package remheadertypes

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
)

// RemBasicHeader for all Respirmesh packets
type RemBasicHeader struct {
	PSize000       uint8
	PSize256       uint8
	ForwardingType uint8
	HeaderType     uint8
	ProtobufType   uint8
}

// PacketSize comm
func (header *RemBasicHeader) PacketSize() int {
	return int(header.PSize000) | int(header.PSize256)<<8
}

// GetRemBasicHeader comm
func GetRemBasicHeader(allData []byte) RemBasicHeader {
	header := RemBasicHeader{}
	buf := bytes.NewReader(allData)
	err := binary.Read(buf, binary.LittleEndian, &header)
	if err != nil {
		fmt.Println(err)
		log.Fatal("INVALID GetRemBasicHeader TYPE")
	}
	return header
}

// type RemDataHeaderByte struct {
// 	ForwardingType uint8
// 	HeaderType     uint8
// 	ProtobufType   uint8
// 	Data           uint8
// }

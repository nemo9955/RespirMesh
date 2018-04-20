package headertypes

// Base Header for all Respirmesh packets
type RemBasicHeader struct {
	ForwardingType uint8
	HeaderType     uint8
	ProtobufType   uint8
}

package remtopo

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"math/rand"
	"strconv"
	"sync"
	"time"
	"unsafe"

	"../RemHeaderTypes"

	"github.com/golang/protobuf/proto"

	"../../../protobuf/rem_go_pb"
	"../tcp_server"
)

// Node generic node structure
type Node struct {
	ID          uint32
	UUID        int64
	Neighbors   []*Node `json:"-,"`
	IsRoot      bool
	lastUpdated int64
	isOld       bool
}

// Edge generic edge structure
type Edge struct {
	Source      *Node
	Target      *Node
	lastUpdated int64
	UUID        int64
	isOld       bool
}

// RemTopology topology structure
type RemTopology struct {
	TopoCleanFreq          time.Duration
	maximumOldTimeLimit    time.Duration
	maximumDeleteTimeLimit time.Duration
	// nodesIntMap map[uint32]*Node
	// edgesIntMap map[int]*Edge

	Nodes []*Node
	Edges []*Edge
	WGr   sync.WaitGroup
}

// NodesToUintList hold comment
func NodesToUintList(n1 *Node, nodes ...*Node) []uint32 {
	T := []uint32{}
	T = append(T, n1.ID)
	return append(T, (NodesToUintList(nodes[0], nodes[1:]...))...)
}

// GetWayTo hold comment
func (g *RemTopology) GetWayTo(N1 *Node, N2 *Node) []*Node {
	NodeArray := []*Node{}
	NodeArray = append(NodeArray, N1)
	if N1 == N2 {
		return NodeArray
	}
	for _, n := range N1.Neighbors {
		if N2 == n {
			NodeArray = append(NodeArray, N2)
			return NodeArray
		}
		TArray := g.GetWayTo(n, N2)
		if TArray != nil {
			NodeArray = append(NodeArray, TArray...)
			return NodeArray
		}
	}
	return nil
}

// GetNodeFromUUID get node from UUID, nil if none
func (g *RemTopology) GetNodeFromUUID(nodeUUID int64) *Node {
	var node *Node
	for _, n := range g.Nodes {
		if n.UUID == nodeUUID {
			node = n
		}
	}
	return node
}

// GetEdgeFromUUID get edge from UUID, nil if none
func (g *RemTopology) GetEdgeFromUUID(edgeUUID int64) *Edge {
	var edge *Edge
	for _, e := range g.Edges {
		if e.UUID == edgeUUID {
			edge = e
		}
	}
	return edge
}

// GetNodeFromID get node from ID, nil if none
func (g *RemTopology) GetNodeFromID(nodeID uint32) *Node {
	var node *Node
	for _, n := range g.Nodes {
		if n.ID == nodeID {
			node = n
		}
	}
	return node
}

// GetEdgeFromID get edge from it'n nodes IDs, nil if none
func (g *RemTopology) GetEdgeFromID(source, target uint32) *Edge {
	var edge *Edge
	for _, e := range g.Edges {
		if e.Source.ID == source && e.Target.ID == target {
			edge = e
		}
	}
	return edge
}

// ObtainEdge create and/or update edge based on node IDs
func (g *RemTopology) ObtainEdge(source, target uint32) *Edge {
	edge := g.GetEdgeFromID(source, target)

	g.WGr.Wait()

	if edge == nil {
		tryUUID := rand.Int63()
		for g.GetEdgeFromUUID(tryUUID) != nil {
			tryUUID = rand.Int63()
		}

		edge = &Edge{
			Source:      g.ObtainNode(source),
			Target:      g.ObtainNode(target),
			lastUpdated: time.Now().Unix(),
			UUID:        tryUUID,
			isOld:       false,
		}

		// edge.lastUpdated = time.Now().Unix()
		// edge.Source.lastUpdated = edge.lastUpdated
		// edge.Target.lastUpdated = edge.lastUpdated

		g.WGr.Add(1)
		g.Edges = append(g.Edges, edge)
		g.WGr.Done()
	} else {
		edge.lastUpdated = time.Now().Unix()
		edge.isOld = false
		edge.Source.lastUpdated = edge.lastUpdated
		edge.Source.isOld = false
		edge.Target.lastUpdated = edge.lastUpdated
		edge.Target.isOld = false
	}

	return edge
}

// ObtainNode create and/or update node in the mesh
func (g *RemTopology) ObtainNode(nodeID uint32) *Node {
	node := g.GetNodeFromID(nodeID)

	g.WGr.Wait()
	if node == nil {
		tryUUID := rand.Int63()
		for g.GetNodeFromUUID(tryUUID) != nil {
			tryUUID = rand.Int63()
		}

		node = &Node{
			ID:          nodeID,
			UUID:        tryUUID,
			Neighbors:   []*Node{},
			lastUpdated: time.Now().Unix(),
			IsRoot:      false,
			isOld:       false,
		}
		g.WGr.Add(1)
		g.Nodes = append(g.Nodes, node)
		g.WGr.Done()
	} else {
		node.lastUpdated = time.Now().Unix()
		node.isOld = false
	}

	return node
}

// CreateRoot shortcut for creating node and marking it as root
func (g *RemTopology) CreateRoot(rootID uint32) *Node {
	node := g.ObtainNode(rootID)

	if node == nil {
		panic("Created root does not exist " + strconv.FormatInt(int64(rootID), 16))
	}

	node.IsRoot = true
	return node

}

// StrID returns the string representation of the ID (usually in base16)
func (n *Node) StrID() string {
	return strconv.FormatInt(int64(n.ID), 16)
}

// StrUUID returns the string representation of the UUID (usually in base16)
func (n *Node) StrUUID() string {
	return strconv.FormatInt(int64(n.UUID), 16)
}

func (n *Node) addNeighbor(node *Node) {
	alreadyThere := false
	for _, n := range n.Neighbors {
		if n.UUID == node.UUID {
			alreadyThere = true
		}
	}
	if alreadyThere == false {
		n.Neighbors = append(n.Neighbors, node)
	}
}

// MakeEdgeFromIDs creates or updates an adge and it's nodes based on ID
func (g *RemTopology) MakeEdgeFromIDs(source, target uint32) *Edge {
	targetNode := g.ObtainNode(target)
	sourceNode := g.ObtainNode(source)

	edge := g.ObtainEdge(source, target)

	if edge.Source.ID != source || edge.Target.ID != target {
		panic("Edge's nodes do not match!")
	}

	edge.lastUpdated = time.Now().Unix()

	targetNode.addNeighbor(sourceNode)
	sourceNode.addNeighbor(targetNode)
	targetNode.lastUpdated = edge.lastUpdated
	sourceNode.lastUpdated = edge.lastUpdated

	return edge
}

// AssembleJSON assemble a json object for the web viewer
func (g *RemTopology) AssembleJSON() []byte {
	type _Node struct {
		ID     string `json:"id"`
		UUID   int64  `json:"group"`
		IsRoot bool   `json:"IsRoot"`
		IsOld  bool   `json:"IsOldNode"`
	}

	type _Edge struct {
		Source   string  `json:"source"`
		Target   string  `json:"target"`
		IsOld    bool    `json:"IsOldEdge"`
		Strength float32 `json:"strength"`
	}

	type _RemTopology struct {
		Nodes []_Node `json:"nodes"`
		Edges []_Edge `json:"links"`
	}

	rtop := _RemTopology{
		Nodes: []_Node{},
		Edges: []_Edge{},
	}

	g.WGr.Wait()
	g.WGr.Add(1)

	for _, node := range g.Nodes {
		mNode := _Node{
			ID:     node.StrID(),
			UUID:   node.UUID,
			IsRoot: node.IsRoot,
			IsOld:  node.isOld,
		}
		rtop.Nodes = append(rtop.Nodes, mNode)
	}

	for _, edge := range g.Edges {
		mEdge := _Edge{
			Source:   edge.Source.StrID(),
			Target:   edge.Target.StrID(),
			IsOld:    edge.isOld,
			Strength: 1,
			// Strength: rand.Float32(),
		}
		rtop.Edges = append(rtop.Edges, mEdge)
	}
	g.WGr.Done()

	jsonStr, _ := json.Marshal(&rtop)
	return jsonStr
}

func (g *RemTopology) updateLifeDuration() {
	tn := time.Now().Unix()

	g.WGr.Wait()
	g.WGr.Add(1)

	for i, edge := range g.Edges {
		if edge == nil {
			continue
		}
		switch {
		case time.Duration(tn-edge.lastUpdated)*time.Second > g.maximumDeleteTimeLimit:
			g.Edges[i] = g.Edges[len(g.Edges)-1]
			g.Edges[len(g.Edges)-1] = nil
			g.Edges = g.Edges[:len(g.Edges)-1]
		case time.Duration(tn-edge.lastUpdated)*time.Second > g.maximumOldTimeLimit:
			edge.isOld = true
		default:
			edge.isOld = false

		}
	}

	for i, node := range g.Nodes {
		if node == nil {
			continue
		}
		switch {
		case time.Duration(tn-node.lastUpdated)*time.Second > g.maximumDeleteTimeLimit:
			g.Nodes[i] = g.Nodes[len(g.Nodes)-1]
			g.Nodes[len(g.Nodes)-1] = nil
			g.Nodes = g.Nodes[:len(g.Nodes)-1]
		case time.Duration(tn-node.lastUpdated)*time.Second > g.maximumOldTimeLimit:
			node.isOld = true
		default:
			node.isOld = false

		}
	}

	g.WGr.Done()
}

// GetRoots returns all roots or isolated nodes
func (g *RemTopology) GetRoots() []*Node {
	roots := make(map[int64]bool)
	rootsArray := []*Node{}

	for _, edge := range g.Edges {
		roots[edge.Target.UUID] = true
	}

	for _, edge := range g.Edges {
		roots[edge.Source.UUID] = false
	}

	for _, node := range g.Nodes {
		if len(node.Neighbors) == 0 {
			roots[node.UUID] = true
		}
	}

	for rootUUID, rVal := range roots {
		// fmt.Println("r ", rootUUID, rVal)
		if rVal {
			rootNode := g.GetNodeFromUUID(rootUUID)
			if rootNode != nil {
				rootsArray = append(rootsArray, rootNode)
			}
		}
	}
	// fmt.Println("ruts : ", rootsArray)
	return rootsArray
}

// UpdateTopoPeriodically hold comment
func (g *RemTopology) UpdateTopoPeriodically() {
	go func() {
		for {
			g.updateLifeDuration()
			time.Sleep(g.TopoCleanFreq)
		}
	}()
}

// New hold comment
func New() *RemTopology {
	remTopo := &RemTopology{
		TopoCleanFreq:          1 * time.Second,
		maximumOldTimeLimit:    6 * time.Second,
		maximumDeleteTimeLimit: 11 * time.Second,
		Nodes: []*Node{},
		Edges: []*Edge{},
	}

	remTopo.UpdateTopoPeriodically()

	return remTopo
}

// Handle hold comment
func (g *RemTopology) Handle(allData []byte, serverSesionUUID uint32, c *tcp_server.Client) {
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

	meshTopo := &rem.RespirMeshInfo{}
	err = proto.Unmarshal(packetData, meshTopo)
	if err != nil {
		fmt.Println("unmarshaling error on Mesh Topology: ", err)
		return
	}
	// fmt.Println("direct TargetId    ", meshTopo.TargetId)
	if header.ForwardingType == uint8(rem.ForwardingType_TO_PARENT_TO_ROOT) && meshTopo.GetTargetId() == 0 {
		meshTopo.TargetId = serverSesionUUID
	}

	fmt.Println("  MESH TOPOLOGY: ", meshTopo.GetType())
	fmt.Println("       TargetId: ", "0x"+strconv.FormatInt(int64(meshTopo.GetTargetId()), 16))
	fmt.Println("       SourceId: ", "0x"+strconv.FormatInt(int64(meshTopo.GetSourceId()), 16))
	fmt.Println("       TargetId: ", strconv.FormatInt(int64(meshTopo.GetTargetId()), 10))
	fmt.Println("       SourceId: ", strconv.FormatInt(int64(meshTopo.GetSourceId()), 10))

	targetNode := g.ObtainNode(meshTopo.GetSourceId())
	sourceNode := g.ObtainNode(meshTopo.GetTargetId())
	g.MakeEdgeFromIDs(targetNode.ID, sourceNode.ID)

}

package remtopo

import (
	"encoding/json"
	"math/rand"
	"strconv"
	"time"
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

		g.Edges = append(g.Edges, edge)
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
		g.Nodes = append(g.Nodes, node)
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
		Source string `json:"source"`
		Target string `json:"target"`
		IsOld  bool   `json:"IsOldEdge"`
	}

	type _RemTopology struct {
		Nodes []_Node `json:"nodes"`
		Edges []_Edge `json:"links"`
	}

	rtop := _RemTopology{
		Nodes: []_Node{},
		Edges: []_Edge{},
	}

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
			Source: edge.Source.StrID(),
			Target: edge.Target.StrID(),
			IsOld:  edge.isOld,
		}
		rtop.Edges = append(rtop.Edges, mEdge)
	}

	jsonStr, _ := json.Marshal(&rtop)
	return jsonStr
}

func (g *RemTopology) updateLifeDuration() {
	tn := time.Now().Unix()

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

func (g *RemTopology) UpdateTopoPeriodically() {
	go func() {
		for {
			g.updateLifeDuration()
			time.Sleep(g.TopoCleanFreq)
		}
	}()
}

func New() *RemTopology {
	remTopo := &RemTopology{
		TopoCleanFreq:          1 * time.Second,
		maximumOldTimeLimit:    4 * time.Second,
		maximumDeleteTimeLimit: 5 * time.Second,
		Nodes: []*Node{},
		Edges: []*Edge{},
	}

	remTopo.UpdateTopoPeriodically()

	return remTopo
}

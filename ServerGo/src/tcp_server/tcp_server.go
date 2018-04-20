package tcp_server

import (
	"bufio"
	"log"
	"net"
	"time"
)

// Client holds info about connection
type Client struct {
	conn   net.Conn
	server *TcpServer
}

// TCP TcpServer
type TcpServer struct {
	address                  string // Address to open connection: localhost:9999
	timeoutDuration          time.Duration
	onNewClientCallback      func(c *Client)
	onClientConnectionClosed func(c *Client, err error)
	onNewBytes               func(c *Client, bytes []byte)
}

// Get Connection
func (c *Client) GetConn() net.Conn {
	return c.conn
}

// Read client data from channel
func (c *Client) listen() {
	reader := bufio.NewReader(c.conn)
	for {
		// message, err := reader.ReadString('\n')
		c.conn.SetReadDeadline(time.Now().Add(c.server.timeoutDuration))

		buf := make([]byte, 512)
		n, err := reader.Read(buf)
		bytes := buf[:n]

		if err != nil {
			delete(connectedClients, c.conn.RemoteAddr().String())
			c.conn.Close()
			go c.server.onClientConnectionClosed(c, err)
			return
		}
		go c.server.onNewBytes(c, bytes)
	}
}

// Send text message to client
func (c *Client) Send(message string) error {
	_, err := c.conn.Write([]byte(message))
	return err
}

// Send bytes to client
func (c *Client) SendBytes(b []byte) error {
	_, err := c.conn.Write(b)
	return err
}

func (c *Client) Conn() net.Conn {
	return c.conn
}

func (c *Client) Close() error {
	return c.conn.Close()
}

// Called right after TcpServer starts listening new client
func (s *TcpServer) OnNewClient(callback func(c *Client)) {
	s.onNewClientCallback = callback
}

// Called right after connection closed
func (s *TcpServer) OnClientConnectionClosed(callback func(c *Client, err error)) {
	s.onClientConnectionClosed = callback
}

// Called when Client receives new Byte array
func (s *TcpServer) OnNewBytes(callback func(c *Client, bytes []byte)) {
	s.onNewBytes = callback
}

var connectedClients = make(map[string]*Client)

// Start network TcpServer
func (s *TcpServer) Listen() {
	listener, err := net.Listen("tcp", s.address)
	if err != nil {
		log.Fatal("Error starting TCP TcpServer.")
	}
	defer listener.Close()

	for {
		conn, _ := listener.Accept()
		client := &Client{
			conn:   conn,
			server: s,
		}
		go client.listen()
		// s.onNewBytes(client, bytes)
		go s.onNewClientCallback(client)
		connectedClients[client.conn.RemoteAddr().String()] = client

	}
}

// Creates new tcp TcpServer instance
func New(address string, timeoutDuration time.Duration) *TcpServer {
	log.Println("Creating TcpServer with address", address)

	tcpServer := &TcpServer{
		address:         address,
		timeoutDuration: timeoutDuration,
	}

	tcpServer.OnNewClient(func(c *Client) {})
	tcpServer.OnNewBytes(func(c *Client, bytes []byte) {})
	tcpServer.OnClientConnectionClosed(func(c *Client, err error) {})

	log.Println("DONE creating TcpServer with address", tcpServer)
	return tcpServer
}

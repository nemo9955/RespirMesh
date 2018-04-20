package main

import (
	"fmt"
	"html/template"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"./RespirMeshServer"
)

var server *respirmeshserver.RmServer = nil

// var server = respirmeshserver.New("0.0.0.0:9995", 60*time.Second)

func main() {
	var quit = make(chan bool)

	server = respirmeshserver.New("0.0.0.0:9995", 60*time.Second)
	log.Println("Respir Mesh server created ", server)
	server.StartRemServer()
	log.Println("Respir Mesh server started ", server)

	cwd, _ := os.Getwd()
	fmt.Println("In PWD, looking for ", filepath.Join(cwd, "../res/web/tmpl/"))
	http.HandleFunc("/", hh_root)
	// http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("/home/Casper/public/client/index/"))))
	go http.ListenAndServe(":9996", nil)

	fmt.Printf("%s", (server.RemTopo.AssembleJSON()))

	<-quit
}

func hh_root(w http.ResponseWriter, r *http.Request) {
	cwd, _ := os.Getwd()
	t, _ := template.ParseFiles(filepath.Join(cwd, "../res/web/tmpl/index.html")) // Parse template file.
	// t.Execute(w, server.RemTopo.AssembleJSON())                                    // merge.
	t.Execute(w, struct {
		Json string
	}{
		Json: fmt.Sprintf("%s", (server.RemTopo.AssembleJSON())),
	}) // merge.

}

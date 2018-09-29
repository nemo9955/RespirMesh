mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))



watch:
	$(info )
	$(info Requires https://github.com/canthefason/go-watcher)
	$(info go get github.com/gorilla/websocket)
	$(info go get github.com/canthefason/go-watcher)
	$(info go install github.com/canthefason/go-watcher/cmd/watcher)
	$(info )
	cd  $(dir  $(mkfile_path) ) ;
	watcher init.go;

run:
	cd  $(dir  $(mkfile_path) ) ;
	go run init.go


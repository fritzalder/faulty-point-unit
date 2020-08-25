DOCKER_NAME = sgx-fpu

all: build run

build:
	docker build -t $(DOCKER_NAME) --no-cache .

run:
	docker run -i -h "badf1oa7" -t $(DOCKER_NAME) 

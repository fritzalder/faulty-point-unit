IMAGE_NAME = sgx-fpu
REPOSITORY = docker.pkg.github.com/fritzalder/faulty-point-unit

all: build run

build:
	docker build -t $(IMAGE_NAME) --no-cache .

run:
	docker run -i -h "badf1oa7" -t $(REPOSITORY)/$(IMAGE_NAME)

pull:
	docker pull $(REPOSITORY)/$(IMAGE_NAME):latest
	docker run -i -h "badf1oa7" -t $(REPOSITORY)/$(IMAGE_NAME)
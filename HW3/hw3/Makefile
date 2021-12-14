all: project

.PHONY: project test docker docker-connect clean

DOCKER_PATH = docker/
SOURCE_PATH = src/
TEST_PATH = test/

project: 
	make -C ${SOURCE_PATH}

test:
	make test -C test/

docker:
	docker-compose -f ${DOCKER_PATH}docker-compose.yml up -d --build
	docker exec -it intro-network-hw3 sh -c "cd /project && /bin/bash"

clean:
	make clean -C ${SOURCE_PATH}
	docker-compose -f ${DOCKER_PATH}docker-compose.yml down
HOST ?= robot@ev3dev.local

BUILD_DIR ?= builddir
SRC_DIR ?= src
DOCKER_TAG ?= ev3dev_cross

BINARY_NAME ?= cppcub3r
BINARY_PATH_BUILD := ${BUILD_DIR}/${SRC_DIR}/${BINARY_NAME}
BINARY_PATH_HOST := ./${BINARY_NAME}

BUILD_DIR_TRACKER := ${BUILD_DIR}/.build_dir_docker
DOCKER_IMAGE_TRACKER := ${BUILD_DIR}/.docker_image_tracker_${DOCKER_TAG}
UPLOAD_TRACKER := ${BUILD_DIR}/.upload_tracker_${HOST}

.PHONY: all build upload docker_image run clean

all: build

${BUILD_DIR_TRACKER}:
	mkdir -p ${BUILD_DIR}
	touch ${BUILD_DIR_TRACKER}

build: ${BINARY_PATH_BUILD}
upload: ${UPLOAD_TRACKER}
docker_image: ${DOCKER_IMAGE_TRACKER}

run: upload
	clear -x
	ssh ${HOST} -t ${BINARY_PATH_HOST}

clean:
	rm -Rf ${BUILD_DIR}

${BINARY_PATH_BUILD}: ${BUILD_DIR_TRACKER} ${DOCKER_IMAGE_TRACKER} ${SRC_DIR}/* CMakeLists.txt
	@echo "Starting Docker Container..."
	docker run --rm -it -v `pwd`:/home/compiler/src ${DOCKER_TAG}

${UPLOAD_TRACKER}: ${BINARY_PATH_BUILD}
	scp ${BINARY_PATH_BUILD} ${HOST}:${BINARY_PATH_HOST}
	touch ${UPLOAD_TRACKER}

${DOCKER_IMAGE_TRACKER}: ${BUILD_DIR_TRACKER} docker/*
	docker build docker -t ${DOCKER_TAG}
	touch ${DOCKER_IMAGE_TRACKER}


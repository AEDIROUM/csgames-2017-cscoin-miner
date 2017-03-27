#!/bin/sh

export DOCKER_HOST="tcp://miners.2017.csgames.org:2376"
export DOCKER_TLS_VERIFY=1
export DOCKER_CERT_PATH="$(pwd)/cscert/"

docker --config . build .
docker --config . create cscoin-miner
docker --config . start cscoin-miner

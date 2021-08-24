#!/bin/bash

# sends the application firmware to the testbed
#
# usage:
#   send_fw.sh <remote_fw_name> [testbed_site]

remote_fw=${1}
site=${2:-grenoble}
user=lanzieri
experiment=lwm2mc2c
remote_path=/senslab/users/$user/shared/exp/$experiment/fw

if [ -z $remote_fw ]
then 
    echo "You must specify the remote FW name"
    exit 1
fi

scp bin/iotlab-m3/wakaama.elf $user@$site.iot-lab.info:$remote_path/$remote_fw.elf

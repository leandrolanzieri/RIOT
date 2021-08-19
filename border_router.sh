#!/bin/bash

# initiate a border router
#
# usage border_router.sh <channel> <node> [site:grenoble]
#

# sudo ethos_uhcpd.py m3-322 tap0 2001:660:5307:3101::/64

# get board translation
. ./iotlab_boards.sh
. ./iotlab_name_conversion.sh


print_usage() {
    echo "Usage: ${0} <channel> <node> [site:grenoble]"
}

channel=${1}
node=${2}
site=${3:-grenoble}

_board=${2%%,*}
board=${iotlab_to_riot[${_board}]}

app_dir=examples/gnrc_border_router
prefix=2001:660:5307:3101::/64
tap=tap0
user=lanzieri
tmux_win=border_router

if [ -z $channel ]
then 
    echo "You must specify the channel"
    print_usage
    exit 1
fi

if [ -z $board ]
then 
    echo "You must specify the board"
    print_usage
    exit 1
fi

if [ -z $node ]
then 
    echo "You must specify the node"
    print_usage
    exit 1
fi

env BOARD=$board make clean -C $app_dir
res=env ETHOS_BAUDRATE=500000 BOARD=$board DEFAULT_CHANNEL=$channel make -C $app_dir all -j

if [ $res ]
then
    echo "Error building the application $app_dir"
    exit 1
fi

echo "flashing $node"
iotlab node --flash $app_dir/bin/$board/*.elf -l $site,$node
tmux new-window -d -n $tmux_win "ssh $user@$site.iot-lab.info"
sleep 5
echo "initiating border router in channel $channel"
tmux send-keys -t $tmux_win "sudo ethos_uhcpd.py ${node/,/-} $tap $prefix" Enter

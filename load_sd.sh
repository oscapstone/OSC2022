IMG=$1 # kernel8.img
SD_DEVICE=$2 #/dev/sdc1
echo "loading [$IMG] to device [$SD_DEVICE]..."
sudo mount -t vfat $SD_DEVICE sd-card-mount
sudo rm -rf sd-card-mount/$1 
sudo cp $1 sd-card-mount/$1
#sudo umount sd-card-mount
#echo "done"
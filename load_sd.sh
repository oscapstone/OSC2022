sudo mount -t vfat /dev/sdc1 sd-card-mount
sudo rm -rf sd-card-mount/kernel8.img 
sudo cp Lab1/kernel8.img sd-card-mount/kernel8.img
sudo umount sd-card-mount
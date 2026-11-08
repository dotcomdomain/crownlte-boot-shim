# crownlte boot shim

uniLoader-based loader that boots the Linux 6.12 Image, DTB and initramfs from the
Galaxy Note9 RECOVERY partition. Based on uniLoader (GPL-2.0).

## Build

```sh
sudo apt install gcc-aarch64-linux-gnu make
mkdir -p blob out
cp Image blob/Image; cp <name>.dtb blob/dtb; cp initramfs.cpio.gz blob/ramdisk
cp configs/crownlte-romram.config out/.config
make O=out ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- \
     KERNEL_PATH=$PWD/blob/Image DT_PATH=$PWD/blob/dtb RAMDISK_PATH=$PWD/blob/ramdisk -j1
```

Output: `out/uniLoader`. `pack-rom-ram.py` in crownlte-rom-ram packs it into a RECOVERY
image.

This repository contains an example of external tree for buildroot. Configuration of the tree is based on *qemu_arm_vexpress_defconfig*. Contents:

+ defconfig for buildroot is placed in *config* ;
+ *package* directory contains three packages:
 + beep ([http://www.johnath.com/beep/](http://www.johnath.com/beep/));
 + smem ([https://www.selenic.com/smem/](https://www.selenic.com/smem/));
 + an example of characted device driver implementing simple storage;
+ *linux-patches* directory contains a patch for device tree which is required for storage driver;
+ *rootfs_overlay* contains configuration file for *mdev*.

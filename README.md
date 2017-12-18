# siar_packages

This metapackage is composed by the most basic developments that enables the SIAR platform to be teleoperated with a Logitec Gamepad. The current version is maintained in Kinetic ROS.

This metapackage is composed by the following packages. For more details about them, please refere to their included README.md files:

* *frame_dropper* Contains two utilities: "frame_dropper" and "image_splitter" for replicating image flow that could be used for transmitting images over the network.
* *siar_driver* A driver for controlling the SIAR platform
* *siar_messages* Compiles the basic messages used by the SIAR platform
* *udp_bridge* A bridge module to transmit data over a UDP link

## Compilation

It is recommended to install it from "siar_launch" git repository (TODO: include link)

However, you can install only this repository to make the SIAR platform basic functionalities ready to go.
 
 
 > roscd 
 
 > cd ../src
 
 > git clone https://github.com/robotics-upo/siar_packages.git
 
 > roscd
 
 > cd ..
 
 > catkin_make

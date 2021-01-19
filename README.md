
*WORK IN PROGRESS*

Sunny Island linux-based command line utility

requires MYBMM for the modules

currently only supports CAN bus


For CAN:

sitool -t can:<device>[,speed]

example:

	sitool -t can:can0,500000


for CANServer/Can-over-ip

sitool -t can_ip:<ip addr>,[port],<interface>,[speed]

example:

	sitool -t can_ip:10.0.0.1,3930,can0,500000


To start the Sunny Island remotey:

sitool -t can:<device>[,speed] -s

example:

	sitool -t can:can0,500000 -s


To stop the Sunny Island remotey:

sitool -t can:<device>[,speed] -S

example:

	sitool -t can:can0,500000 -S

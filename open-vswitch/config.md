# Open vSwitch



## Flow

```shell
# C
ovs-ofctl add-flow br0 "priority=100,in_port=2,dl_type=0x0800,nw_proto=6,nw_dst=192.168.3.32,actions=set_field:192.168.3.19->nw_dst,4" 
ovs-ofctl add-flow br0 "priority=100,in_port=4,dl_type=0x0800,nw_proto=6,nw_src=192.168.3.19,actions=set_field:192.168.3.32->nw_src,2"
ovs-ofctl add-flow br0 "priority=100,in_port=2,dl_type=0x0800,nw_proto=6,nw_dst=192.168.3.31,actions=set_field:192.168.3.18->nw_dst,4" 
ovs-ofctl add-flow br0 "priority=100,in_port=4,dl_type=0x0800,nw_proto=6,nw_src=192.168.3.18,actions=set_field:192.168.3.31->nw_src,2"
ovs-ofctl add-flow br0 "priority=100,in_port=2,dl_type=0x0800,nw_proto=6,nw_dst=192.168.4731,actions=set_field:192.168.3.20->nw_dst,4" 
ovs-ofctl add-flow br0 "priority=100,in_port=2,dl_type=0x0800,nw_proto=6,nw_dst=192.168.3.47,actions=set_field:192.168.3.20->nw_dst,4" 
ovs-ofctl add-flow br0 "priority=100,in_port=4,dl_type=0x0800,nw_proto=6,nw_src=192.168.3.20,actions=set_field:192.168.3.47->nw_src,2"
ovs-ofctl add-flow br0 "priority=100,in_port=4,dl_type=0x0800,nw_proto=6,nw_src=192.168.3.19,actions=set_field:192.168.3.35->nw_src,2"
 ovs-ofctl add-flow br0 "priority=100,in_port=2,dl_type=0x0800,nw_proto=6,nw_dst=192.168.3.35,actions=set_field:192.168.3.19->nw_dst,4"


# R
ovs-ofctl dump-flows br0

# D
ovs-ofctl del-flows br0 "flow_id=10"
ovs-ofctl del-flows br0 "flow_id=11"
ovs-ofctl del-flows br0 "flow_id=12"
ovs-ofctl del-flows br0 "flow_id=13"
ovs-ofctl del-flows br0 "flow_id=14"
ovs-ofctl del-flows br0 "flow_id=15"
```



## campture 封包

```shell
sudo tcpdump -e -i br0 -n -vv -l tcp port 4002
sudo tcpdump -e -i eth0 -n -vv -l tcp port 4002
sudo tcpdump -e -i eth0 -n -vv -l 
sudo tcpdump -e -i eth0 -n -vv -l | grep 192.168.3.35
```



## group

```shell
# C
ovs-ofctl -O OpenFlow14 add-group br0 "group_id=1, type=select, bucket, actions=set_field:192.168.3.6->nw_dst,4, bucket, actions=set_field:192.168.3.31->nw_dst,4"
ovs-ofctl -O OpenFlow14 add-group br0 "group_id=2, type=select, bucket, actions=set_field:192.168.3.28->nw_dst,4, bucket, actions=set_field:192.168.3.32->nw_dst,4"
ovs-ofctl -O OpenFlow14 add-group br0 "group_id=3, type=select, bucket, actions=set_field:192.168.3.29->nw_dst,4, bucket, actions=set_field:192.168.3.33->nw_dst,4"
ovs-ofctl -O OpenFlow14 add-group br0 "group_id=4, type=select, bucket, actions=set_field:192.168.3.30->nw_dst,4, bucket, actions=set_field:192.168.3.34->nw_dst,4"
ovs-ofctl -O OpenFlow14 add-group br0 "group_id=1, type=select, bucket, actions=set_field:192.168.3.6->nw_dst,4"


# R
ovs-ofctl dump-groups br0

# D
ovs-ofctl del-groups -O OpenFlow14 br0 "group_id=2"
```


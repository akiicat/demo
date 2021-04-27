# Network

## clear arp cache

```shell
sudo ip -s -s neigh flush all
arp -n
```

## clear DNS cache

```shell
sudo service nscd restart
```

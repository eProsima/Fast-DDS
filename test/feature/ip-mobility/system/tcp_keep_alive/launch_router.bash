# Copyright (C) 2026, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

#!/bin/bash
set -e

# Note: This script is intended to be used in a privileged container, since it requires to bring down and up the eth0 interface.

LAN_IFACE=${LAN_IFACE:-$(ip -o -4 addr show | awk '/192\.168\./ {print $2; exit}')}
WAN_IFACE=${WAN_IFACE:-$(ip -o -4 addr show | awk '/10\.10\.16\./ {print $2; exit}')}

CLIENT_IP=${CLIENT_IP:-""}
TCP_PORTS=${TCP_PORTS:-""}

PEER_LAN_CIDR=${PEER_LAN_CIDR:-""}
PEER_WAN_IP=${PEER_WAN_IP:-""}

echo "[router] LAN_IFACE=$LAN_IFACE, WAN_IFACE=$WAN_IFACE"
echo "[router] CLIENT_IP=$CLIENT_IP"
echo "[router] TCP_PORTS=$TCP_PORTS"
echo "[router] PEER_LAN_CIDR=$PEER_LAN_CIDR via PEER_WAN_IP=$PEER_WAN_IP"

# 1) Enable forwarding
if ! echo 1 > /proc/sys/net/ipv4/ip_forward 2>/dev/null; then
  echo "[router] Warning: cannot write ip_forward. Ensure sysctls net.ipv4.ip_forward=1"
fi

# 2) Basic filter: allow forwarding between LAN <-> WAN
iptables -F FORWARD || true
iptables -A FORWARD -i "$LAN_IFACE" -o "$WAN_IFACE" -j ACCEPT
iptables -A FORWARD -i "$WAN_IFACE" -o "$LAN_IFACE" -j ACCEPT

# 3) NAT for “LAN to WAN” (keep your original behavior)
iptables -t nat -F
iptables -t nat -A POSTROUTING -o "$WAN_IFACE" -j MASQUERADE
echo "[router] MASQUERADE enabled on $WAN_IFACE"

# 4) Port forwarding WAN -> CLIENT_IP
if [ -n "$CLIENT_IP" ]; then
  for port in $TCP_PORTS; do
    iptables -t nat -A PREROUTING -i "$WAN_IFACE" -p tcp --dport "$port" \
      -j DNAT --to-destination "${CLIENT_IP}:${port}"
    echo "[router] DNAT TCP $port -> ${CLIENT_IP}:${port}"
  done
fi

# 5) NEW: Static route to the other robot LAN via the peer router on backbone
if [ -n "$PEER_LAN_CIDR" ] && [ -n "$PEER_WAN_IP" ]; then
  ip route replace "$PEER_LAN_CIDR" via "$PEER_WAN_IP" dev "$WAN_IFACE"
  echo "[router] Route added: $PEER_LAN_CIDR via $PEER_WAN_IP dev $WAN_IFACE"
fi

# Container will be closed by docker-compose after the test ends when pub or sub ends
echo "[router] Network config applied. Keeping container alive..."
tail -f /dev/null

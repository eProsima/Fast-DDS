#!/usr/bin/env bash
set -euo pipefail

# change_lan_ip.sh — change robot's local LAN IP (same subnet) without changing mask
# Example: toggle between 192.168.20.13/24 and 192.168.20.33/24

LAN_PREFIX="${LAN_PREFIX:-192.168.20.}"   # robot LAN prefix
LAN_IFACE="${LAN_IFACE:-$(ip -o -4 addr show | awk -v p="$LAN_PREFIX" '$4 ~ "^"p {print $2; exit}')}"
MASK="${MASK:-24}"                       # /24 == 255.255.255.0
GW="${GW:-192.168.20.1}"                  # router LAN IP (robot default gateway)

IP_A="${IP_A:-192.168.20.13}"
IP_B="${IP_B:-192.168.20.33}"

MODE="${1:-toggle}"   # toggle | toA | toB | status

if [[ -z "${LAN_IFACE:-}" ]]; then
  echo "ERROR: Could not auto-detect LAN iface (prefix $LAN_PREFIX)."
  ip -br addr
  exit 1
fi

have_ip() {
  ip -4 addr show dev "$LAN_IFACE" | awk '{print $2}' | grep -q "^$1/$MASK$"
}

current_ip() {
  ip -4 -o addr show dev "$LAN_IFACE" scope global | awk '{print $4}' | head -n1 || true
}

set_ip() {
  local newip="$1"

  ip link set "$LAN_IFACE" up

  # Remove both candidates if present (idempotent)
  ip addr del "$IP_A/$MASK" dev "$LAN_IFACE" 2>/dev/null || true
  ip addr del "$IP_B/$MASK" dev "$LAN_IFACE" 2>/dev/null || true

  # Add the new address
  ip addr add "$newip/$MASK" dev "$LAN_IFACE"

  # Ensure default route goes via router on the LAN
  ip route replace default via "$GW" dev "$LAN_IFACE" 2>/dev/null || true

  # Best-effort ARP announcement (arping might not exist in the image)
  arping -c 2 -A -I "$LAN_IFACE" "$newip" >/dev/null 2>&1 || true
}

case "$MODE" in
  status)
    echo "LAN_IFACE=$LAN_IFACE"
    echo "Current LAN IP: $(current_ip)"
    echo "Gateway (GW): $GW"
    ;;
  toA)
    echo "Switching LAN IP -> $IP_A/$MASK on $LAN_IFACE"
    set_ip "$IP_A"
    ;;
  toB)
    echo "Switching LAN IP -> $IP_B/$MASK on $LAN_IFACE"
    set_ip "$IP_B"
    ;;
  toggle)
    if have_ip "$IP_A"; then
      echo "Toggling LAN IP: $IP_A -> $IP_B"
      set_ip "$IP_B"
    elif have_ip "$IP_B"; then
      echo "Toggling LAN IP: $IP_B -> $IP_A"
      set_ip "$IP_A"
    else
      echo "Neither $IP_A/$MASK nor $IP_B/$MASK found; setting $IP_A/$MASK"
      set_ip "$IP_A"
    fi
    ;;
  *)
    echo "Usage: $0 {toggle|toA|toB|status}"
    echo "Env: LAN_PREFIX LAN_IFACE MASK GW IP_A IP_B"
    exit 2
    ;;
esac

echo "Now:"
ip -br addr show dev "$LAN_IFACE"
ip route | sed -n '1,25p'

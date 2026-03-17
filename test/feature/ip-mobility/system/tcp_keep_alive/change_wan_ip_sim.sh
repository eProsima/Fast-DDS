#!/usr/bin/env bash
set -euo pipefail

# change_wan_ip_sim.sh — change router "public" (WAN/backbone) IP without bringing link down

BACKBONE_PREFIX="${BACKBONE_PREFIX:-172.28.16.}"   # backbone net prefix
WAN_IFACE="${WAN_IFACE:-$(ip -o -4 addr show | awk -v p="$BACKBONE_PREFIX" '$4 ~ "^"p {print $2; exit}')}"
MASK="${MASK:-24}"

IP_A="${IP_A:-172.28.16.100}"
IP_B="${IP_B:-172.28.16.111}"

MODE="${1:-toggle}"   # toggle | toA | toB | status

if [[ -z "${WAN_IFACE:-}" ]]; then
  echo "ERROR: Could not auto-detect WAN iface (prefix $BACKBONE_PREFIX)."
  ip -br addr
  exit 1
fi

have_ip() {
  ip -4 addr show dev "$WAN_IFACE" | awk '{print $2}' | grep -q "^$1/$MASK$"
}

current_ip() {
  ip -4 -o addr show dev "$WAN_IFACE" scope global | awk '{print $4}' | head -n1 || true
}

gw_from_ip() {
  local ip="${1%%/*}"
  awk -F. '{printf "%s.%s.%s.1\n",$1,$2,$3}' <<<"$ip"
}

set_ip() {
  local newip="$1"

  # Keep interface UP; just swap the address
  ip link set "$WAN_IFACE" up

  # Remove both candidates if present (idempotent)
  ip addr del "$IP_A/$MASK" dev "$WAN_IFACE" 2>/dev/null || true
  ip addr del "$IP_B/$MASK" dev "$WAN_IFACE" 2>/dev/null || true

  # Add the new one
  ip addr add "$newip/$MASK" dev "$WAN_IFACE"

  # Ensure a sane default route (optional but usually helpful)
  local gw
  gw="$(gw_from_ip "$newip/$MASK")"
  ip route replace default via "$gw" dev "$WAN_IFACE" 2>/dev/null || true

  # Nudge ARP (best-effort; arping may not be installed)
  arping -c 2 -A -I "$WAN_IFACE" "$newip" >/dev/null 2>&1 || true
}

case "$MODE" in
  status)
    echo "WAN_IFACE=$WAN_IFACE"
    echo "Current WAN IP: $(current_ip)"
    ;;
  toA)
    echo "Switching WAN IP -> $IP_A/$MASK on $WAN_IFACE"
    set_ip "$IP_A"
    ;;
  toB)
    echo "Switching WAN IP -> $IP_B/$MASK on $WAN_IFACE"
    set_ip "$IP_B"
    ;;
  toggle)
    if have_ip "$IP_A"; then
      echo "Toggling WAN IP: $IP_A -> $IP_B"
      set_ip "$IP_B"
    elif have_ip "$IP_B"; then
      echo "Toggling WAN IP: $IP_B -> $IP_A"
      set_ip "$IP_A"
    else
      echo "Neither $IP_A/$MASK nor $IP_B/$MASK found; setting $IP_A/$MASK"
      set_ip "$IP_A"
    fi
    ;;
  *)
    echo "Usage: $0 {toggle|toA|toB|status}"
    echo "Env: BACKBONE_PREFIX WAN_IFACE MASK IP_A IP_B"
    exit 2
    ;;
esac

echo "Now:"
ip -br addr show dev "$WAN_IFACE"
ip route | sed -n '1,25p'

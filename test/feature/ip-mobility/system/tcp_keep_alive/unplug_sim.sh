#!/usr/bin/env bash
set -euo pipefail

BACKBONE_PREFIX="${BACKBONE_PREFIX:-10.10.16.}"  # backbone net prefix
WAN_IFACE="${WAN_IFACE:-$(ip -o -4 addr show | awk -v p="$BACKBONE_PREFIX" '$4 ~ "^"p {print $2; exit}')}"
MODE="${1:-toggle}"  # toggle | down | up | status

if [[ -z "${WAN_IFACE:-}" ]]; then
  echo "ERROR: Could not auto-detect backbone iface (prefix $BACKBONE_PREFIX)."
  ip -br addr
  exit 1
fi

state() { ip -br link show "$WAN_IFACE" | awk '{print $2}'; }

case "$MODE" in
  status)
    echo "$WAN_IFACE is $(state)"
    ;;
  down)
    echo "Bringing $WAN_IFACE DOWN (simulate unplug)"
    ip link set "$WAN_IFACE" down
    ;;
  up)
    echo "Bringing $WAN_IFACE UP (simulate plug-in)"
    ip link set "$WAN_IFACE" up
    ;;
  toggle)
    if [[ "$(state)" == "UP" ]]; then
      echo "Bringing $WAN_IFACE DOWN (simulate unplug)"
      ip link set "$WAN_IFACE" down
    else
      echo "Bringing $WAN_IFACE UP (simulate plug-in)"
      ip link set "$WAN_IFACE" up
    fi
    ;;
  *)
    echo "Usage: $0 {toggle|down|up|status}"
    exit 2
    ;;
esac

echo "Now:"
ip -br link show "$WAN_IFACE"
ip route | sed -n '1,20p'

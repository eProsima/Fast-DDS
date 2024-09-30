echo "Putting down eth0 interface..."
ifconfig eth0 down
echo "Launching subscriber..."
${EXAMPLE_DIR}/DDSCommunicationSubscriber --xmlfile ${EXAMPLE_DIR}/simple_reliable_profile.xml --samples 10 --seed 0 --magic T --rescan 2 &
echo "Subscriber launched. Waiting 2 seconds and bring up eth0 interface..."
sleep 2s
ifconfig eth0 up
echo "eth0 interface is up. Waiting 3s for the matching to occur..."
sleep 3s

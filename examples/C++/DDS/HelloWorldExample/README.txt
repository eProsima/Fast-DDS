1. Build codes

   $ mkdir build && cd build
   $ cmake ..
   $ make

2. Copy run.sh

   $ cp run.sh build

3. Run run.sh

   $ cd build
   $ ./run.sh

   While the problem occurs, this script will exit.
   e.g.
   [Fast-DDS][add_instance] loop change : seq_no = 58
   +++ Fail: id:58, index:442 RECEIVED
   [Fast-DDS][add_instance] loop change : seq_no = 57
   +++ Fail: id:57, index:443 RECEIVED
   [Fast-DDS][add_instance] loop change : seq_no = 56
   +++ Fail: id:56, index:444 RECEIVED
   [Fast-DDS][add_instance] loop change : seq_no = 55
   +++ Fail: id:55, index:445 RECEIVED
   Subscriber unmatched.
   + sync
   + kill 4951
   + grep Fail: sub.log
   + '[' 0 -eq 0 ']'
   + break

   And you will find 'pub.log' and 'sub.log' in current directoy.
   You will find failure in 'sub.log'.

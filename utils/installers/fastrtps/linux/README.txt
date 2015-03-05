eProsima Fast RTPS installation requires the following steps:

1. eProsima Fast RTPS requires the installation of eProsima FastCDR library on
your system to run some of the provided examples and compile the code generated with eProsima FASTRTPSGEN.
eProsima FastCDR library is provided under the folder
"requiredcomponents". Extract the content of the package
"fastcdr_1.0.0.tar.gz" and execute:

    For 32-bit machines
    $ cd fastcdr; ./configure --libdir=/usr/lib; make; make install

    For 64-bit machines
    $ cd fastcdr; ./configure --libdir=/usr/lib64; make; make install

2. eProsima Fast RTPS also requires Boost libraries. Install them using your
Linux distribution package manager.

3. Install the eProsima Fast RTPS software.

    For 32-bit machines
    $ cd FastRTPS; ./configure --libdir=/usr/lib; make; make install

    For 64-bit machines
    $ cd FastRTPS; ./configure --libdir=/usr/lib64; make; make install

For more information read the Installation manual located in
"FastRTPS/doc/pdf/FastRTPS_Installation Manual.pdf"

/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

The code provided here is a template to allow the user a fast implementation of its own publisher and subscriber. It doesn't compile right now as some files are missing.

Dependencies:

- Boost 1.53
- eProsima FastBuffers
- eProsima RTPS

To complete the example and compile it please follow these steps:

1) Generate your own .idl file. We will assumme is mytype.idl.
2) Execute fastbuffers mytype.idl. You should get mytype.cpp and mytype.h.
3) Add mytype.cpp to the source files list in the Makefile.
4) Add mytype.h to the SimplePubSubType.cpp file.
5) Change the pointer to the correct type in SimplePubSubType.cpp.
6) Write your own code in the publisher and subscriber files.
7) Compile and execute!!!

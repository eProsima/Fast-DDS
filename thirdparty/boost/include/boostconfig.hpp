#pragma once

#define BOOST_DATE_TIME_NO_LIB

#ifdef _MSC_VER

#include <stdlib.h>

//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_LASTBOOTUPTIME
#define BOOST_INTERPROCESS_SHARED_DIR_PATH "C:\\ProgramData\\fastrtps_interprocess"

#include <boost/interprocess/detail/workaround.hpp>
#if defined(BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION)
#error "BOOST_INTERPROCESS_FORCE_GENERIC_EMULATION must be disabled in boost/interprocess/detail/workarround.hpp"
#endif

class SharedDirCreator
{
public:
	SharedDirCreator();
	void forzeConstruct();
	static void clean() {
		system("del " BOOST_INTERPROCESS_SHARED_DIR_PATH "\\*.* /Q");
	}
private:
	bool created_;
};

extern SharedDirCreator g_shared_dir_creator;
#define BOOST_INTERPROCESS_USE_WINDOWS
//#define BOOST_INTERPROCESS_BOOTSTAMP_IS_SESSION_MANAGER_BASED
//#define BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING
//#define BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS 1000

#endif // _MSC_VER_

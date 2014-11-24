SUBDIRS = include

lib_LTLIBRARIES = libfastrtps.la
libfastrtps_la_SOURCES = fastrtps_SOURCES

libfastrtps_la_LDFLAGS = -version-number VERSION_MAJOR:VERSION_MINOR:VERSION_RELEASE $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) -lboost_system -lboost_thread 

AM_CPPFLAGS = -std=c++0x -Iinclude -Iinclude/fastrtps $(BOOST_CPPFLAGS)
ACLOCAL_AMFLAGS = -I m4

#fastrtps_licenciasdir = $(datadir)/doc/licenses
#fastrtps_licencias_DATA = fastrtps_LICENSE.txt LGPLv3_LICENSE.txt

install-data-hook:
	mkdir -p $(DESTDIR)$(datadir)/doc/fastrtps; \
	cp -r doc/* $(DESTDIR)$(datadir)/doc/fastrtps; \
	cp -r README.html $(DESTDIR)$(datadir)/doc/fastrtps; \
	cp -r fastrtps_LIBRARY_LICENSE.txt $(DESTDIR)$(datadir)/doc/fastrtps; \
	cp -r LGPLv3_LICENSE.txt $(DESTDIR)$(datadir)/doc/fastrtps; \
	mkdir -p $(DESTDIR)$(datadir)/fastrtps; \
	cp -r examples $(DESTDIR)$(datadir)/fastrtps/;
	mkdir -p $(DESTDIR)$(datadir)/rtpsgen; \
	cp -r rtpsgen $(DESTDIR)$(datadir)/fastrtps/;

uninstall-hook:
	rm -r $(DESTDIR)$(datadir)/doc/fastrtps; \
	rm -r $(DESTDIR)$(datadir)/fastrtps; \
	rm -r $(DESTDIR)$(datadir)/include/fastrtps

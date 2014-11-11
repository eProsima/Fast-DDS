SUBDIRS = include

lib_LTLIBRARIES = libeprosimartps.la
libeprosimartps_la_SOURCES = EPROSIMARTPS_SOURCES

libeprosimartps_la_LDFLAGS = -version-number VERSION_MAJOR:VERSION_MINOR:VERSION_RELEASE $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) -lboost_system -lboost_thread 

AM_CPPFLAGS = -std=c++0x -Iinclude -Iinclude/eprosimartps $(BOOST_CPPFLAGS)
ACLOCAL_AMFLAGS = -I m4

#eprosimartps_licenciasdir = $(datadir)/doc/licenses
#eprosimartps_licencias_DATA = EPROSIMARTPS_LICENSE.txt LGPLv3_LICENSE.txt

install-data-hook:
	mkdir -p $(DESTDIR)$(datadir)/doc/eprosimartps; \
	cp -r doc/* $(DESTDIR)$(datadir)/doc/eprosimartps; \
	cp -r README.html $(DESTDIR)$(datadir)/doc/eprosimartps; \
	cp -r EPROSIMARTPS_LIBRARY_LICENSE.txt $(DESTDIR)$(datadir)/doc/eprosimartps; \
	cp -r LGPLv3_LICENSE.txt $(DESTDIR)$(datadir)/doc/eprosimartps; \
	mkdir -p $(DESTDIR)$(datadir)/eprosimartps; \
	cp -r examples $(DESTDIR)$(datadir)/eprosimartps/;
	mkdir -p $(DESTDIR)$(datadir)/rtpsgen; \
	cp -r rtpsgen $(DESTDIR)$(datadir)/eprosimartps/;

uninstall-hook:
	rm -r $(DESTDIR)$(datadir)/doc/eprosimartps; \
	rm -r $(DESTDIR)$(datadir)/eprosimartps; \
	rm -r $(DESTDIR)$(datadir)/include/eprosimartps

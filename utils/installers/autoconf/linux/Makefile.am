SUBDIRS = include

lib_LTLIBRARIES = libeprosimartps.la
libeprosimartps_la_SOURCES = EPROSIMARTPS_SOURCES

libeprosimartps_la_LDFLAGS = -version-number VERSION_MAJOR:VERSION_MINOR:VERSION_RELEASE -lboost_thread -lboost_system

AM_CPPFLAGS = -std=c++0x -Iinclude -Iinclude/eprosimartps
ACLOCAL_AMFLAGS = -I m4

#eprosimartps_licenciasdir = $(datadir)/doc/licenses
#eprosimartps_licencias_DATA = EPROSIMARTPS_LICENSE.txt LGPLv3_LICENSE.txt

install-data-hook:
	mkdir -p $(DESTDIR)$(datadir)/doc/eprosimartps; \
	cp -r doc/* $(DESTDIR)$(datadir)/doc/eprosimartps; \
	mkdir -p $(DESTDIR)$(datadir)/eprosimartps; \
	cp -r examples $(DESTDIR)$(datadir)/eprosimartps/;

uninstall-hook:
	rm -r $(DESTDIR)$(datadir)/doc/eprosimartps; \
	rm -r $(DESTDIR)$(datadir)/eprosimartps

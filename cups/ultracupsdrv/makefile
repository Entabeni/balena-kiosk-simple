VPATH = src:ppd:bin

ppds = alphacompass.ppd.gz alphapilot.ppd.gz alphapro100.ppd.gz alphapro500.ppd.gz alphapro700.ppd.gz alphapro700360.ppd.gz authentys.ppd.gz authentysplus.ppd.gz authentyspro.ppd.gz authentyspro360.ppd.gz authentyspronto.ppd.gz authentysretrax.ppd.gz doh.ppd.gz doppie100.ppd.gz doppie200.ppd.gz doppie300.ppd.gz dtpmk1.ppd.gz dtpmk2.ppd.gz dtpmk3.ppd.gz dtpmk3360.ppd.gz elliadencaroline.ppd.gz enduro.ppd.gz enduro3E.ppd.gz enduro3ELE.ppd.gz endurop.ppd.gz essentraplus.ppd.gz essentrapro.ppd.gz essentrasolo.ppd.gz fagoop310e.ppd.gz helix.ppd.gz helixrt.ppd.gz id2350e.ppd.gz identitypro.ppd.gz idmadv.ppd.gz idmadvplus.ppd.gz idmaker.ppd.gz idmsec.ppd.gz idmsecure360.ppd.gz idmvalue.ppd.gz ing171.ppd.gz it-2600.ppd.gz jke160c.ppd.gz jke700c.ppd.gz jke701c.ppd.gz matm300.ppd.gz matm500.ppd.gz mc200.ppd.gz orphdupl.ppd.gz p2500s.ppd.gz p4500s.ppd.gz p4500splus.ppd.gz pcard.ppd.gz plascorioprole360.ppd.gz ppcid2000.ppd.gz ppcid2100.ppd.gz ppcid2300.ppd.gz ppcid3100.ppd.gz ppcid3300.ppd.gz ppcid3350e360.ppd.gz ppcrtp7500w.ppd.gz pricecardproduo.ppd.gz pricecardproflex360.ppd.gz pricecardprolite.ppd.gz pricecardproxtd.ppd.gz pridento.ppd.gz pridentood.ppd.gz pridentop.ppd.gz pridentoplusod.ppd.gz pridentopro360.ppd.gz pridentopro360xtd.ppd.gz pridpro.ppd.gz pridproxtd.ppd.gz pro.ppd.gz pro360.ppd.gz pro360xtd.ppd.gz prole.ppd.gz pronto.ppd.gz proselect360.ppd.gz proxtd.ppd.gz ptpcard.ppd.gz rewritea.ppd.gz rewriteb.ppd.gz rtp7500w.ppd.gz sc2500.ppd.gz sc4500.ppd.gz sc6500.ppd.gz sc6500360.ppd.gz sc7500.ppd.gz secs200.ppd.gz titant3.ppd.gz titant5.ppd.gz titant5360.ppd.gz xr160.ppd.gz xr260.ppd.gz xr360.ppd.gz

DEFS=
LIBS=-lcupsimage -lcups 
INC=-I/ultracupsdrv/src

ifdef RPMBUILD
DEFS=-DRPMBUILD
LIBS=-ldl 
endif

define dependencies
@if [ ! -e /usr/include/cups ]; then echo "CUPS headers not available - install cups-devel for the distribution in use - #exiting"; exit 1; fi
endef

define init
@if [ ! -e bin ]; then echo "mkdir bin"; mkdir bin; fi
endef

define sweep
@if [ -e bin ]; then echo "rm -f bin/*"; rm -f bin/*; rmdir bin; fi
@if [ -e install ]; then echo "rm -f install/*"; rm -f install/*; rmdir install; fi
endef

install/setup: rastertoultra cmdtoultra $(ppds) setup
	# packaging
	@if [ -e install ]; then rm -f install/*; rmdir install; fi
	mkdir install
	cp bin/rastertoultra install
	cp bin/cmdtoultra install
	cp bin/*.ppd install
	cp bin/setup install

.PHONY: install
install:
	@if [ ! -e install ]; then echo "Please run make package first."; exit 1; fi
	# installing
	cd install; exec ./setup

.PHONY: remove
remove:
	#removing from default location (other locations require manual removal)
	@if [ -e /usr/lib/cups/filter/rastertoultra ]; then echo "Removing rastertoultra"; rm -f /usr/lib/cups/filter/rastertoultra; fi
	@if [ -e /usr/lib/cups/filter/cmdtoultra ]; then echo "Removing cmdtoultra"; rm -f /usr/lib/cups/filter/cmdtoultra; fi
	@if [ -d /usr/share/cups/model/ultra ]; then echo "Removing dir .../cups/model/ultra"; rm -rf /usr/share/cups/model/ultra; fi

.PHONY: rpmbuild
rpmbuild:
	@if [ ! -e install ]; then echo "Please run make package first."; exit 1; fi
	# installing
	RPMBUILD="true"; export RPMBUILD; cd install; exec ./setup

.PHONY: help
help:
	# Help for ultracupsdrv make file usage
	#
	# command          purpose
	# ------------------------------------
	# make              compile all sources and create the install directory
	# make install      execute the setup shell script from the install directory [require root user permissions]
	# make remove       removes installed files from your system (assumes default install location) [requires root user permissions]
	# make clean        deletes all compiles files and their folders

rastertoultra: rastertoultra.c
	$(dependencies)
	$(init)
	# compiling rastertoultra filter
	gcc -Wl,-rpath,/usr/lib -Wall -fPIC -O2 -o bin/rastertoultra src/magencd.c src/rastertoultra.c -lcupsimage -lcups -lm
#	gcc -Wl,-rpath,/usr/lib64 -Wall -fPIC -O2 -o bin/rastertoultra src/rastertoultra.c src/magencd.c -lcupsimage -lcups -lm

cmdtoultra: cmdtoultra.c
#	$(dependencies)
#	$(init)
	# compiling cmdtoultra filter
	gcc -Wl,-rpath,/usr/lib -Wall -fPIC -O2 -o bin/cmdtoultra src/cmdtoultra.c -lcupsimage -lcups -lm
#	gcc -Wl,-rpath,/usr/lib64 -Wall -fPIC -O2 -o bin/rastertoultra src/rastertoultra.c -lcupsimage -lcups -lm

$(ppds): %.ppd.gz: %.ppd
	# copy ppd file
	cp $< bin

setup: setup.sh
	$(dependencies)
	$(init)
	# create setup shell script
	cp src/setup.sh bin/setup
	chmod +x bin/setup

.PHONY: clean
clean:
	# cleaning
	$(sweep)


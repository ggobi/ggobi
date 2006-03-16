SUBDIRS = intl libltdl src plugins po

EXTRA_DIST = bootstrap CPLicense.txt

datafilesdir = $(pkgdatadir)/data
dist_datafiles_DATA = \
	data/places.xml \
  data/sleep.csv \
  data/tmin.xml \
  data/Shipman.csv \
  data/laser.csv \
  data/prim7.xml \
  data/sleep.xml \
  data/tmin1.csv \
  data/Shipman.xml \
  data/laser.xml \
  data/ratcns.csv \
  data/snetwork.xml \
  data/tmin1.xml \
  data/adhoc.xml \
  data/morsecodes.xml \
  data/ratcns.xml \
  data/stdColorMap.xml \
  data/tmin2.xml \
  data/algal-bloom.xml \
  data/olive.csv \
  data/river.xml \
  data/tao-nomiss.xml \
  data/tmin3.xml \
  data/buckyball.xml \
  data/olive.xml \
  data/roos.csv \
  data/tao.csv \
  data/tmin4.xml \
  data/cube6.xml \
  data/perm4.xml \
  data/roos.xml \
  data/tao.xml \
  data/eies.xml \
  data/perm5.xml \
  data/sarsHK.xml \
  data/tips.csv \
  data/flea.csv \
  data/pigs.xml \
  data/sat.csv \
  data/tips.xml \
  data/flea.xml \
  data/places.csv \
  data/sat.xml \
  data/tmin.csv

docfilesdir = $(pkgdatadir)/doc
dist_docfiles_DATA = \
	doc/GNUmakefile \
	doc/manual.tex \
	doc/manual.bib \
	doc/XML.tex

figuresdir = $(docfilesdir)/Figures
dist_figures_DATA = \
  doc/Figures/dbmsGUI.jpg \
  doc/Figures/olive-brush4.jpg \
  doc/Figures/olive-t1da.jpg \
  doc/Figures/rats1b.jpg \
  doc/Figures/olive-1da.jpg \
  doc/Figures/olive-colorchoose.jpg \
  doc/Figures/olive-tour1.jpg \
  doc/Figures/rats2a.jpg \
  doc/Figures/olive-1db.jpg \
  doc/Figures/olive-nth-1d.jpg \
  doc/Figures/olive-tour2.jpg \
  doc/Figures/rats2b.jpg \
  doc/Figures/olive-1dc.jpg \
  doc/Figures/olive-nth-2d.jpg \
  doc/Figures/olive-var.jpg \
  doc/Figures/rats3a.jpg \
  doc/Figures/olive-1dd.jpg \
  doc/Figures/olive-nth-t1da.jpg \
  doc/Figures/olive1.jpg \
  doc/Figures/rats3b.jpg \
  doc/Figures/olive-2da.jpg \
  doc/Figures/olive-nth-t1db.jpg \
  doc/Figures/olive2.jpg \
  doc/Figures/rats4a.jpg \
  doc/Figures/olive-2db.jpg \
  doc/Figures/olive-par.jpg \
  doc/Figures/plotLayout12.jpg \
  doc/Figures/rats4b.jpg \
  doc/Figures/olive-brush1.jpg \
  doc/Figures/olive-stha.jpg \
  doc/Figures/plotLayout22.jpg \
  doc/Figures/rats5a.jpg \
  doc/Figures/olive-brush2.jpg \
  doc/Figures/olive-sthb.jpg \
  doc/Figures/plotLayout32.jpg \
  doc/Figures/rats5b.jpg \
  doc/Figures/olive-brush3.jpg \
  doc/Figures/olive-sthc.jpg \
  doc/Figures/rats1a.jpg

sharedir = $(pkgdatadir)/share
dist_share_DATA = share/colorschemes.xml

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ggobi-system.pc

install-data-hook:
	cd $(DESTDIR)$(pkgconfigdir) && test -f ggobi-system.pc && \
  mv ggobi-system.pc ggobi.pc

uninstall-local:
	rm -f $(DESTDIR)$(pkgconfigdir)/ggobi.pc

DISTCHECK_CONFIGURE_FLAGS = --with-all-plugins

# We add symlinks for developer convenience
all-local: ggobi

ggobi:
	ln -s src/ggobi ggobi

# Use this rule to build your ggobirc (for loading plugins)
ggobirc: all
	echo "<ggobirc>" > $@
	echo "<plugins>" >> $@

	for i in $(PLUGIN_DIRS) ; do \
	  echo "" >> $@ ; \
	  cat plugins/$$i/plugin.xml >> $@ ; \
	done
	echo "</plugins>" >> $@ 
	echo "</ggobirc>" >> $@

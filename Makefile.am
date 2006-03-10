SUBDIRS                 = libltdl src plugins

# Set up special distributed files. Most of this is necessary because we 
# have our build scripts in the custom 'config' directory.
AUX_DIST                = $(ac_aux_dir)/config.guess \
                        $(ac_aux_dir)/config.sub \
                        $(ac_aux_dir)/install-sh \
                        $(ac_aux_dir)/ltconfig \
                        $(ac_aux_dir)/ltmain.sh \
                        $(ac_aux_dir)/mdate-sh \
                        $(ac_aux_dir)/missing
EXTRA_DIST              = bootstrap

dist-hook:
	 (cd $(distdir) && mkdir $(ac_aux_dir))
	 for file in $(AUX_DIST) $(AUX_DIST_EXTRA); do \
		 cp $$file $(distdir)/$$file; \
	 done
	 
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

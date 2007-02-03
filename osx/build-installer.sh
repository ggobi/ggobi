# Requires path structure in ~/ggobi as follows:
#  svn co http://ggobi.org/svn/ggobi/ggobi/trunk ggobi
#  svn co http://ggobi.org/svn/ggobi/ggobi/branches/ggobi-2.1.4 ggobi-release
#  svn co http://ggobi.org/svn/ggobi/web
#  svn co http://ggobi.org/svn/ggobi/interface/rggobi/branches/rggobi-2.1 rggobi
#  svn co http://statgraphics.had.co.nz/svn/projects/rgtk2/trunk/RGtk2
#
# Also requires latest version of xcode

# Build prereqs --------------------------------------------
sudo ruby build-installer.rb

export PATH=/usr/local/gcc4.0/bin:/usr/local/gtk2/bin:/usr/local/ggobi/bin:$PATH
export LDFLAGS=-L/usr/local/gtk2/lib
export CFLAGS=-I/usr/local/gtk2/include

# Build and install ggobi binary ---------------------------
cd ~/ggobi/ggobi-release
./configure --prefix=/usr/local/ggobi 
make
sudo make install

# Build installer ------------------------------------------
cd ~/ggobi/ggobi/osx
sudo rm -rf *.{dmg,pkg}
sudo rm -rf installer/usr/local

sudo mkdir -p installer/usr/local
sudo rsync -rltz /usr/local/gtk2 installer/usr/local 
sudo rsync -rltz /usr/local/ggobi installer/usr/local 
sudo mkdir -p installer/usr/local/gcc4.0/lib/
sudo cp /usr/local/gcc4.0/lib/libgcc_s.1.0.dylib installer/usr/local/gcc4.0/lib/ 

# Doesn't seem to work - have to build by hand
# sudo /Developer/Tools/packagemaker -build -p"Install GGobi".pkg -proj ggobi-installer.pmproj 
open ggobi-installer.pmproj 

rm *.dmg
sudo hdiutil create -fs HFS+ -volname GGobi -srcfolder "Install GGobi.pkg" ggobi-2.1.5.dmg

cp ggobi-2.1.5.dmg ~/ggobi/web/v2/downloads/ggobi-2.1.5-`uname -p`.dmg

# Build packages ----------------------------------------------
export PKG_CONFIG_PATH=/usr/local/gtk2/lib/pkgconfig:/usr/local/ggobi/lib/pkgconfig 
cd ~/ggobi

rm RGtk2/src/*.{o, so}
R CMD install RGtk2
R CMD build RGtk2

rm rggobi/src/*.{o, so}
R CMD install rggobi
R CMD build rggobi

mv {RGtk2,rggobi}_*.tar.gz ~/ggobi/web/v2/downloads/

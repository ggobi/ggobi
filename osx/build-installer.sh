sudo ruby build-installer.rb
export PATH=/usr/local/gcc4.0/bin:/usr/local/gtk2/bin:$PATH
wget http://www.ggobi.org/downloads/ggobi-2.1.4.tar.gz
sudo mkdir /usr/local/ggobi
tar xzf ggobi-2.1.4.tar.gz
cd ggobi-2.1.4
./bootstrap
LDFLAGS=-L/usr/local/gtk2/lib CFLAGS=-I/usr/local/gtk2/include ./configure --prefix=/usr/local/ggobi --with-all-plugins
make
sudo make install




mkdir -p installer/usr/local
sudo rsync -rltz /usr/local/gtk2 installer/usr/local 
sudo rsync -rltz /usr/local/ggobi installer/usr/local 
mkdir -p installer/usr/local/gcc4.0/lib/
cp /usr/local/gcc4.0/lib/libgcc_s.1.0.dylib installer/usr/local/gcc4.0/lib/ 

sudo /Developer/Tools/packagemaker -build -proj ggobi-installer.pmproj -pGGobi.pkg
 	
rm ggobi.dmg
hdiutil create -fs HFS+ -volname GGobi -srcfolder "Install GGobi.pkg" ggobi-2.1.4.dmg

cp ggobi-2.1.4.dmg ~/ggobi/web/v2/downloads/
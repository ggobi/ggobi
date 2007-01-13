ruby build-installer.rb
export PATH=/usr/local/gcc4.0/bin:/usr/local/gtk2/bin:$PATH
svn co http://www.ggobi.org/svn/ggobi/ggobi/branches/ggobi-2.1.4
sudo mkdir /usr/local/ggobi
cd ggobi-2.1.4
./bootstrap
./configure --prefix=/usr/local/ggobi --with-all-plugins
make
sudo make install

mkdir -p installer/usr/local
rsync -rltz /usr/local/gtk2 installer/usr/local 
rsync -rltz /usr/local/ggobi installer/usr/local 
mkdir -p installer/usr/local/gcc4.0/lib/
cp /usr/local/gcc4.0/lib/libgcc_s.1.0.dylib installer/usr/local/gcc4.0/lib/ 

sudo /Developer/Tools/packagemaker -build -proj ggobi-installer.pmproj -p "Install GGobi.pkg" 

rm ggobi.dmg
hdiutil create -fs HFS+ -volname GGobi -srcfolder "Install GGobi.pkg" ggobi-2.1.4.dmg

cp ggobi-2.1.4.dmg ~/ggobi/web/v2/downloads/
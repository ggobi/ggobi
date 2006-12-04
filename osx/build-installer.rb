require 'rubygems'
require 'pathname'
require 'arrayfields'
require 'fileutils'

# Check for prerequistes
# gcc -v 4.0.3

class Package 
  attr_reader :source
  attr_accessor :makeinst, :extraconf
  
  def initialize(source) 
    @source = Pathname.new(source)
    @makeinst = "make install"
  end
  
  def pkgname 
    pieces = @source.basename.to_s.split(/-/)
    pieces[0..(pieces.length - 2)].join("-")    
  end

  def self.load(packages)
    p = packages.map {|source| Package.new(source) }
    p.fields = p.map {|i| i.pkgname }
    p
  end
  
  def to_s
    "<Package: #{pkgname}>"
  end

  # Downloading source package
  # ============================
  def download_path
    Pathname.new("source") + @source.basename
  end
  def download!
    `curl #{@source} -o #{download_path}` unless download_path.exist?
  end

  def unpack!
    c = (@source.extname == ".bz2") ? "j" : "z"
    `tar -x#{c}f #{download_path} -C build` unless build_path.exist?
  end

  # Building
  # ============================
  def build_path    
    Pathname.new("build") + @source.basename.to_s.gsub(/\.tar\..*/, "")
  end

  def configure!
    return if File.exist?("#{build_path}/config.status")
    puts "configuring..."
    `cd #{build_path}; PATH=/usr/local/gcc4.0:$PATH ./configure #{$common_options} #{extraconf} 1>&2` 
    raise "Configure failure" unless $?.exitstatus == 0
  end

  def build!
    `cd #{build_path}; PATH=/usr/local/gcc4.0:$PATH PREFIX=#{$prefix} #{@makeinst} 1>&2`  
    raise "Build failure" unless $?.exitstatus == 0
  end
  
end

# Bootstrap all dependencies
# ============================================

$packages = Package.load([
  "http://pkgconfig.freedesktop.org/releases/pkg-config-0.20.tar.gz", 
  "http://ftp.gnu.org/gnu/libtool/libtool-1.5.22.tar.gz", 
  "http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.bz2", 
  "http://ftp.gnu.org/gnu/automake/automake-1.9.6.tar.bz2", 
  "http://heanet.dl.sourceforge.net/sourceforge/libpng/libpng-1.2.12.tar.bz2", 
  "ftp://ftp.remotesensing.org/pub/libtiff/tiff-3.8.2.tar.gz", 
  "http://people.imendio.com/richard/gtk-osx/files/jpeg-6b.tar.gz", 
  "http://ftp.gnu.org/gnu/gettext/gettext-0.14.5.tar.gz", 
  "http://heanet.dl.sourceforge.net/sourceforge/expat/expat-2.0.0.tar.gz", 
  "http://heanet.dl.sourceforge.net/sourceforge/freetype/freetype-2.1.10.tar.bz2", 
  "http://fontconfig.org/release/fontconfig-2.3.2.tar.gz", 
  "http://people.imendio.com/richard/gtk-osx/files/docbook-files-1.tar.gz", 
  "http://people.imendio.com/richard/gtk-osx/files/gnome-doc-utils-fake-1.tar.gz",
  "http://ftp.gnome.org/pub/GNOME/sources/gtk-doc/1.6/gtk-doc-1.6.tar.bz2",
  "http://ftp.gnome.org/pub/GNOME/sources/intltool/0.35/intltool-0.35.0.tar.bz2", 
  "http://icon-theme.freedesktop.org/releases/hicolor-icon-theme-0.9.tar.gz", 
  "http://ftp.gnome.org/pub/GNOME/sources/gnome-icon-theme/2.14/gnome-icon-theme-2.14.2.tar.bz2",
  "http://cairographics.org/releases/cairo-1.2.4.tar.gz",
  "ftp://ftp.gtk.org/pub/glib/2.12/glib-2.12.4.tar.gz", 
  "ftp://ftp.gtk.org/pub/gtk/v2.10/dependencies/atk-1.9.1.tar.bz2", 
  "ftp://ftp.gtk.org/pub/pango/1.14/pango-1.14.7.tar.gz", 
  "ftp://ftp.gtk.org/pub/gtk/v2.10/gtk+-2.10.6.tar.gz",
  "http://ftp.gnome.org/pub/GNOME/sources/libxml2/2.6/libxml2-2.6.0.tar.bz2"
])

$prefix = "/usr/local/gtk2"  # /Users/hadley/ggobi/ggobi/osx/target
$common_options="--prefix='#{$prefix}' --disable-static --enable-shared --disable-gtk-doc --disable-scrollkeeper"

$packages["jpeg"].makeinst = "make install-lib"
$packages["gtk-doc"].extraconf = "--with-xml-catalog=#{$prefix}/etc/xml/catalog"
x = $packages["libtool"]
def x.build!
  `cd #{build_path};PATH=/usr/local/gcc4.0:$PATH #{@makeinst}`
  raise "Build failure" unless $?.exitstatus == 0
  # Setup glibtool* links since some stuff expects them to be named like that on OSX
  `ln -s #{$prefix}/bin/libtoolize #{$prefix}/bin/glibtoolize`
  `ln -s #{$prefix}/bin/libtool #{$prefix}/bin/glibtool`
end


def build() 
  # set_env(arch)
  # `mv target-#{$arch} target`

  FileUtils.mkdir_p "build"
  FileUtils.mkdir_p $prefix unless File.exist? $prefix
  
  ENV['PREFIX'] =       $prefix
  ENV['PATH']=          ["/usr/local/gcc4.0/bin", "#{$prefix}/bin", ENV['PATH']].join(":")
  ENV['LIBTOOLIZE']=    "#{$prefix}/bin/libtoolize"
  ENV['LDFLAGS']=       "-L#{$prefix}/lib"
  ENV['CPPFLAGS']=      "-I#{$prefix}/include"
  ENV['XDG_DATA_DIRS']= "#{$prefix}/share"
  
  begin
    $packages.each do |p| 
      puts "\n\n\n" + p.pkgname + " " + "-" * (60 - p.pkgname.length)
      p.download!
      p.unpack!
      p.configure!
      p.build!
    end      
  rescue Exception => e
    puts e
  ensure
    #`mv target target-#{$arch}`    
  end
end

build 
# Build to /usr/local/gtk2

# PKG_CONFIG=/usr/local/gtk2/lib/pkgconfig
# export PATH=/usr/local/gcc4.0/bin:/usr/local/gtk2/bin:$PATH
# svn co http://www.ggobi.org/svn/ggobi/ggobi/branches/ggobi-2.1.4
# sudo mkdir /usr/local/ggobi
# cd ggobi-2.1.4
# ./bootstrap
# ./configure --prefix=/usr/local/ggobi --with-all-plugins
# make
# sudo make install

# mv to installer
# 
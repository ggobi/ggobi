use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'FIRST_MAKEFILE'  => 'Makefile.perl',
    'MAKEFILE'  => 'Makefile.perl',
    'NAME'	=> 'GGobi',
    'VERSION_FROM' => 'GGobi/plugin.pm', 
    'LIBS'	=> [ "-L../../ -lggobi" ],   
    'DEFINE'	=> '-g',    
    'INC'	=> '-I. -I../.. `gtk-config --cflags`',
);
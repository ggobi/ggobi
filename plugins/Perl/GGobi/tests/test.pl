use lib "/home/duncan/Projects/ggobi/ggobi/plugins/Perl/GGobi";
use lib "/home/duncan/Projects/ggobi/ggobi/plugins/Perl";

require plugin;

use GGobi::plugin;

$p = new GGobi::Plugin();

$p->onClose();

$x = new GGobi::ArgList(1,"abc", "def");
print $x, " ", $x->[2], "\n";

$x = new GGobi::ArgTable('a' => 1, 'abc' => "def");
print $x, " ", $x->{'abc'}, "\n";

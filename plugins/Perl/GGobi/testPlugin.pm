use GGobi::plugin;

{
=head1 DESCRIPTION
 This is a simple test of the Perl plugin mechanism for GGobi.
 This implements a very basic plugin that doesn't really do much.
 It just proves that the Perl language plugin is working.
=cut
  package GGobi::TestPlugin;

  sub new {
      print "[GGobi::TestPlugin] # arguments: $#_\n";

      my ($class, $gg, $plugin, $namedArgs, $args) = @_;

      print "[GGobi::TestPlugin] class = ", $class,"\n";

      print "gg ", $gg, "\n";
      print "plugin ", $plugin, "\n";
 
      print "args: ", ref($args),"\n";
      foreach $z (@{$args}) {
	  print $z,"\n";
      }

      print "namedArgs: ", ref($namedArgs),"\n";
      foreach $z (keys(%{$namedArgs})) {
	  print $z, ": ", $namedArgs->{$z}, "\n";
      }

      my $this = { 'x' => 1};
      bless \$this, $class;
  }
}

# print "Loaded testPlugin.pm\n";

1;

use GGobi;

require GGobiReferences;

=head1 General Perl Plugin for GGobi

This is the basic class that implements the general (i.e. not
input/data) plugin. It provides methods for the basic hooks that
GGobi calls at different times to load, instantiate, update menus
for a GGobi instance.

=cut

{
=head1 GGobi::Plugin

This is the standard base class for a Plugin
 that provides the basic methods that are called
 directly from the GGobi-Perl plugin mechanism.
 One can extend this and be certain that all the
 relevant methods are provided and in a degenerate
 fashion so as to cause no extra computation.

=cut
 package GGobi::Plugin;

 sub new() {
    print "[ggobi::plugin] In onCreate\n";
    my $class = shift;
    my $this = 1;
    bless \$this, $class;
 }


=head2 onClose

This method is currently never invoked.
  The DESTROY method is where we clean up when 
  plugin instance is no longer needed.

=cut

 sub onClose() {
    print "[ggobi::plugin] In onClose\n"; 
    return(1);
 }

=head2 updateDisplayMenu

This is the method that is called when a new dataset is added to 
 the GGobi instance and so the Display menu in the toolbar is augmented
 with controls for the new dataset. 
 If this plugin needs to add to this menu for a dataset,
 it can do this at this point.

=cut
  sub updateDisplayMenu {
      my $this = shift;
      print "[updateDisplayMenu] ", ref($this), "\n";

      return(1);
  }
}

# print "Loaded plugin.pm\n";

=begin comment


# These two Arg* classes are not used.
{
    package GGobi::ArgList;
sub new {
    my $class = shift;
    my @this = (@_);

    bless \@this, $class;
}
}

{
    package GGobi::ArgTable;
    sub new {
	my $class = shift;
	my %this = @_;

	bless \%this, $class;
    }
}

=end comment

=cut

1;




=head1 AUTHOR

 Duncan Temple Lang duncan@research.bell-labs.com

=head1 SEE ALSO

=over 4

=item *
    perl(1).

=item *
     GGobi - http://www.ggobi.org

=item *
     GGobi Plugins - http://www.ggobi.org/plugins.pdf

=back
=cut


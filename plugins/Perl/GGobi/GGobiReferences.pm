=head1 C-level GGobi References

 Perl Classes for representing references to C-level data structure
 instances in GGobi. These represent things such as ggobi,
 data and plugin instances.

=cut

{

=head1 GGobi::GGobiGenericRef

 This is the lowest-level base class for representing a reference to a
 C-level data structure instance in Perl. We extend this class to
 provide interfaces to different types.  It simply stores the address
 (passed as the second argument) of the C-level object.

=cut

    package GGobi::GGobiGenericRef;

    sub new {
	my ($class, $ptr) = @_;
        my $this = $ptr; # { 'address' => $ptr };

	my $x = bless \$this, $class;
	return($x);
    }
}

{
=head1 GGobi::GGobiRef

  This is a proxy/reference in Perl for a C-level
  ggobid instance.
  
=cut
    package GGobi::GGobiRef;
    our @ISA = qw(GGobi::GGobiGenericRef);

    sub numDatasets {
        my $this = shift;
	@x = GGobi.dataSetNames($this);
        return($#x + 1);
    }

    sub DESTROY {
#	print "GGobiRef::Destroy\n";
    }
}

{

=head1 GGobi::PluginInstRef

  A reference to a GGobi plugin instance.

=cut

    package GGobi::PluginInstRef;
    our @ISA = qw(GGobi::GGobiGenericRef);
    sub DESTROY {
#      print "PluginInstRef::Destroy\n";
    }
}


print "Loaded GGobiReferences\n";

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



{
=head GGobi::GGobiGenericRef
 This is the lowest-level base class for 
 representing a reference to a C-level data structure
 instance in Perl. We extend this class to provide 
 interfaces to different types.
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
=head GGobi::GGobiRef
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
	print "GGobiRef::Destroy\n";
    }
}

{
    package GGobi::PluginInstRef;
    our @ISA = qw(GGobi::GGobiGenericRef);
    sub DESTROY {
       print "PluginInstRef::Destroy\n";
    }
}


print "Loaded GGobiReferences\n";

1;

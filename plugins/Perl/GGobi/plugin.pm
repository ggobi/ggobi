{
    package GGobi::ArgList;
sub new {
    my $class = shift;
    my @this = (@_);
    foreach $x (@this) {
	print $x, "\n";
    }
    bless \@this, $class;
}
}

{
    package GGobi::ArgTable;
sub new {
    my $class = shift;
    my %this = @_;
    foreach $x (keys(%this)) {
	print $x,"\n";
    }
    bless \%this, $class;
}
}

{
    package GGobi::GGobiGenericRef;

    sub new {
	my ($class, $ptr) = @_;
        my $this = { 'address' => $ptr };

	my $x = bless \$this, $class;
	return($x);
    }
}

{
    package GGobi::GGobiRef;
    our @ISA = qw(GGobi::GGobiGenericRef);

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


{
 package GGobi::Plugin;


 sub new() {
    print "[ggobi::plugin] In onCreate\n";
    my $class = shift;
    my $this = 1;
    bless \$this, $class;
 }


 sub onClose() {
    print "[ggobi::plugin] In onClose\n"; 
    return(1);
 }
}

# print "Loaded plugin.pm\n";

1;

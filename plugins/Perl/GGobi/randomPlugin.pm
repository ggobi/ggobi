require inputPlugin;

# use GGobi::PullInputPlugin;

{
    package PullRandom;
    our @ISA = qw(GGobi::PullInputPlugin);
    sub new {
	my ($class, $gg, $plugin, $namedArgs, $args) = @_;

        # read the parameters
        my $this = {
                    numRows => 10,
                    numVars => 3
                   };

	print join("\n", keys(%{$namedArgs})), "\n";
	print "numRows: ", $namedArgs->{'numRows'}, "\n";

        $this->{numRows} = $namedArgs->{'numRows'} if defined $namedArgs->{'numRows'};
        $this->{numVars} = $namedArgs->{'numVars'} if defined $namedArgs->{'numVars'};
 
        bless $this, $class;
    }

    sub getNumRecords {
	my $this = shift;
	return($this->{numRows});
    }

    sub getVariableNames {
	my $this = shift;
        my @x = ();
	my $ctr = 0;
        foreach $z (1..$this->{numVars}) {
	    $x[$ctr] = "Var " . $z;
	    $ctr++;
	}
	return(@x);
    }

    sub getRecord {
	my $this = shift;
	my @vals = ();
	my $ctr = 0;
        while($ctr < $this->{numVars}) {
	    $vals[$ctr++] = rand(100);
	}
	return(@vals);
    }

   sub getSourceDescription {
       my ($this, $brief) = @_;
       my $txt = "";
       if($brief) {
	   $txt = "PerlRandom";
       } else {
 	   $txt = "Randomly generated data via a Perl input plugin";
       }
       return($txt);
   }
}

1;

require inputPlugin;

# use GGobi::PullInputPlugin;

{

=head1 DESCRIPTION

This is an example of an input plugin in Perl.  It simply
   generates its input at random.  The dimensions of the dataset
   (i.e. number of rows and variables) can be specified in the
   configuration for the plugin using the C<named> element within
   the C<options> element of the XML defining the plugin.

=cut

    package PullRandom;
    our @ISA = qw(GGobi::PullInputPlugin);



    sub new {
	my ($class, $gg, $plugin, $namedArgs, $args) = @_;

        # read the parameters
        my $this = {
                    numRows => 10,
                    numVars => 3
                   };

         # override the default dimensions if the number of rows or variables
         # is specified in the 
         # see plugin.xml in the directory above.

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
         # Var 1, Var 2, ..., Var n
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
         # make up an array numVars random numbers.
         #
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


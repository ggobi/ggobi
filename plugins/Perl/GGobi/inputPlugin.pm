require GGobiReferences;

use GGobi;

{
   package GGobi::InputPlugin;

   sub new {
       my $class = shift; 
       my $this = {};

       bless \$this, $class;
   }

}

{
=head1 DESCRIPTION
  The idea for this plugin is that it reads the data when it is
  instantiated/created and then GGobi asks it for the details of the
  dataset and GGobi creates and populates the datad by calling the different
  methods. In this way, GGobi 'pulls' the data from the plugin.
  This is in contrast with the 'push' mechanism in which the plugin
  creates and populates the datad instance in GGobi directly by itself.
=cut
   package GGobi::PullInputPlugin;

   our @ISA = qw(GGobi::InputPugin);  

   sub getNumRecords {
       my ($this) = shift;

       return(-1);
   }

   sub getNumVariables {
       my ($this) = shift;

       return(-1);
   }

   sub getVariableNames {
       my ($this) = shift;

       return(undef);
   }

   sub getSourceDescription {
       my ($this, $brief) = @_;

       my $desc;
       if($brief) {
	   $desc = "Degenerate/unimplemented plugin";
	} else {
	   $desc = "This is an unimplemented plugin";
	}
       return($desc);
   }
}

{
  package GGobi::RecordWisePlugin;
  our @ISA = qw(GGobi::PullInputPugin);


  sub getRecord {
      my ($this, $which) = @_;
 
      return(undef);
  }
}




{
=head1 PushInputPlugin
 
  This version of the GGobi input plugin in Perl works
  by reading the dataset(s) and then returning them 
  as complete objects.
  This is more independent of GGobi than the PullInputPlugin interface
  in which GGobi must do more work to create the datad itself.

=cut 

    package GGobi::PushInputPlugin;
    our @ISA = qw(GGobi::InputPlugin);

=head1 getData

 This is called by GGobi after the plugin has been created
 and the GGobi instance initialized and it is expected to 
 to create and return the datasets.

 What is the most convenient interface for returning multiple
 datasets? as an array of datad references?

=cut
    sub getData {
	my ($this, $gg) = @_;
	my $data = undef;  # Create the datad here (using the GGobi::References module)
         # ... populate the datad 

 	return($data);
    }
}

require GGobiReferences;

use GGobi;

=head1 Perl Input Plugins

 These define a hierarchy of input plugin classes that allow us to use
 Perl objects to read data into GGobi.  There are two styles of
 plgins: pull and push.  Pull plugins get the data and leave it to the
 core GGobi/Perl interface to query the number of records and
 variables, create the dataset and fill in each record as GGobi is
 ready for them.

 See randomPlugin.pm for an example.

=cut

{

=head2 GGobi::InputPlugin

 This is the basic, top-level input plugin class.
 It provides no methods and must be extended to be useful.

=cut

   package GGobi::InputPlugin;

   sub new {
       my $class = shift; 
       my $this = {};

       bless \$this, $class;
   }

}

{

=head1 GGobi::PullInputPlugin

  This is the high-level Pull input plugin.  The idea is that it reads
  the data when it is instantiated/created and then GGobi asks it for
  the details of the dataset and GGobi creates and populates the datad
  by calling the different methods. In this way, GGobi 'pulls' the
  data from the plugin.  This is in contrast with the 'push' mechanism
  in which the plugin creates and populates the datad instance in
  GGobi directly by itself.

=cut
   package GGobi::PullInputPlugin;

   our @ISA = qw(GGobi::InputPugin);  

=head2 getNumRecords

  Query the number of records in this dataset.

=cut

   sub getNumRecords {
       my ($this) = shift;

       return(-1);
   }

=head2 getNumVariables

  Query the number of variables.  This is not actually used, since we
  call C<getVariableNames> and compute the length of that array to
  determine the number of variables.

=cut

   sub getNumVariables {
       my ($this) = shift;

       return(-1);
   }

=head2 getVariableNames

   Query the names of the variables in this dataset.

=cut

   sub getVariableNames {
       my ($this) = shift;

       return(undef);
   }

=head2 getSourceDescription

   Get either a short or verbose description of the dataset.  The
   short version is used in menus, the display tree, etc.  The long
   version is used in providing a human-readable description of the
   dataset.

   The second argument is a logical value (1 or 0) indicating whether
   we want the brief or verbose description, respectively.

=cut

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

=head1 GGobi::RecordWisePlugin

  This provides a method to allow each record to be queried one at a
  time.  Extensions or parallel classes might provide a C<nextRecord>
  method and remember where they are in the stream.

=cut

  package GGobi::RecordWisePlugin;
  our @ISA = qw(GGobi::PullInputPugin);


  sub getRecord {
      my ($this, $which) = @_;
 
      return(undef);
  }
}




{

=head1 GGobi::PushInputPlugin
 
  This version of the GGobi input plugin in Perl works
  by reading the dataset(s) and then returning them 
  as complete objects.
  This is more independent of GGobi than the PullInputPlugin interface
  in which GGobi must do more work to create the datad itself.

=cut 

    package GGobi::PushInputPlugin;
    our @ISA = qw(GGobi::InputPlugin);

=head2 getData

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


package GGobi;

require 5.005_62;
use strict;
use warnings;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use GGobi ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);
our $VERSION = '0.10';

bootstrap GGobi $VERSION;

# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

GGobi - Interface from Perl to native GGobi code

=head1 SYNOPSIS

  use GGobi;


=head1 DESCRIPTION

 This currently provides access from Perl within GGobi when Perl is acting as a plugin.
 Access is limited to a very small subset of the GGobi API, and will evolve over time.

=head2 EXPORT

    numDataSets(GGobi::GGobiRef);


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

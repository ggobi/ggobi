#!/usr/common/bin/perl

# usage: xmlConvert base
#   reads base.nodes and base.edges and produces base.xml

if (!$ARGV[0]) {
  print "usage: xmlConvert base\n" ;
  exit 0;
}

$base = $ARGV[0] ;

# First read in the node variables.  Count the number of nodes,
# and prepare two associative arrays:  use nodes to write out
# the node records, and use nodeindex to get the node indices for
# the edge records.
$nnodevars = 0;
$nnodes = 0;
open (FID, "<$base.nodes") ;
while ($_ = <FID>) {
  chomp();
  # Assume that the first row contains the variable labels
  if ($nnodevars == 0) {
    (@nodevarlabels) = split ;
    $nnodevars = $#nodevarlabels + 1;
  } else {
    ($node, $nodes{$nnodes}) = split ' +', $_, 2 ;
    $nodelabel{$nnodes} = $node;
    $nodeindex{$node} = $nnodes;
    $nnodes++;
  }
}
close FID;

# If there's a rowlabels file, then overwrite $nodelabel{$nnodes}
# with that information
$i = 0;
open (FID, "<$base.rowlab") ;
while ($_ = <FID>) {
  chomp();
  $nodelabel{$i} = $_;
  $i++;
  if ($i >= $nnodes) {
    break;
  }
}
close FID;

open (FID, "<$base.edges") ;
$nedgevars = 0;
$nedges = 0;
while ($_ = <FID>) {
  chomp();
  if ($nedgevars == 0) {
    (@edgevarlabels) = split ;
    $nedgevars = $#edgevarlabels + 1;
  } else {
    ($edgea, $edgeb, $evars) = split ' ', $_, 3 ;
    $edges{$nedges,a} = $nodeindex{$edgea};
    $edges{$nedges,b} = $nodeindex{$edgeb};
    $edges{$nedges} = $evars;
    $nedges++;
  }
}
close FID;

# write_xml_header

 print "<?xml version=\"1.0\"?>";
 print "\n";
 print "<!DOCTYPE ggobidata SYSTEM \"ggobi2.dtd\">";
 print "\n\n";

# write_dataset_header

 print "<ggobidata ";
 print "missingValue=\".\" " ;
 print "numRecords=\"$nnodes\"" ;
 print ">\n" ;

# write_xml_description

 print "<description>\n" ;
 print "</description>\n" ;

# write_xml_variables

 print "<variables count=\"$nnodevars\">\n" ;
 for ($j=0; $j<$nnodevars; $j++) {
  print "  <variable>" ;
  print $nodevarlabels[$j] ;
  print "</variable>\n" ;
 }
 print "</variables>\n" ; 

# write_xml_records

 print "<records>\n" ;

# read a row, write a row: turn the label column into a label attribute
 for ($i = 0; $i < $nnodes; $i++) {
   # write_xml_record
   print "<record" ;
   print " label=\"$nodelabel{$i}\"" ;
   print ">\n" ;
   print $nodes{$i};
   print "\n" ;
   print "</record>\n" ;
 }

 print "</records>\n\n" ;
# end of nodes and node variables

# the edge variables
 print "<edgevariables count=\"" . $nedgevars . "\">\n" ;
 for ($j=0; $j<$nedgevars; $j++) {
  print "  <edgevariable>" ;
  print $edgevarlabels[$j] ;
  print "</edgevariable>\n" ;
 }
 print "</edgevariables>\n" ; 

 print "<edgerecords\n" ;
 print " numEdgeRecords=\"$nedges\"" ;
 print ">\n" ;

 for ($i = 0; $i < $nedges; $i++) {
   print "<edgerecord " ;
   print "source=\"$edges{$i,a}\" destination=\"$edges{$i,b}\"" ;
   print ">\n" ;
   print $edges{$i} ;
   print "\n" ;
   print "</edgerecord>\n" ;
 }
 print "</edgerecords>\n" ;

# write_dataset_footer
 print "</ggobidata>\n" ;

#!/usr/bin/perl

my $ARGC=@ARGV;

sub PrintSeveral {
	my ($Host, $Repetitions) = @_;
	for (my $i=0; $i<$Repetitions; $i++) {
		print $Host."\n";
	}
}

if ($ARGC == 1) {
	$ProcsPerNode = $ARGV[0];
}
else {
	$ProcsPerNode = 1;
}

my $IN=$ENV{SLURM_NODELIST};

my ($HostPrefix, $NumbersList) = $IN =~ /(\w+)\[(.+)\]/x ;

my @NumbersArray = split(/,/, $NumbersList);

for (my $i=0; $i<@NumbersArray; $i++)
{
	if ( index($NumbersArray[$i], '-') == -1 ) 
	{
		PrintSeveral($HostPrefix.$NumbersArray[$i],  $ProcsPerNode);
	}
	else
	{
		my ($RangeMin, $RangeMax) = split(/-/, $NumbersArray[$i]);
		for (my $j=$RangeMin; $j<=$RangeMax; $j++)
		{
			PrintSeveral($HostPrefix.$j, $ProcsPerNode);
		}
	}
}


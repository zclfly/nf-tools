#!/usr/bin/perl -w 
#
# Converts stdin and converts NFL_I_XXX nad NFL_T_XXX fields into 
# perl hash structure 
#

my $SUFFIX = "\t\t# generated by h2fields.pl";

use strict; 

my %H;

while (<STDIN>) {
	# parse follwig lines
	# #define NFL_T_FIRST         "first"     // pod:## - Timestamp of first seen packet E<10>
	# #define NFL_I_FIRST         01
	if (/#define\s+NFL_(\w)_(\w+)\s+(.+?)\s+/) {
		my ($type, $field, $value) = ($1, $2, $3);
		
		$value =~ s/\"//g;
		$H{$field}->{$type} = $value;
	}
}

printf "# generated with h2fields.pl\n";
printf "package Net::NfDump::Fields;\n";
printf "our %%NFL_FIELDS_INT = (\n";
foreach ( sort { $H{$a}->{'I'} <=> $H{$b}->{'I'} }  keys %H ) {
	printf "\t%d => \'%s\',\n", $H{$_}->{'I'}, $H{$_}->{'T'};

}
printf ");\n\n";

printf "our %%NFL_FIELDS_TXT = (\n";
foreach ( sort { $H{$a}->{'I'} <=> $H{$b}->{'I'} }  keys %H ) {
	printf "\t\'%s\' => %d,\n", $H{$_}->{'T'}, $H{$_}->{'I'};

}
printf ");\n\n";


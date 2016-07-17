#!/usr/bin/perl

#CSV:xlacko06

use strict;
use warnings;
use utf8;
use Encode;
use Text::CSV;
use Getopt::Long qw(Configure GetOptions);

Getopt::Long::Configure("no_auto_abbrev", "no_ignore_case");#, "require_order");

# navratove hodnoty programu
our $ALL_OK = 0;
our $WRONG_PARAMS = 1;
our $WRONG_INPUT = 2;
our $WRONG_OUTPUT = 3;
our $WRONG_FF = 4;
our $INVALID_ELEMENT = 30;
our $INVALID_HEADER = 31;
our $INVALID_COLUMNS = 32;


################################################################################
# pomocne funkce pre spracovanie jedneho riadku a ukoncenie programu
################################################################################

# spracovani radku rozparsovaneho pomocou Text::CSV
sub handle_row(\@\@$$) {
	my @tags = @{$_[0]}; 	# "tagy" elementov
	my @row = @{$_[1]};		# hodnoty elementov
	my $info = $_[2];		# informacie o zachadzani s riadkom
	my $m_value = $_[3];	# nahradna hodnota pre chybajuce stlpce
	my $temp;
	for (@tags) {
		$temp = shift(@row);
		if (defined($temp)) {
			$temp =~ s/&/&amp;/g;
			$temp =~ s/</&lt;/g;
			$temp =~ s/>/&gt;/g;
			$temp =~ s/"/&quot;/g;
			print OUTPUT "<$_>",$temp,"</$_>";
		}
		else { 	# malo hodnot, doplni sa nahradnym textom
			if ($info & 1) {		# alebo ostane prazdne
				print OUTPUT "<$_>",$m_value,"</$_>";
			}
			else {
				print OUTPUT "<$_>","</$_>";
			}
		}
	}
	if ($info & 2) {
		my $index = scalar(@tags);	# $index sluzi ako novy index
		for (@row) {	# tisk prebyvajucich hodnot
			$_ =~ s/&/&amp;/g;
			$_ =~ s/</&lt;/g;
			$_ =~ s/>/&gt;/g;
			$_ =~ s/"/&quot;/g;
			print OUTPUT "<col",++$index,">",$_,"</col$index>";
		}
	}
}

# uzatvorenie vstupneho/vystupneho prudu, exit
sub ExitProgram($) {
	# print OUTPUT "Exit: ", $_[0], "\n";
	close INPUT;
	close OUTPUT;
	exit $_[0];
}

################################################################################
# konec pomocnych funkci
################################################################################

################################################################################
# spracovani argumentu
################################################################################
our $help; our $input; our $output; our $nogen; our $root_elem; our $separator;
our $header; our $line_elem; our $index; our $start_index; our $e_rec; our $m_value;
our $all_cols;

foreach ( @ARGV ) {
	$_ = Encode::decode_utf8( $_);
}

# kontrola rovnakych parametrov 
my $arg_scal = scalar(@ARGV)-1;
my $h1;
my $a;
for ($a = 0; $a < $arg_scal; $a++) {
	for (my $b = $a+1; $b <= $arg_scal; $b++) {
		$ARGV[$a] =~ /^-+([^=]*)/;
		$h1 = $1;
		$ARGV[$b] =~ /^-+([^=]*)/;
		exit $WRONG_PARAMS if ($h1 eq $1);
		if (($h1 eq "e") && ($1 eq "error-recovery")) {
			exit $WRONG_PARAMS;
		}
	}
}

exit $WRONG_PARAMS if (!GetOptions(
		"help" => \$help,
	"input=s" => \$input,
"output=s" => \$output,
"n" => \$nogen,
'r=s' => \$root_elem,
"s=s" => \$separator,
"h" => \$header,
"l=s" => \$line_elem,
"i" => \$index,
"start=i" => \$start_index,
"e|error-recovery" => \$e_rec,
"missing-value=s" => \$m_value,
"all-columns" => \$all_cols
));

# ostali na vstupe este nejake parametre ?
if (scalar(@ARGV) != 0) {
	exit $WRONG_PARAMS;
}

if ($help){ # spracovani napovedy
	if (not ($input or $output or $nogen or $root_elem or $separator or $header
				or $line_elem or $index or $start_index or $e_rec or $m_value or $all_cols)){ 
		print 
		"pouziti:\
--help 
    vytiskne tuto napovedu

--output=filename 
    textovy vystupni XML soubor s obsahem prevedenym ze vstupniho souboru.
-n 
    negenerovat XML hlavicku na vystup skriptu 

-r=root-element 
    jmeno paroveho korenoveho elementu obalujici vysledek

-s=separator 
    nastaveni separatoru (jeden znak) sloupcu na kazdem radku vstupniho CSV

-h 
    prvni radek (presneji prvni zaznam) CSV souboru slouzi jako hlavicka

-l=line-element 
    jmeno elementu, ktery obaluje zvlast kazdy radek vstupniho CSV

-i 
    zajisti vlozeni atributu index s ciselnou hodnotou do elementu line-element

--start=n 
    inicializace inkrementalniho citace pro parametr -i na zadane kladne cele cislo

-e, --error-recovery 
    zotaveni z chybneho poctu sloupcu na neprvnim radku

--missing-value=val 
    pokud nejaka vstupni bunka (sloupec) chybi, tak je doplnena zde uvedena hodnota val

--all-columns 
    sloupce, ktere jsou v nekorektnim CSV navic, nejsou ignorovany, ale jsou take vlozeny do vysledneho XML\n";
		exit $ALL_OK;
	}
	else {
		exit $WRONG_PARAMS;
	}
}

################################################################################
# overeni vstupu a vystupu, otvorenie v binmode pre spracovanie utf znakov
################################################################################

if (defined($input)) {	
	open(INPUT, "<$input") or &ExitProgram ( $WRONG_INPUT);
}
else {
	open(INPUT, "<&STDIN");
}

if (defined($output)) {	
	open(OUTPUT, ">$output") or &ExitProgram ($WRONG_OUTPUT);
}
else {
	open(OUTPUT, ">&STDOUT");
}
binmode(OUTPUT, ":utf8");
binmode(INPUT, ":encoding(utf8)");

################################################################################
# overeni korektnosti korenoveho elementu
################################################################################

if (defined($root_elem)) {
	chomp($root_elem);
	$root_elem =~ /^[:A-Z_a-z\x{7f}-\x{EFFFF}][:A-Z_a-z.0-9\x{7f}-\x{EFFFF}-]*$/ or &ExitProgram ($INVALID_ELEMENT);
}

# overeni korektnosti separatoru (delka=1 alebo TAB)
if (defined($separator)) {
	if (($separator =~ /^TAB$/)) {
		$separator = "\t";
	} elsif (length($separator) != 1) {
		&ExitProgram ($WRONG_PARAMS);
	}
} else {
	$separator = ",";
}

# overeni korektnosti line-elementu
if (defined($line_elem)) {
	$line_elem =~ /^[:A-Z_a-z\x{7f}-\x{EFFFF}][:A-Z_a-z.0-9\x{7f}-\x{EFFFF}-]*$/ 
		or &ExitProgram ($INVALID_ELEMENT);
}
# overeni pripadne nahradeni znakov v missing-value
if (defined($m_value)) {
	$m_value =~ s/&/&amp;/g;
	$m_value =~ s/</&lt;/g;
	$m_value =~ s/>/&gt;/g;
	$m_value =~ s/"/&quot;/g;
}

################################################################################
# praca s indexom: kontrola spravnosti parametrov a znakov v indexe
################################################################################

if (defined($start_index)) {
	if (not defined($index) or not defined($line_elem)) {
		&ExitProgram ($WRONG_PARAMS);
	}
	else {
		$index = $start_index if $start_index =~ /^[0-9]*$/ or &ExitProgram ($WRONG_PARAMS);
	}
} else {
	if (not defined($index) and not defined($line_elem)) {
		$line_elem = "row";
	}
	elsif (defined($index) and not defined($line_elem)) {
		&ExitProgram ($WRONG_PARAMS);
	}
	elsif (defined($index) and defined($line_elem)) {
		$index = 1;
	}
}

################################################################################
# vysporiadanie sa s chybnym CSV
################################################################################

our $recovery = 0; 	# sluzi na identifikaciu typu opravy
if (defined($e_rec)) {
	if (defined($m_value)) {
		$recovery = $recovery | 1; # bitove flagy
	}
	if (defined($all_cols)) {
		$recovery = $recovery | 2;
		if (defined($header)) {
			$recovery = $recovery | 4;
		}
	}
}
elsif (defined($m_value) or defined($all_cols)) {
	&ExitProgram ($WRONG_PARAMS);
}

################################################################################
# # # # # # # # # # # # # # # # #    # #  ####    #####   ####   # #     #      
# samotne spracovani CSV suboru #    # #  ###       #     ###    # #     #     
# # # # # # # # # # # # # # # # #    #    ####      #     ####   #  #    #### 
################################################################################

my $csv = Text::CSV->new ({
		sep_char => $separator,
		allow_whitespace => 0,
		eol => "\015\012",
		binary => 1,
	});

# generovani hlavicky
if (not defined($nogen)) {
	print OUTPUT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

# generovani korenoveho elementu
if (defined($root_elem)) {
	print OUTPUT "<$root_elem>\n";
}


################################################################################
# generovanie nazvov stlpcovych elementov - spracovanie prveho riadku vstupu
# ak bol zadany parameter -h, prvy riadok sa pouzije ako nazvy stlpcovych
# elementov, nahradia sa nekorektne znaky
# alebo sa nazvy stlpcovych elementov vygeneru s nazvom colX, pre pocet stlpcov
# v riadku
################################################################################

# flag indikujuci nacteni CRLF
my $crlf = 1;

my @col_elements;
my @field;
my $col_number = 1;
if (defined($header)) {	# ako stlpcove elementy sa vezmu hodnoty prveho riadku
	$_ = <INPUT>;
	if ((not defined($_)) || ($_ !~ /\r\n$/)) {
		$crlf = 0;
	}
	if(defined($_) && $csv->parse($_)) {
		@col_elements=$csv->fields;
	}	
	else { 
		&ExitProgram ($WRONG_FF); 
	}
	for (@col_elements) { # kontrola jednotlivych elementov a nahradenie nekorektych znakov
		&ExitProgram ($INVALID_HEADER) if ( $_ !~ /^[:A-Z_a-z\x{7f}-\x{EFFFF}]/); # exit, ak sa začina nepripustym znakom
		$_ =~ s/[^A-Za-z0-9\x{7f}-\x{EFFFF}:._-]/-/g
	}
	$col_number = scalar(@col_elements);
}
else {	# stlpcove elementy budu colX
	$_ = <INPUT>; 
	if ((not defined($_)) || ($_ !~ /\r\n$/)) {
		$crlf = 0;
	}
	if(defined($_) && $csv->parse($_)) {
		@field=$csv->fields; 
		$col_number = scalar(@field);
	}
	for(my $a = 1; $a <= $col_number; $a++) {
		$col_elements[$a-1] = "col" . $a;
	}
	print OUTPUT "<$line_elem";
	print OUTPUT " index=\"",$index++, "\"" if (defined($index));
	print OUTPUT ">";
	&handle_row(\@col_elements, \@field, 0, $m_value); #\*OUTPUT);
	print OUTPUT "</$line_elem>\n";
}

################################################################################
# subor sa v cykle nacita po riadkoch, testuje sa priznak koncu riadku
# rozparsuje sa do pola, ktore sa pomocou funkcie handle_row spracuje
################################################################################
while(<INPUT>) {
	if ($crlf == 0) {
		&ExitProgram ($WRONG_FF);
	}
	if ($_ !~ /\r\n$/) {
		$crlf = 0;
	}
	if($csv->parse( $_)) {	# naparsovani radku
		@field=$csv->fields;	# ulozeni naprasovanych hodnot
		print OUTPUT "<$line_elem";	# tisk otvaracieho riadkoveho elementu
		print OUTPUT " index=\"",$index++, "\"" if (defined($index));
		print OUTPUT ">";
		if ((scalar(@field) == $col_number) or defined($e_rec)) {
			# spracovanie jedneho riadku vstupu
			&handle_row(\@col_elements, \@field, $recovery, $m_value); 
		}
		else { 
			&ExitProgram ($INVALID_COLUMNS); 
		}
		print OUTPUT "</$line_elem>\n";
	}
	else { 
		&ExitProgram ($WRONG_FF); 
	}
}

# kontrola posledneho riadku na \r\n
if ($crlf == 1) {
	&ExitProgram($WRONG_FF);
}

if (defined($root_elem)) {
	print OUTPUT "</$root_elem>\n";
}

close INPUT;
close OUTPUT;
exit $ALL_OK;

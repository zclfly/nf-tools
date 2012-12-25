#use 5.010001;
use 5.000001;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'Net::NfDump',
    VERSION_FROM      => 'lib/Net/NfDump.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/Net/NfDump.pm', # retrieve abstract from module
       AUTHOR         => 'Tomas Podermanski <tpoder@cis.vutbr.cz>') : ()),
    LIBS              => [''], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I.', # e.g., '-I. -I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    OBJECT            => '$(O_FILES)', # link all the C files too
    MYEXTLIB          => 'libnf/libnf$(LIB_EXT)',
    NORECURS          => 0,
    clean             => {FILES => "nfdump/ nfdump"},
    realclean         => {FILES => "nfdump-*.tar.gz"},
);
if  (eval {require ExtUtils::Constant; 1}) {
  # If you edit these definitions to change the constants used by this module,
  # you will need to use the generated const-c.inc and const-xs.inc
  # files to replace their "fallback" counterparts before distributing your
  # changes.
  my @names = (qw());
  ExtUtils::Constant::WriteConstants(
                                     NAME         => 'Net::NfDump',
                                     NAMES        => \@names,
                                     DEFAULT_TYPE => 'IV',
                                     C_FILE       => 'const-c.inc',
                                     XS_FILE      => 'const-xs.inc',
                                  );

}
else {
  use File::Copy;
  use File::Spec;
  foreach my $file ('const-c.inc', 'const-xs.inc') {
    my $fallback = File::Spec->catfile('fallback', $file);
    copy ($fallback, $file) or die "Can't copy $fallback to $file: $!";
  }
}

sub MY::postamble {
'
NFDUMP		= nfdump-1.6.8p1
NFDUMP_MD5	= 853d917527d1a20837a899ffd88b736c
NFDUMP_SRC	= $(NFDUMP).tar.gz
NFDUMP_URL	= http://downloads.sourceforge.net/project/nfdump/stable/$(NFDUMP)/$(NFDUMP_SRC)

$(NFDUMP).tar.gz:
		@echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
		@echo "!!                                                     !!"
		@echo "!!  If automatic download fails please get nfdump      !!"
		@echo "!!  sources manually                                   !!"
		@echo "!!                                                     !!"
		@echo "!!  VERSION: $(NFDUMP)                            !!"
		@echo "!!  URL:     http://sourceforge.net/projects/nfdump/   !!"
		@echo "!!  FILE:    $(NFDUMP).tar.gz                     !!"
		@echo "!!                                                     !!"
		@echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
		wget -nv $(NFDUMP_URL) || curl -L -o $(NFDUMP_SRC) $(NFDUMP_URL)


nfdump/bin/nfdump.o: $(NFDUMP).tar.gz 
		./md5sum.sh $(NFDUMP_MD5) $(NFDUMP).tar.gz 
		tar xzf $(NFDUMP).tar.gz
		mv -f $(NFDUMP) nfdump 
		cd nfdump && export CFLAGS=-fPIC && ./configure  && make

$(MYEXTLIB): libnf/Makefile nfdump/bin/nfdump.o
		cd libnf && $(MAKE) $(PASSTHRU)
';
}


BEGIN {
  unless ($ENV{AUTHOR_TESTING}) {
    require Test::More;
    Test::More::plan(skip_all => 'these tests are for testing by the author');
  }
}

use strict;
use warnings;
use Test::More;

# generated by Dist::Zilla::Plugin::Test::PodSpelling 2.006008
use Test::Spelling 0.12;
use Pod::Wordlist;


add_stopwords(<DATA>);
all_pod_files_spelling_ok( qw( bin lib  ) );
__DATA__
Alders
Alders'
Eilam
Eilam's
MaxMind
MaxMind's
Oschwald
Oschwald's
Rolsky
Rolsky's
GeoIP
MMDB
TW
Teredo
uint
zh
Olaf
oalders
Greg
goschwald
Dave
drolsky
Inc
tjmather
lib
DB
Writer
Tree
Processor
VisualizeTree
Serializer
Util
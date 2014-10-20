
BEGIN {
  unless ($ENV{AUTHOR_TESTING}) {
    require Test::More;
    Test::More::plan(skip_all => 'these tests are for testing by the author');
  }
}

use strict;
use warnings;

# this test was generated with Dist::Zilla::Plugin::Test::NoTabs 0.09

use Test::More 0.88;
use Test::NoTabs;

my @files = (
    'lib/MaxMind/DB/Writer.pm',
    'lib/MaxMind/DB/Writer/Serializer.pm',
    'lib/MaxMind/DB/Writer/Tree.pm',
    'lib/MaxMind/DB/Writer/Tree/Processor/VisualizeTree.pm',
    'lib/MaxMind/DB/Writer/Util.pm',
    't/MaxMind/DB/Writer/Serializer-deduplication.t',
    't/MaxMind/DB/Writer/Serializer-large-pointer.t',
    't/MaxMind/DB/Writer/Serializer-types/array.t',
    't/MaxMind/DB/Writer/Serializer-types/boolean.t',
    't/MaxMind/DB/Writer/Serializer-types/bytes.t',
    't/MaxMind/DB/Writer/Serializer-types/double.t',
    't/MaxMind/DB/Writer/Serializer-types/end_marker.t',
    't/MaxMind/DB/Writer/Serializer-types/float.t',
    't/MaxMind/DB/Writer/Serializer-types/int32.t',
    't/MaxMind/DB/Writer/Serializer-types/map.t',
    't/MaxMind/DB/Writer/Serializer-types/pointer.t',
    't/MaxMind/DB/Writer/Serializer-types/uint128.t',
    't/MaxMind/DB/Writer/Serializer-types/uint16.t',
    't/MaxMind/DB/Writer/Serializer-types/uint32.t',
    't/MaxMind/DB/Writer/Serializer-types/uint64.t',
    't/MaxMind/DB/Writer/Serializer-types/utf8_string.t',
    't/MaxMind/DB/Writer/Serializer-utf8-as-bytes.t',
    't/MaxMind/DB/Writer/Serializer-utf8-round-trip.t',
    't/MaxMind/DB/Writer/Serializer.t',
    't/MaxMind/DB/Writer/Tree-bigint.t',
    't/MaxMind/DB/Writer/Tree-ipv4-and-6.t',
    't/MaxMind/DB/Writer/Tree-ipv6-aliases.t',
    't/MaxMind/DB/Writer/Tree-iterator-large-dataset.t',
    't/MaxMind/DB/Writer/Tree-iterator.t',
    't/MaxMind/DB/Writer/Tree-output/0.0.0.0.t',
    't/MaxMind/DB/Writer/Tree-output/basic.t',
    't/MaxMind/DB/Writer/Tree-output/ipv6-aliases.t',
    't/MaxMind/DB/Writer/Tree-output/record-deduplication.t',
    't/MaxMind/DB/Writer/Tree-output/utf8-data.t',
    't/MaxMind/DB/Writer/Tree-output/verifies.t',
    't/MaxMind/DB/Writer/Tree-record-collisions.t',
    't/MaxMind/DB/Writer/Tree.t',
    't/author-no-tabs.t',
    't/lib/Test/MaxMind/DB/Writer.pm',
    't/lib/Test/MaxMind/DB/Writer/Iterator.pm',
    't/lib/Test/MaxMind/DB/Writer/Serializer.pm',
    't/release-cpan-changes.t',
    't/release-eol.t',
    't/release-pod-linkcheck.t',
    't/release-pod-no404s.t',
    't/release-pod-syntax.t',
    't/test-data/geolite2-sample.json'
);

notabs_ok($_) foreach @files;
done_testing;

use strict;
use warnings;

use lib 't/lib';

use Test::Fatal;
use Test::MaxMind::DB::Common::Data qw( test_cases_for );
use Test::MaxMind::DB::Writer::Serializer qw( test_encoding_of_type );
use Test::More;

use MaxMind::DB::Writer::Serializer;

test_encoding_of_type( map => test_cases_for('map') );

{
    my $serializer = MaxMind::DB::Writer::Serializer->new();

    like(
        exception { $serializer->_type_for_key('bad key') },
        qr/\QCould not determine the type for map key "bad key"/,
        'cannot guess the type for an unknown hash key'
    );
}

done_testing();

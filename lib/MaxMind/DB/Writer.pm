package MaxMind::DB::Writer;
# git description: v0.050007-12-gc68712d
$MaxMind::DB::Writer::VERSION = '0.060000';

use strict;
use warnings;

1;

# ABSTRACT: Create MaxMind DB database files

__END__

=pod

=head1 NAME

MaxMind::DB::Writer - Create MaxMind DB database files

=head1 VERSION

version 0.060000

=head1 SYNOPSIS

    use MaxMind::DB::Writer::Tree;
    use Net::Works::Network;

    my %types = (
        color => 'utf8_string',
        dogs  => [ 'array', 'utf8_string' ],
        size  => 'uint16',
    );

    my $tree = MaxMind::DB::Writer::Tree->new(
        ip_version            => 6,
        record_size           => 24,
        database_type         => 'My-IP-Data',
        languages             => ['en'],
        description           => { en => 'My database of IP data' },
        map_key_type_callback => sub { $types{ $_[0] } },
    );

    my $network
        = Net::Works::Network->new_from_string( string => '8.23.0.0/16' );

    $tree->insert_network(
        $network,
        {
            color => 'blue',
            dogs  => [ 'Fido', 'Ms. Pretty Paws' ],
            size  => 42,
        },
    );

    open my $fh, '>:bytes', '/path/to/my-ip-data.mmdb';
    $tree->write_tree($fh);

=head1 DESCRIPTION

This distribution contains the code necessary to write L<MaxMind DB database
files|http://maxmind.github.io/MaxMind-DB/>. See L<MaxMind::DB::Writer::Tree>
for API docs.

=head1 WINDOWS SUPPORT

This distribution does not currently work on Windows. Reasonable patches for
Windows support are very welcome. You will probably need to start by making
L<Math::Int128> work on Windows, since we use that module's C API for dealing
with 128-bit integers to represent IPv6 addresses numerically.

=head1 SUPPORT

Please report all issues with this code using the GitHub issue tracker at
L<https://github.com/maxmind/MaxMind-DB-Writer-perl/issues>.

We welcome patches as pull requests against our GitHub repository at
L<https://github.com/maxmind/MaxMind-DB-Writer-perl>.

=head1 AUTHORS

=over 4

=item *

Olaf Alders <oalders@maxmind.com>

=item *

Greg Oschwald <goschwald@maxmind.com>

=item *

Dave Rolsky <drolsky@maxmind.com>

=back

=head1 CONTRIBUTOR

=for stopwords tjmather

tjmather <tjmather@maxmind.com>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2014 by MaxMind, Inc..

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut

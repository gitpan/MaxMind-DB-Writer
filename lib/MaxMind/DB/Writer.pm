package MaxMind::DB::Writer;
$MaxMind::DB::Writer::VERSION = '0.050004';
use strict;
use warnings;

1;

# ABSTRACT: Create MaxMind DB database files

__END__

=pod

=encoding UTF-8

=head1 NAME

MaxMind::DB::Writer - Create MaxMind DB database files

=head1 VERSION

version 0.050004

=head1 SYNOPSIS

    use MaxMind::DB::Writer::Tree;
    use Net::Works::Network;

    my $tree = MaxMind::DB::Writer::Tree->new(
        ip_version    => 6,
        record_size   => 24,
        database_type => 'My-IP-Data',
        languages     => ['en'],
        description   => { en => 'My database of IP data' },
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

    open my $fh, '>:raw', '/path/to/my-ip-data.mmdb';
    $tree->write_tree($fh);

=head1 DESCRIPTION

This distribution contains the code necessary to write L<MaxMind DB database
files|http://maxmind.github.io/MaxMind-DB/>. See L<MaxMind::DB::Writer::Tree>
for API docs.

=head1 AUTHORS

=over 4

=item *

Dave Rolsky <autarch@urth.org>

=item *

Olaf Alders <olaf@wundercounter.com>

=back

=head1 COPYRIGHT AND LICENSE

This software is Copyright (c) 2014 by MaxMind, Inc..

This is free software, licensed under:

  The Artistic License 2.0 (GPL Compatible)

=cut

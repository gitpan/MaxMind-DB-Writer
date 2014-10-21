# NAME

MaxMind::DB::Writer - Create MaxMind DB database files

# VERSION

version 0.050006

# SYNOPSIS

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

# DESCRIPTION

This distribution contains the code necessary to write [MaxMind DB database
files](http://maxmind.github.io/MaxMind-DB/). See [MaxMind::DB::Writer::Tree](https://metacpan.org/pod/MaxMind::DB::Writer::Tree)
for API docs.

# WINDOWS SUPPORT

This distribution does not currently work on Windows. Reasonable patches for
Windows support are very welcome. You will probably need to start by making
[Math::Int128](https://metacpan.org/pod/Math::Int128) work on Windows, since we use that module's C API for dealing
with 128-bit integers to represent IPv6 addresses numerically.

# SUPPORT

Please report all issues with this code using the GitHub issue tracker at
[https://github.com/maxmind/MaxMind-DB-Writer-perl/issues](https://github.com/maxmind/MaxMind-DB-Writer-perl/issues).

We welcome patches as pull requests against our GitHub repository at
[https://github.com/maxmind/MaxMind-DB-Writer-perl](https://github.com/maxmind/MaxMind-DB-Writer-perl).

# AUTHORS

- Olaf Alders <oalders@maxmind.com>
- Greg Oschwald <goschwald@maxmind.com>
- Dave Rolsky <drolsky@maxmind.com>

# COPYRIGHT AND LICENSE

This software is Copyright (c) 2014 by MaxMind, Inc..

This is free software, licensed under:

    The Artistic License 2.0 (GPL Compatible)

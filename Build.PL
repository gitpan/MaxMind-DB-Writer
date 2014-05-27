use strict;
use warnings;

use lib 'inc';

use Config::AutoConf;
use Module::Build;

my $mb = Module::Build->new(
    _mb_args(),
    c_source => 'c',
);

$mb->extra_compiler_flags(
    @{ $mb->extra_compiler_flags },
    qw(-std=c99 -fms-extensions -Wall -g)
);

_check_c_prereqs($mb);

$mb->create_build_script();

sub _mb_args {

    # This is a funky way of allowing this code to run stand-alone and as a
    # template to be processes by Dist::Zilla::Plugin::ModuleBuild::Custom
    my $data = do { local $/; <DATA> };

    if ( $data !~ /^\#/ ) {
        my $args = eval $data;
        return %{$args};
    }
    else {
        return (
            module_name          => 'MaxMind::DB::Writer',
            dist_abstract        => 'Whatever',
            dist_version         => '42',
            license              => 'perl',
            recursive_test_files => 1,
        );
    }
}

sub _check_c_prereqs {
    my $mb = shift;

    my @include_dirs = map { my $dir = $_; $dir =~ s/^-I//; $dir }
        grep { /^-I/ } @{ $mb->extra_compiler_flags() || [] };
    my @lib_dirs = grep { /^-L/ } @{ $mb->extra_linker_flags() || [] };

    my $ac = Config::AutoConf->new(
        extra_include_dirs => \@include_dirs,
        extra_link_flags   => \@lib_dirs,
    );

    if ( $ac->check_type('unsigned __int128') ) {
        1;
    }
    elsif ( $ac->check_type('unsigned int __attribute__ ((__mode__ (TI)))') )
    {
        $mb->extra_compiler_flags(
            @{ $mb->extra_compiler_flags || [] },
            '-DMMDBW_UINT128_USING_MODE',
        );
    }
    elsif ( !$ac->check_type('unsigned __int128') ) {
        warn <<'EOF';

  It looks like your compiler doesn't support the "unsigned __int128" or
  "unsigned int __attribute__ ((__mode__ (TI)))" types. One of these types is
  necessary to compile the MaxMind::DB::Reader::XS module.

EOF

        exit 1;
    }

    return;
}

__DATA__
 {
          'dist_abstract' => 'Create MaxMind DB database files',
          'test_requires' => {
                               'Scalar::Util' => '0',
                               'Test::Requires' => '0',
                               'Net::Works::Address' => '0',
                               'List::AllUtils' => '0',
                               'Test::Fatal' => '0',
                               'utf8' => '0',
                               'Test::Bits' => '0',
                               'List::Util' => '0',
                               'Test::More' => '0.88',
                               'Test::MaxMind::DB::Common::Data' => '0',
                               'File::Temp' => '0',
                               'autodie' => '0',
                               'lib' => '0'
                             },
          'script_files' => [],
          'recursive_test_files' => 1,
          'license' => 'artistic_2',
          'build_requires' => {
                                'Module::Build' => '0.3601'
                              },
          'dist_name' => 'MaxMind-DB-Writer',
          'dist_author' => [
                             'Dave Rolsky <autarch@urth.org>',
                             'Olaf Alders <olaf@wundercounter.com>'
                           ],
          'module_name' => 'MaxMind::DB::Writer',
          'requires' => {
                          'bytes' => '0',
                          'XSLoader' => '0',
                          'Net::Works::Network' => '0.16',
                          'IO::Handle' => '0',
                          'MaxMind::DB::Metadata' => '0',
                          'Sereal::Encoder' => '0',
                          'Moose::Util::TypeConstraints' => '0',
                          'Encode' => '0',
                          'Net::Works' => '0.16',
                          'Data::Dumper::Concise' => '0',
                          'Data::IEEE754' => '0',
                          'MaxMind::DB::Common' => '0.031003',
                          'MooseX::StrictConstructor' => '0',
                          'Digest::MD5' => '0',
                          'Moose' => '0',
                          'warnings' => '0',
                          'namespace::autoclean' => '0',
                          'Digest::SHA1' => '0',
                          'Devel::Dwarn' => '0',
                          'MaxMind::DB::Reader::Decoder' => '0',
                          'constant' => '0',
                          'Exporter' => '0',
                          'Carp' => '0',
                          'strict' => '0',
                          'Math::Int128' => '0.06',
                          'Test::Deep::NoTest' => '0',
                          'MaxMind::DB::Role::Debugs' => '0'
                        },
          'recommends' => {},
          'configure_requires' => {
                                    'Module::Build' => '0.3601'
                                  },
          'dist_version' => '0.050004'
        };


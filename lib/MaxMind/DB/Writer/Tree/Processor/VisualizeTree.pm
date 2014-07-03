package MaxMind::DB::Writer::Tree::Processor::VisualizeTree;
$MaxMind::DB::Writer::Tree::Processor::VisualizeTree::VERSION = '0.050005';
use strict;
use warnings;

use Data::Dumper::Concise;
use Digest::MD5 qw( md5_hex );
use GraphViz2;
use Net::Works::Network 0.16;

use Moose;

has ip_version => (
    is       => 'ro',
    isa      => 'Int',
    required => 1,
);

has show_full_data_record => (
    is      => 'ro',
    isa     => 'Bool',
    default => 0,
);

has graph => (
    is       => 'ro',
    isa      => 'GraphViz2',
    init_arg => undef,
    default  => sub { GraphViz2->new( global => { directed => 1 } ) },
);

has _labels => (
    is       => 'ro',
    init_arg => undef,
    default  => sub { {} },
);

sub process_node_record {
    my $self               = shift;
    my $node_num           = shift;
    my $dir                = shift;
    my $node_ip_num        = shift;
    my $node_mask_length   = shift;
    my $record_ip_num      = shift;
    my $record_mask_length = shift;
    my $record_node_num    = shift;

    $self->graph()->add_edge(
        from => $self->_label_for_node(
            $node_num, $node_ip_num, $node_mask_length
        ),
        to => $self->_label_for_node(
            $record_node_num, $record_ip_num, $record_mask_length
        ),
        label => ( $dir ? 'RIGHT' : 'LEFT' ),
    );

    return 1;
}

sub process_empty_record {
    return;
}

sub process_data_record {
    my $self               = shift;
    my $node_num           = shift;
    my $dir                = shift;
    my $node_ip_num        = shift;
    my $node_mask_length   = shift;
    my $record_ip_num      = shift;
    my $record_mask_length = shift;
    my $value              = shift;

    $self->graph()->add_edge(
        from => $self->_label_for_node(
            $node_num, $node_ip_num, $node_mask_length
        ),
        to => $self->_network( $record_ip_num, $record_mask_length ) . ' = '
            . $self->_data_record_representation($value),
        label => ( $dir ? 'RIGHT' : 'LEFT' ),
    );

    return 1;
}

sub _label_for_node {
    my $self        = shift;
    my $node_num    = shift;
    my $ip_num      = shift;
    my $mask_length = shift;

    my $network = $self->_network( $ip_num, $mask_length );

    return $self->_labels()->{$node_num} //=
          "Node $node_num - "
        . $network->as_string() . ' ('
        . $network->first()->as_string . ' - '
        . $network->last()->as_string() . ')';
}

sub _data_record_representation {
    my $self  = shift;
    my $value = shift;

    if ( $self->show_full_data_record() ) {
        return quotemeta( Dumper($value) );
    }
    else {
        return md5_hex( Dumper($value) );
    }
}

sub _network {
    my $self        = shift;
    my $ip_num      = shift;
    my $mask_length = shift;

    return Net::Works::Network->new_from_integer(
        integer     => $ip_num,
        mask_length => $mask_length,
        version     => $self->ip_version(),
    );
}

__PACKAGE__->meta()->make_immutable();

1;
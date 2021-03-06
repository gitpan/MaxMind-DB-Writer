#include "tree.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>

#define LOCAL

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#define NETWORK_BIT_VALUE(network, current_bit)                    \
    (network)->bytes[((network)->max_depth0 - (current_bit)) >> 3] \
    & (1U << (~((network)->max_depth0 - (current_bit)) & 7))

/* This is also defined in MaxMind::DB::Common but we don't want to have to
 * fetch it every time we need it. */
#define DATA_SECTION_SEPARATOR_SIZE 16

#define SHA1_KEY_LENGTH 27

typedef struct encode_args_s {
    PerlIO *output_io;
    SV *root_data_type;
    SV *serializer;
    HV *data_pointer_cache;
} encode_args_s;

/* *INDENT-OFF* */
/* --prototypes automatically generated by dev-bin/regen-prototypes.pl - don't remove this comment */
LOCAL void insert_resolved_network(MMDBW_tree_s *tree, MMDBW_network_s *network,
                                   SV *key_sv, SV *data);
LOCAL const char *const store_data_in_tree(MMDBW_tree_s *tree, SV *key_sv,
                                           SV *data_sv);
LOCAL MMDBW_network_s resolve_network(MMDBW_tree_s *tree,
                                      const char *const ipstr,
                                      const uint8_t mask_length);
LOCAL const char *const network_as_string(const char *const ipstr,
                                          const uint8_t mask_length);
LOCAL void free_network(MMDBW_network_s *network);
LOCAL void delete_network(MMDBW_tree_s *tree, const char *const ipstr,
                          const uint8_t mask_length);
LOCAL bool tree_has_network(MMDBW_tree_s *tree, MMDBW_network_s *network);
LOCAL void insert_record_for_network(MMDBW_tree_s *tree,
                                     MMDBW_network_s *network,
                                     MMDBW_record_s *new_record,
                                     bool merge_record_collisions);
LOCAL bool merge_records(MMDBW_tree_s *tree,
                         MMDBW_network_s *network,
                         MMDBW_record_s *new_record,
                         MMDBW_record_s *record_to_set);
LOCAL void merge_hash(HV *from, HV *to);
LOCAL MMDBW_node_s *find_node_for_network(MMDBW_tree_s *tree,
                                          MMDBW_network_s *network,
                                          int *current_bit,
                                          MMDBW_node_s *(if_not_node)(
                                              MMDBW_tree_s *tree,
                                              MMDBW_record_s *record));
LOCAL MMDBW_node_s *return_null(
    MMDBW_tree_s *UNUSED(tree), MMDBW_record_s *UNUSED(record));
LOCAL MMDBW_node_s *make_next_node(MMDBW_tree_s *tree, MMDBW_record_s *record);
LOCAL void assign_node_numbers(MMDBW_tree_s *tree);
LOCAL void encode_node(MMDBW_tree_s *tree, MMDBW_node_s *node,
                       mmdbw_uint128_t UNUSED(network), uint8_t UNUSED(depth));
LOCAL uint32_t record_value_as_number(MMDBW_tree_s *tree,
                                      MMDBW_record_s *record,
                                      encode_args_s *args);
LOCAL void iterate_tree(MMDBW_tree_s *tree,
                        MMDBW_node_s *node,
                        mmdbw_uint128_t network,
                        const uint8_t depth,
                        bool depth_first,
                        void(callback) (MMDBW_tree_s *tree,
                                        MMDBW_node_s *node,
                                        mmdbw_uint128_t network,
                                        const uint8_t depth));
LOCAL void assign_node_number(MMDBW_tree_s *tree, MMDBW_node_s *node,
                              mmdbw_uint128_t UNUSED(network), uint8_t UNUSED(
                                  depth));
LOCAL SV *key_for_data(SV *data);
LOCAL void *checked_malloc(size_t size);
/* --prototypes end - don't remove this comment-- */
/* *INDENT-ON* */

MMDBW_tree_s *new_tree(const uint8_t ip_version, uint8_t record_size,
                       bool merge_record_collisions)
{
    MMDBW_tree_s *tree = checked_malloc(sizeof(MMDBW_tree_s));

    /* XXX - check for 4 or 6 */
    tree->ip_version = ip_version;
    /* XXX - check for 24, 28, or 32 */
    tree->record_size = record_size;
    tree->merge_record_collisions = merge_record_collisions;
    tree->data_table = NULL;
    tree->last_node = NULL;
    tree->is_finalized = false;
    tree->iteration_args = NULL;
    tree->root_node = new_node(tree);

    return tree;
}

int insert_network(MMDBW_tree_s *tree, const char *const ipstr,
                   const uint8_t mask_length, SV *key, SV *data)
{
    MMDBW_network_s network = resolve_network(tree, ipstr, mask_length);

    if (tree->ip_version == 4 && network.family == AF_INET6) {
        return -1;
    }

    insert_resolved_network(tree, &network, key, data);

    free_network(&network);

    return 0;
}

LOCAL void insert_resolved_network(MMDBW_tree_s *tree, MMDBW_network_s *network,
                                   SV *key_sv, SV *data)
{
    const char *const key = store_data_in_tree(tree, key_sv, data);
    MMDBW_record_s new_record = {
        .type    = MMDBW_RECORD_TYPE_DATA,
        .value   = {
            .key = key
        }
    };

    insert_record_for_network(tree, network, &new_record,
                              tree->merge_record_collisions);

    tree->is_finalized = 0;
}

LOCAL const char *const store_data_in_tree(MMDBW_tree_s *tree, SV *key_sv,
                                           SV *data_sv)
{
    const char *const key = SvPVbyte_nolen(key_sv);

    MMDBW_data_hash_s *data = NULL;
    HASH_FIND(hh, tree->data_table, key, SHA1_KEY_LENGTH, data);

    if (NULL == data) {
        data = checked_malloc(sizeof(MMDBW_data_hash_s));

        SvREFCNT_inc(data_sv);
        data->data_sv = data_sv;

        data->key = checked_malloc(SHA1_KEY_LENGTH + 1);
        strcpy((char *)data->key, key);

        HASH_ADD_KEYPTR(hh, tree->data_table, data->key, SHA1_KEY_LENGTH, data);
    }

    return data->key;
}

/* XXX - this is mostly copied from libmaxminddb - can we somehow share this code? */
LOCAL MMDBW_network_s resolve_network(MMDBW_tree_s *tree,
                                      const char *const ipstr,
                                      const uint8_t mask_length)
{
    struct addrinfo ai_hints;
    ai_hints.ai_socktype = 0;

    if (tree->ip_version == 6 || NULL != strchr(ipstr, ':')) {
        ai_hints.ai_flags = AI_NUMERICHOST | AI_V4MAPPED;
        ai_hints.ai_family = AF_INET6;
    } else {
        ai_hints.ai_flags = AI_NUMERICHOST;
        ai_hints.ai_family = AF_INET;
    }

    struct addrinfo *addresses;
    int status = getaddrinfo(ipstr, NULL, &ai_hints, &addresses);
    if (status) {
        croak("Bad IP address: %s - %s\n", ipstr, gai_strerror(status));
    }

    int family = addresses->ai_addr->sa_family;
    uint8_t *bytes;
    if (family == AF_INET) {
        bytes = checked_malloc(4);
        memcpy(bytes,
               &((struct sockaddr_in *)addresses->ai_addr)->sin_addr.s_addr,
               4);
    } else {
        bytes = checked_malloc(16);
        memcpy(
            bytes,
            ((struct sockaddr_in6 *)addresses->ai_addr)->sin6_addr.s6_addr,
            16);
    }

    MMDBW_network_s network = {
        .bytes          = bytes,
        .mask_length    = mask_length,
        .family         = addresses->ai_addr->sa_family,
        .max_depth0     = (family == AF_INET ? 31 : 127),
        .address_string = ipstr,
        .as_string      = network_as_string(ipstr,       mask_length)
    };

    freeaddrinfo(addresses);

    return network;
}

LOCAL const char *const network_as_string(const char *const ipstr,
                                          const uint8_t mask_length)
{
    /* 3 chars for up to 3 digits of netmask and 1 for the slash and 1 for the \0. */
    char *string = checked_malloc(strlen(ipstr) + 5);
    sprintf(string, "%s/%u", ipstr, mask_length);
    return (const char *)string;
}

LOCAL void free_network(MMDBW_network_s *network)
{
    free((char *)network->bytes);
    free((char *)network->as_string);
}

struct network {
    const char *const ipstr;
    const uint8_t mask_length;
};

static struct network ipv4_aliases[] = {
    {
        .ipstr = "::ffff:0:0",
        .mask_length = 96
    },
    {
        .ipstr = "2001::",
        .mask_length = 32
    },
    {
        .ipstr = "2002::",
        .mask_length = 16
    }
};

void alias_ipv4_networks(MMDBW_tree_s *tree)
{
    if (tree->ip_version == 4) {
        return;
    }

    MMDBW_network_s ipv4_root_network = resolve_network(tree, "::0.0.0.0", 96);

    int current_bit;
    MMDBW_node_s *ipv4_root_node_parent =
        find_node_for_network(tree, &ipv4_root_network, &current_bit,
                              &return_null);
    /* If the current_bit is not 32 then we found some node further up the
     * tree that would eventually lead to that network. This means that there
     * are no IPv4 addresses in the tree, so there's nothing to alias. */
    if (32 != current_bit) {
        return;
    }

    if (MMDBW_RECORD_TYPE_NODE != ipv4_root_node_parent->left_record.type) {
        return;
    }

    MMDBW_node_s *ipv4_root_node =
        ipv4_root_node_parent->left_record.value.node;
    for (int i = 0; i <= 2; i++) {
        MMDBW_network_s alias_network =
            resolve_network(tree, ipv4_aliases[i].ipstr,
                            ipv4_aliases[i].mask_length);

        int current_bit;
        MMDBW_node_s *last_node_for_alias = find_node_for_network(
            tree, &alias_network, &current_bit, &make_next_node);
        if (NETWORK_BIT_VALUE(&alias_network, current_bit)) {
            last_node_for_alias->right_record.type = MMDBW_RECORD_TYPE_ALIAS;
            last_node_for_alias->right_record.value.node = ipv4_root_node;
        } else {
            last_node_for_alias->left_record.type = MMDBW_RECORD_TYPE_ALIAS;
            last_node_for_alias->left_record.value.node = ipv4_root_node;
        }

        free_network(&alias_network);
    }

    free_network(&ipv4_root_network);
}

LOCAL void insert_record_for_network(MMDBW_tree_s *tree,
                                     MMDBW_network_s *network,
                                     MMDBW_record_s *new_record,
                                     bool merge_record_collisions)
{
    int current_bit;
    MMDBW_node_s *node_to_set =
        find_node_for_network(tree, network, &current_bit, &make_next_node);

    MMDBW_record_s *record_to_set, *other_record;
    if (NETWORK_BIT_VALUE(network, current_bit)) {
        record_to_set = &(node_to_set->right_record);
        other_record = &(node_to_set->left_record);
    } else {
        record_to_set = &(node_to_set->left_record);
        other_record = &(node_to_set->right_record);
    }

    if (merge_record_collisions &&
        MMDBW_RECORD_TYPE_DATA == new_record->type) {

        if (merge_records(tree, network, new_record, record_to_set)) {
            return;
        }
    }

    /* If this record we're about to insert is a data record, and the other
     * record in the node also has the same data, then we instead want to
     * insert a single data record in this node's parent. We do this by
     * inserting the new record for the parent network, which we can calculate
     * quite easily by subtracting 1 from this network's mask length. */
    if (MMDBW_RECORD_TYPE_DATA == new_record->type
        && MMDBW_RECORD_TYPE_DATA == other_record->type
        ) {

        const char *const new_key = new_record->value.key;
        const char *const other_key = other_record->value.key;

        if (strlen(new_key) == strlen(other_key)
            && 0 == strcmp(new_key, other_key)) {

            int bytes_length = network->family == AF_INET ? 4 : 16;
            uint8_t *bytes = checked_malloc(bytes_length);
            memcpy(bytes, network->bytes, bytes_length);

            uint8_t parent_mask_length = network->mask_length - 1;
            MMDBW_network_s parent_network = {
                .bytes          = bytes,
                .mask_length    = parent_mask_length,
                .max_depth0     = network->max_depth0,
                .family         = network->family,
                .address_string = network->address_string,
                .as_string      = network_as_string(
                    network->address_string, parent_mask_length)
            };

            /* We don't need to merge record collisions in this insert as
             * we have already merged the new record with the existing
             * record
             */
            insert_record_for_network(tree, &parent_network, new_record, false);
            free_network(&parent_network);
            return;
        }
    }

    record_to_set->type = new_record->type;
    if (MMDBW_RECORD_TYPE_DATA == new_record->type) {
        record_to_set->value.key = new_record->value.key;
    } else if (MMDBW_RECORD_TYPE_NODE == new_record->type ||
               MMDBW_RECORD_TYPE_ALIAS == new_record->type) {
        record_to_set->value.node = new_record->value.node;
    }

    return;
}

LOCAL bool merge_records(MMDBW_tree_s *tree,
                         MMDBW_network_s *network,
                         MMDBW_record_s *new_record,
                         MMDBW_record_s *record_to_set)
{
    if (MMDBW_RECORD_TYPE_NODE == record_to_set->type ||
        MMDBW_RECORD_TYPE_ALIAS == record_to_set->type) {

        if (network->mask_length > network->max_depth0) {
            croak("Something is very wrong. Mask length is too long.");
        }

        uint8_t new_mask_length = network->mask_length + 1;

        MMDBW_network_s left = {
            .bytes          = network->bytes,
            .mask_length    = new_mask_length,
            .max_depth0     = network->max_depth0,
            .family         = network->family,
            .address_string = network->address_string,
            .as_string      = network_as_string(
                network->address_string, new_mask_length)
        };

        MMDBW_record_s new_left_record = {
            .type    = new_record->type,
            .value   = {
                .key = new_record->value.key
            }
        };

        insert_record_for_network(tree, &left, &new_left_record, true);

        free((char *)left.as_string);

        int bytes_length = network->family == AF_INET ? 4 : 16;
        uint8_t right_bytes[bytes_length];
        memcpy(&right_bytes, network->bytes, bytes_length);

        right_bytes[ (new_mask_length - 1) / 8]
            |= 1 << ((network->max_depth0 + 1 - new_mask_length) % 8);

        char right_address_string[AF_INET ? INET_ADDRSTRLEN :
                                  INET6_ADDRSTRLEN];
        inet_ntop(network->family, &right_bytes, right_address_string,
                  sizeof(right_address_string));

        MMDBW_network_s right = {
            .bytes          = (const uint8_t *const)&right_bytes,
            .mask_length    = new_mask_length,
            .max_depth0     = network->max_depth0,
            .family         = network->family,
            .address_string = right_address_string,
            .as_string      = network_as_string(
                right_address_string, new_mask_length)
        };

        MMDBW_record_s new_right_record = {
            .type    = new_record->type,
            .value   = {
                .key = new_record->value.key
            }
        };

        insert_record_for_network(tree, &right, &new_right_record, true);

        free((char *)right.as_string);

        /* There's no need continuing with the original record as the relevant
         * data has already been inserted further down the tree by the code
         * above. */
        return true;
    }
    /* This must come before the node pruning code in
       insert_record_for_network, as we only want to prune nodes where the
       merged record matches. */
    else if (MMDBW_RECORD_TYPE_DATA == record_to_set->type) {
        SV *merged = merge_hashes_for_keys(tree,
                                           record_to_set->value.key,
                                           new_record->value.key,
                                           network);
        SV *key_sv = key_for_data(merged);
        const char *const new_key = store_data_in_tree(tree, key_sv, merged);
        SvREFCNT_dec(key_sv);

        new_record->value.key = new_key;
    }

    return false;
}

SV *merge_hashes_for_keys(MMDBW_tree_s *tree, const char *const key_from,
                          const char *const key_into, MMDBW_network_s *network)
{
    SV *data_from = data_for_key(tree, key_from);
    SV *data_into = data_for_key(tree, key_into);

    if (!(SvROK(data_from) && SvROK(data_into)
          && SvTYPE(SvRV(data_from)) == SVt_PVHV
          && SvTYPE(SvRV(data_into)) == SVt_PVHV)) {
        croak(
            "Cannot merge data records unless both records are hashes - inserting %s",
            network->as_string);
    }

    HV *hash_from = (HV *)SvRV(data_from);
    HV *hash_into = (HV *)SvRV(data_into);
    HV *hash_new = newHV();

    merge_hash(hash_into, hash_new);
    merge_hash(hash_from, hash_new);

    SV *merged_ref = newRV_inc((SV *)hash_new);
    /* We aren't keeping the original HV * around so we decrement its ref
     * count. */
    SvREFCNT_dec(hash_new);

    return merged_ref;
}

LOCAL void merge_hash(HV *from, HV *to)
{
    (void)hv_iterinit(from);
    HE *he;
    while (NULL != (he = hv_iternext(from))) {
        STRLEN key_length;
        const char *const key = HePV(he, key_length);
        U32 hash = HeHASH(he);
        if (hv_exists(to, key, key_length)) {
            continue;
        }

        SV *value = HeVAL(he);
        SvREFCNT_inc(value);
        (void)hv_store(to, key, key_length, value, hash);
    }

    return;
}

SV *lookup_ip_address(MMDBW_tree_s *tree, const char *const ipstr)
{
    MMDBW_network_s network =
        resolve_network(tree, ipstr, tree->ip_version == 6 ? 128 : 32);

    int current_bit;
    MMDBW_node_s *node_for_address =
        find_node_for_network(tree, &network, &current_bit, &return_null);

    MMDBW_record_s record_for_address;
    if (NETWORK_BIT_VALUE(&network, current_bit)) {
        record_for_address = node_for_address->right_record;
    } else {
        record_for_address = node_for_address->left_record;
    }

    if (MMDBW_RECORD_TYPE_NODE == record_for_address.type ||
        MMDBW_RECORD_TYPE_ALIAS == record_for_address.type) {
        croak(
            "WTF - found a node or alias record for an address lookup - %s - current_bit = %i",
            ipstr, current_bit);
        return &PL_sv_undef;
    } else if (MMDBW_RECORD_TYPE_EMPTY == record_for_address.type) {
        return &PL_sv_undef;
    } else {
        return newSVsv(data_for_key(tree, record_for_address.value.key));
    }
}

LOCAL MMDBW_node_s *find_node_for_network(MMDBW_tree_s *tree,
                                          MMDBW_network_s *network,
                                          int *current_bit,
                                          MMDBW_node_s *(if_not_node)(
                                              MMDBW_tree_s *tree,
                                              MMDBW_record_s *record))
{
    MMDBW_node_s *node = tree->root_node;
    uint8_t last_bit = network->max_depth0 - (network->mask_length - 1);

    for (*current_bit = network->max_depth0; *current_bit > last_bit;
         (*current_bit)--) {

        int next_is_right = NETWORK_BIT_VALUE(network, *current_bit);
        MMDBW_record_s *record =
            next_is_right
            ? &(node->right_record)
            : &(node->left_record);

        MMDBW_node_s *next_node;
        if (MMDBW_RECORD_TYPE_NODE == record->type ||
            MMDBW_RECORD_TYPE_ALIAS == record->type) {
            next_node = record->value.node;
        } else {
            next_node = if_not_node(tree, record);
            if (NULL == next_node) {
                return node;
            }

            if (next_is_right) {
                record->type = MMDBW_RECORD_TYPE_NODE;
                record->value.node = next_node;
            } else {
                record->type = MMDBW_RECORD_TYPE_NODE;
                record->value.node = next_node;
            }
        }

        node = next_node;
    }

    return node;
}

LOCAL MMDBW_node_s *return_null(
    MMDBW_tree_s *UNUSED(tree), MMDBW_record_s *UNUSED(record))
{
    return NULL;
}

LOCAL MMDBW_node_s *make_next_node(MMDBW_tree_s *tree, MMDBW_record_s *record)
{
    MMDBW_node_s *next_node = new_node(tree);
    if (MMDBW_RECORD_TYPE_DATA == record->type) {
        next_node->left_record.type = MMDBW_RECORD_TYPE_DATA;
        next_node->left_record.value.key = record->value.key;
        next_node->right_record.type = MMDBW_RECORD_TYPE_DATA;
        next_node->right_record.value.key = record->value.key;
    }

    return next_node;
}

MMDBW_node_s *new_node(MMDBW_tree_s *tree)
{
    MMDBW_node_s *node = checked_malloc(sizeof(MMDBW_node_s));

    node->number = 0;
    node->left_record.type = node->right_record.type = MMDBW_RECORD_TYPE_EMPTY;
    node->next_node = NULL;

    if (NULL != tree->last_node) {
        tree->last_node->next_node = node;
    }
    tree->last_node = node;

    return node;
}

void finalize_tree(MMDBW_tree_s *tree)
{
    if (tree->is_finalized) {
        return;
    }

    assign_node_numbers(tree);
    tree->is_finalized = true;
    return;
}

LOCAL void assign_node_numbers(MMDBW_tree_s *tree)
{
    tree->node_count = 0;
    start_iteration(tree, false, &assign_node_number);
}

void write_search_tree(MMDBW_tree_s *tree, SV *output, const bool alias_ipv6,
                       SV *root_data_type, SV *serializer)
{
    if (alias_ipv6) {
        alias_ipv4_networks(tree);
    }

    finalize_tree(tree);

    /* This is a gross way to get around the fact that with C function
     * pointers we can't easily pass different params to different
     * callbacks. */
    encode_args_s args = {
        .output_io          = IoOFP(sv_2io(output)),
        .root_data_type     = root_data_type,
        .serializer         = serializer,
        .data_pointer_cache = newHV()
    };

    tree->iteration_args = (void *)&args;
    start_iteration(tree, false, &encode_node);
    tree->iteration_args = NULL;

    /* When the hash is _freed_, Perl decrements the ref count for each value
     * so we don't need to mess with them. */
    SvREFCNT_dec((SV *)args.data_pointer_cache);

    return;
}

LOCAL void encode_node(MMDBW_tree_s *tree, MMDBW_node_s *node,
                       mmdbw_uint128_t UNUSED(network), uint8_t UNUSED(depth))
{
    encode_args_s *args = (encode_args_s *)tree->iteration_args;

    uint32_t left =
        htonl(record_value_as_number(tree, &(node->left_record), args));
    uint32_t right =
        htonl(record_value_as_number(tree, &(node->right_record), args));

    uint8_t *left_bytes = (uint8_t *)&left;
    uint8_t *right_bytes = (uint8_t *)&right;

    if (24 == tree->record_size) {
        PerlIO_printf(args->output_io, "%c%c%c%c%c%c",
                      left_bytes[1], left_bytes[2], left_bytes[3],
                      right_bytes[1], right_bytes[2], right_bytes[3]);
    } else if (28 == tree->record_size) {
        PerlIO_printf(args->output_io, "%c%c%c%c%c%c%c",
                      left_bytes[1], left_bytes[2],
                      left_bytes[3],
                      (left_bytes[0] <<
                       4) | (right_bytes[0] & 15),
                      right_bytes[1], right_bytes[2],
                      right_bytes[3]);
    } else {
        PerlIO_printf(args->output_io, "%c%c%c%c%c%c%c%c",
                      left_bytes[0], left_bytes[1],
                      left_bytes[2], left_bytes[3],
                      right_bytes[0], right_bytes[1],
                      right_bytes[2], right_bytes[3]);
    }
}

LOCAL uint32_t record_value_as_number(MMDBW_tree_s *tree,
                                      MMDBW_record_s *record,
                                      encode_args_s *args)
{
    uint32_t record_value;

    if (MMDBW_RECORD_TYPE_EMPTY == record->type) {
        record_value = tree->node_count;
    } else if (MMDBW_RECORD_TYPE_NODE == record->type ||
               MMDBW_RECORD_TYPE_ALIAS == record->type) {
        record_value = record->value.node->number;
    } else {
        SV **cache_record =
            hv_fetch(args->data_pointer_cache, record->value.key,
                     SHA1_KEY_LENGTH, 0);
        if (cache_record) {
            // It is ok to return this without the size check below as it
            // would have already croaked when it was inserted if it was too
            // big.
            return SvIV(*cache_record);
        }

        dSP;
        ENTER;
        SAVETMPS;

        PUSHMARK(SP);
        EXTEND(SP, 5);
        PUSHs(args->serializer);
        PUSHs(args->root_data_type);
        PUSHs(sv_2mortal(newSVsv(data_for_key(tree, record->value.key))));
        PUSHs(&PL_sv_undef);
        PUSHs(sv_2mortal(newSVpvn(record->value.key, strlen(record->value.key))));
        PUTBACK;

        int count = call_method("store_data", G_SCALAR);

        SPAGAIN;

        if (count != 1) {
            croak("Expected 1 item back from ->store_data() call");
        }

        SV *rval = POPs;
        if (!(SvIOK(rval) || SvUOK(rval))) {
            croak(
                "The serializer's store_data() method returned an SV which is not SvIOK or SvUOK!");
        }
        uint32_t position = (uint32_t )SvUV(rval);

        PUTBACK;
        FREETMPS;
        LEAVE;

        record_value = position + tree->node_count +
                       DATA_SECTION_SEPARATOR_SIZE;

        SV *value = newSViv(record_value);
        (void)hv_store(args->data_pointer_cache, record->value.key,
                       SHA1_KEY_LENGTH, value, 0);
    }

    if (record_value > MAX_RECORD_VALUE(tree->record_size)) {
        croak("Node value of %u exceeds the record size of %u bits",
              record_value, tree->record_size);
    }

    return record_value;
}

/* We need to maintain a hash of already-seen nodes to handle the case of
 * trees with aliases. We don't want to go down the same branch more than
 * once. */
void start_iteration(MMDBW_tree_s *tree,
                     bool depth_first,
                     void(callback) (MMDBW_tree_s *tree,
                                     MMDBW_node_s *node,
                                     mmdbw_uint128_t network,
                                     uint8_t depth))
{
    mmdbw_uint128_t network = 0;
    uint8_t depth = 0;

    iterate_tree(tree, tree->root_node, network, depth, depth_first, callback);

    return;
}

LOCAL void iterate_tree(MMDBW_tree_s *tree,
                        MMDBW_node_s *node,
                        mmdbw_uint128_t network,
                        const uint8_t depth,
                        bool depth_first,
                        void(callback) (MMDBW_tree_s *tree,
                                        MMDBW_node_s *node,
                                        mmdbw_uint128_t network,
                                        const uint8_t depth))
{
    if (!depth_first) {
        callback(tree, node, network, depth);
    }

    const uint8_t max_depth0 = tree->ip_version == 6 ? 127 : 31;
    if (MMDBW_RECORD_TYPE_NODE == node->left_record.type) {
        iterate_tree(tree,
                     node->left_record.value.node,
                     network,
                     depth + 1,
                     depth_first,
                     callback);
    }

    if (depth_first) {
        callback(tree, node, network, depth);
    }

    if (MMDBW_RECORD_TYPE_NODE == node->right_record.type) {
        iterate_tree(tree,
                     node->right_record.value.node,
                     FLIP_NETWORK_BIT(network, max_depth0, depth),
                     depth + 1,
                     depth_first,
                     callback);
    }
}

LOCAL void assign_node_number(MMDBW_tree_s *tree, MMDBW_node_s *node,
                              mmdbw_uint128_t UNUSED(network), uint8_t UNUSED(
                                  depth))
{
    node->number = tree->node_count++;
    return;
}

LOCAL SV *key_for_data(SV *data)
{
    dSP;
    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    EXTEND(SP, 1);
    PUSHs(data);
    PUTBACK;

    const char *const sub = "MaxMind::DB::Writer::Util::key_for_data";
    int count = call_pv(sub, G_SCALAR);

    SPAGAIN;

    if (count != 1) {
        croak("Expected 1 item back from %s() call", sub);
    }

    SV *key = POPs;
    SvREFCNT_inc(key);

    PUTBACK;
    FREETMPS;
    LEAVE;

    return key;
}

SV *data_for_key(MMDBW_tree_s *tree, const char *const key)
{
    MMDBW_data_hash_s *data = NULL;
    HASH_FIND(hh, tree->data_table, key, strlen(key), data);

    if (NULL != data) {
        return data->data_sv;
    } else {
        return &PL_sv_undef;
    }
}

void free_tree(MMDBW_tree_s *tree)
{
    finalize_tree(tree);

    MMDBW_node_s *node = tree->root_node;
    while (NULL != node) {
        MMDBW_node_s *last_node = node;
        node = last_node->next_node;
        free(last_node);
    }

    MMDBW_data_hash_s *data, *tmp;
    HASH_ITER(hh, tree->data_table, data, tmp) {
        HASH_DEL(tree->data_table, data);
        SvREFCNT_dec(data->data_sv);
        free((char *)data->key);
        free(data);
    }

    free(tree);
}

const char *const record_type_name(int record_type)
{
    return MMDBW_RECORD_TYPE_EMPTY == record_type
           ? "empty"
           : MMDBW_RECORD_TYPE_NODE == record_type
           ? "node"
           : MMDBW_RECORD_TYPE_ALIAS == record_type
           ? "alias"
           : "data";
}

static SV *module;
void dwarn(SV *thing)
{
    if (NULL == module) {
        module = newSVpv("Devel::Dwarn", 0);
        load_module(PERL_LOADMOD_NOIMPORT, module, NULL);
    }

    dSP;
    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    EXTEND(SP, 1);
    PUSHs(thing);
    PUTBACK;

    (void)call_pv("Devel::Dwarn::Dwarn", G_VOID);

    SPAGAIN;

    PUTBACK;
    FREETMPS;
    LEAVE;
}

void warn_hex(uint8_t digest[16], char *where)
{
    char *hex = md5_as_hex(digest);
    fprintf(stderr, "MD5 = %s (%s)\n", hex, where);
    free(hex);
}

char *md5_as_hex(uint8_t digest[16])
{
    char *hex = checked_malloc(33);
    for (int i = 0; i < 16; ++i) {
        sprintf(&hex[i * 2], "%02x", digest[i]);
    }

    return hex;
}

LOCAL void *checked_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (NULL == ptr) {
        abort();
    }

    return ptr;
}

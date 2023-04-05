#include "./generator_double_fields.h"
#include "./card.h"
#include "./avl_tree.h"
#include "./io_utils.h"
#include "../testing_h/testing.h"
#include <string.h>

#define MSE_GENERATOR_FIELD_GENERATOR(fname) \
static int __mse_generate_set_##fname##_eq(mse_set_generator_t *gen, \
                                       avl_tree_node_t **res, \
                                       mtg_all_printings_cards_t *cards, \
                                       double arg) \
{ \
    /* Prepare the bounds of the search */ \
    mtg_card_t lower, upper; \
    memset(&lower, 0, sizeof(lower)); \
    upper = lower; \
 \
    lower.id = min_uuid(); \
    upper.id = max_uuid(); \
    lower.fname = upper.fname = arg; \
    /* Search the tree */ \
    ASSERT(tree_lookup_2(cards->indexes.card_##fname##_tree, res, (void *) &lower, (void *) &upper)); \
    return 1; \
} \
 \
static int __mse_generate_set_##fname##_lt(mse_set_generator_t *gen, \
                                       avl_tree_node_t **res, \
                                       mtg_all_printings_cards_t *cards, \
                                       double arg) \
{ \
    /* Prepare the bounds of the search */ \
    mtg_card_t lower; \
    memset(&lower, 0, sizeof(lower)); \
    lower.id = min_uuid(); \
 \
    lower.fname = arg; \
 \
    /* Search the tree */ \
    ASSERT(tree_lookup(cards->indexes.card_##fname##_tree, res, 1, (void *) &lower)); \
    return 1; \
} \
 \
static int __mse_generate_set_##fname##_lt_inc(mse_set_generator_t *gen, \
        avl_tree_node_t **res, \
        mtg_all_printings_cards_t *cards, \
        double arg) \
{ \
    /* Prepare the bounds of the search */ \
    mtg_card_t lower; \
    memset(&lower, 0xFF, sizeof(lower)); \
    lower.id = max_uuid(); \
 \
    lower.fname = arg; \
 \
    /* Search the tree */ \
    ASSERT(tree_lookup(cards->indexes.card_##fname##_tree, res, 1, (void *) &lower)); \
    return 1; \
} \
 \
static int __mse_generate_set_##fname##_gt(mse_set_generator_t *gen, \
                                       avl_tree_node_t **res, \
                                       mtg_all_printings_cards_t *cards, \
                                       double arg) \
{ \
    /* Prepare the bounds of the search */ \
    mtg_card_t lower; \
    memset(&lower, 0xFF, sizeof(lower)); \
    lower.id = max_uuid(); \
    lower.fname = arg; \
 \
    /* Search the tree */ \
    ASSERT(tree_lookup(cards->indexes.card_##fname##_tree, res, 0, (void *) &lower)); \
    return 1; \
} \
 \
static int __mse_generate_set_##fname##_gt_inc(mse_set_generator_t *gen, \
        avl_tree_node_t **res, \
        mtg_all_printings_cards_t *cards, \
        double arg) \
{ \
    /* Prepare the bounds of the search */ \
    mtg_card_t lower; \
    memset(&lower, 0, sizeof(lower)); \
    lower.id = min_uuid(); \
    lower.fname = arg; \
 \
    /* Search the tree */ \
    ASSERT(tree_lookup(cards->indexes.card_##fname##_tree, res, 0, (void *) &lower)); \
    return 1; \
} \
 \
int mse_generate_set_##fname(mse_set_generator_t *gen, \
                           avl_tree_node_t **res, \
                           mtg_all_printings_cards_t *cards) \
{ \
    double arg; \
    ASSERT(mse_to_double(gen->argument, &arg)); \
 \
    switch(gen->generator_op) { \
    case MSE_SET_GENERATOR_OP_INCLUDES: \
    case MSE_SET_GENERATOR_OP_EQUALS: \
        return __mse_generate_set_##fname##_eq(gen, res, cards, arg); \
    case MSE_SET_GENERATOR_OP_LT: \
        return __mse_generate_set_##fname##_lt(gen, res, cards, arg); \
    case MSE_SET_GENERATOR_OP_LT_INC: \
        return __mse_generate_set_##fname##_lt_inc(gen, res, cards, arg); \
    case MSE_SET_GENERATOR_OP_GT: \
        return __mse_generate_set_##fname##_gt(gen, res, cards, arg); \
    case MSE_SET_GENERATOR_OP_GT_INC: \
        return __mse_generate_set_##fname##_gt_inc(gen, res, cards, arg); \
    } \
 \
    lprintf(LOG_ERROR, "Cannot find operation %d for " #fname "\n", gen->generator_op); \
    return 0; \
}

MSE_GENERATOR_FIELD_GENERATOR(power)
MSE_GENERATOR_FIELD_GENERATOR(toughness)
MSE_GENERATOR_FIELD_GENERATOR(cmc)

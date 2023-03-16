#include "./test_card.h"
#include "../testing_h/testing.h"
#include "../src/card.h"
#include <unistd.h>
#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include <math.h>

#define ORACLE "First strike (This creature deals combat damage before creatures without first strike.)\nWhen Ancestor's Chosen comes into play, you gain 1 life for each card in your graveyard."

static int test_card_parse_json()
{
    FILE *f = fopen("./test_card.json", "r");
    ASSERT(f != NULL);
    json_error_t error;
    json_t *json = json_loadf(f, 0, &error);
    ASSERT(json != NULL);

    mtg_card_t card;
    ASSERT(parse_card_json(json, &card));

    ASSERT(card.name != NULL);
    ASSERT(strcmp(card.name, "Ancestor's Chosen") == 0);

    ASSERT(card.oracle_text != NULL);
    ASSERT(strcmp(card.oracle_text, ORACLE) == 0);

    ASSERT(card.colours == card.colour_identity);
    ASSERT(card.colours == MSE_WHITE);

    ASSERT(card.mana_cost != NULL);
    ASSERT(strcmp(card.mana_cost, "{5}{W}{W}") == 0);

    ASSERT(fabs(card.cmc - 7.0) < 0.01);

    free_card(&card);

    json_decref(json);
    return 1;
}

SUB_TEST(test_card, {&test_card_parse_json, "Test parse card from JSON"})

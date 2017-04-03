/*
 * Marvin - an UCI/XBoard compatible chess engine
 * Copyright (C) 2015 Martin Danielsson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#include "chess.h"

/* Different flags for transposition table entries */
enum {
    TT_EXACT,
    TT_BETA,
    TT_ALPHA,
    TT_PV
};

/*
 * Create the main transposition table.
 *
 * @param pos The board structure.
 * @param size The amount of memory to use for the table (in MB).
 */
void hash_tt_create_table(struct gamestate *pos, int size);

/*
 * Destroy the main transposition table.
 *
 * @param pos The board structure.
 */
void hash_tt_destroy_table(struct gamestate *pos);

/*
 * Clear the main transposition table.
 *
 * @param pos The board structure.
 */
void hash_tt_clear_table(struct gamestate *pos);

/*
 * Increase the age of the main transposition table.
 *
 * @param pos The board structure.
 */
void hash_tt_age_table(struct gamestate *pos);

/*
 * Store a new position in the main transposition table.
 *
 * @param pos The board structure.
 * @param move The best move found.
 * @param depth The depth to which the position was searched.
 * @param score The score for the position.
 * @param type The type of the score.
 */
void hash_tt_store(struct gamestate *pos, uint32_t move, int depth, int score,
                   int type);

/*
 * Lookup the current position in the main transposition table.
 *
 * @param pos The board structure.
 * @param depth The depth to which this position should be searched.
 * @param alpha The current alpha value.
 * @param beta The current beta value.
 * @param move Location to store the move at.
 * @param score Location to store the score at.
 * @return Returns true if the entry is good enough to trigger a cutoff.
 */
bool hash_tt_lookup(struct gamestate *pos, int depth, int alpha, int beta,
                    uint32_t *move, int *score);

/*
 * Make sure that the PV is present in the main transposition table.
 *
 * @param pos The board structure.
 * @param pv The PV to insert.
 */
void hash_tt_insert_pv(struct gamestate *pos, struct pv *pv);

/*
 * Create the pawn transposition table.
 *
 * @param pos The board structure.
 * @param size The amount of memory to use for the table (in MB).
 */
void hash_pawntt_create_table(struct gamestate *pos, int size);

/*
 * Destroy the pawn transposition table.
 *
 * @param pos The board structure.
 */
void hash_pawntt_destroy_table(struct gamestate *pos);

/*
 * Clear the pawn transposition table.
 *
 * @param pos The board structure.
 */
void hash_pawntt_clear_table(struct gamestate *pos);

/*
 * Initialize an item to store in the pawn transposition table.
 *
 * @param item The item to initialize.
 */
void hash_pawntt_init_item(struct pawntt_item *item);

/*
 * Store a new position in the pawn transposition table.
 *
 * @param pos The board structure.
 * @param item The item to store. Note that the pawn key field is ignored,
 *             the key in the board structure is always used.
 */
void hash_pawntt_store(struct gamestate *pos, struct pawntt_item *item);

/*
 * Lookup the current position in the pawn transposition table.
 *
 * @param pos The board structure.
 * @param item Location where the found item is stored.
 * @return Returns true if the position was found, false otherwise.
 */
 bool hash_pawntt_lookup(struct gamestate *pos, struct pawntt_item *item);

#endif
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
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "evalparams.h"
#include "validation.h"
#include "bitboard.h"
#include "hash.h"
#include "fen.h"
#include "utils.h"

/* Phase valuse for different piece types */
#define PAWN_PHASE      0
#define KNIGHT_PHASE    1
#define BISHOP_PHASE    1
#define ROOK_PHASE      2
#define QUEEN_PHASE     4

/*
 * Different evaluation components. The first two elements of each array
 * is the evaluation for each side, and the third contains the summarized
 * evaluation of the specific component (from white's pov).
 */
struct eval {
    bool in_pawntt;
    struct pawntt_item pawntt;
    bool endgame[2];
    int material[NPHASES][3];
    int material_adj[NPHASES][3];
    int psq[NPHASES][3];
    int pawn_structure[NPHASES][3];
    int king_safety[NPHASES][3];
    int positional[NPHASES][3];
    int mobility[NPHASES][3];
    int sum[NPHASES][3];
};

/*
 * Table with mobility scores for the different pieces. The table is
 * initialized by the eval_reset function.
 */
static int mobility_table[NPIECES];

/*
 * Table with scores for passed pawns based on rank. The table is
 * initialized by the eval_reset function.
 */
static int passed_pawn_scores[NRANKS];

/* Material values for all pieces */
int material_values[NPIECES] = {
    100,    /* White pawn */
    100,    /* Black pawn */
    325,    /* White knight */
    325,    /* Black knight */
    325,    /* White bishop */
    325,    /* Black bishop */
    500,    /* White rook */
    500,    /* Black rook */
    975,    /* White queen */
    975,    /* Black queen */
    20000,  /* White king */
    20000   /* Black king */
};

/*
 * Calculate a numerical value between 0 and 256 for
 * the current phase of the game. The formula is taken from
 * https://chessprogramming.wikispaces.com/Tapered+Eval
 */
static int calculate_game_phase(struct gamestate *pos)
{
    int total_phase;
    int phase;

    total_phase = 24;
    phase = total_phase;
    phase -= BITCOUNT(pos->bb_pieces[WHITE_KNIGHT]);
    phase -= BITCOUNT(pos->bb_pieces[BLACK_KNIGHT]);
    phase -= BITCOUNT(pos->bb_pieces[WHITE_BISHOP]);
    phase -= BITCOUNT(pos->bb_pieces[BLACK_BISHOP]);
    phase -= 2*BITCOUNT(pos->bb_pieces[WHITE_ROOK]);
    phase -= 2*BITCOUNT(pos->bb_pieces[BLACK_ROOK]);
    phase -= 4*BITCOUNT(pos->bb_pieces[WHITE_QUEEN]);
    phase -= 4*BITCOUNT(pos->bb_pieces[BLACK_QUEEN]);
    phase = (phase*256 + (total_phase/2))/total_phase;

    /*
     * Guard against negative phase values. The phase value might
     * become negative in case of promotion.
     */
    return MAX(phase, 0);
}

/*
 * Calculate a score that is an interpolation of the middlegame and endgame
 * based on the current phase of the game.
 */
static int calculate_tapered_eval(int phase, int score_mg, int score_eg)
{
    return ((score_mg*(256 - phase)) + (score_eg*phase))/256;
}

static void evaluate_pawn_shield(struct gamestate *pos,
                                 struct pawntt_item *item, int side)
{
    const uint64_t rank1[NSIDES] = {rank_mask[RANK_2], rank_mask[RANK_7]};
    const uint64_t rank2[NSIDES] = {rank_mask[RANK_3], rank_mask[RANK_6]};
    uint64_t pawns;
    uint64_t shield;
    int      score;

    pawns = pos->bb_pieces[PAWN+side];

    /* Queenside pawn shield */
    score = 0;
    shield = pawns&file_mask[FILE_A]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_A]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    shield = pawns&file_mask[FILE_B]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_B]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    shield = pawns&file_mask[FILE_C]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_C]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    if ((file_mask[FILE_A]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    if ((file_mask[FILE_B]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    if ((file_mask[FILE_C]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    item->pawn_shield[side][QUEENSIDE] = MAX(score, 0);

    /* Kingside pawn shield */
    score = 0;
    shield = pawns&file_mask[FILE_F]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_F]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    shield = pawns&file_mask[FILE_G]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_G]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    shield = pawns&file_mask[FILE_H]&rank1[side];
    if (shield != 0ULL) {
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK1;
    } else {
        shield = pawns&file_mask[FILE_H]&rank2[side];
        score += BITCOUNT(shield)*PAWN_SHIELD_RANK2;
    }
    if ((file_mask[FILE_F]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    if ((file_mask[FILE_G]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    if ((file_mask[FILE_H]&pos->bb_pieces[PAWN+side]) == 0ULL) {
        score += PAWN_SHIELD_HOLE;
    }
    item->pawn_shield[side][KINGSIDE] = MAX(score, 0);
}

/*
 * - double pawns
 * - isolated pawns
 * - passed pawns
 * - pawn shield
 */
static void evaluate_pawn_structure(struct gamestate *pos, struct eval *eval,
                                    int side)
{
    uint64_t            pieces;
    int                 sq;
    uint64_t            pawns;
    int                 k;
    int                 file;
    uint64_t            left;
    uint64_t            right;
    uint64_t            middle;
    int                 opp_side;
    int                 rank;
    uint64_t            blockers;
    struct pawntt_item  *item;

    item = &eval->pawntt;
    opp_side = FLIP_COLOR(side);
    pieces = pos->bb_pieces[PAWN+side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        file = FILENR(sq);
        rank = RANKNR(sq);

        /*
         * Look for isolated pawns. Isolated pawns are pawns
         * with no friendly pawns on the adjacent files.
         */
        left = (file!=FILE_A)?pos->bb_pieces[side+PAWN]&file_mask[file-1]:0ULL;
        right = (file!=FILE_H)?pos->bb_pieces[side+PAWN]&file_mask[file+1]:0ULL;
        if ((left == 0ULL) && (right == 0ULL)) {
            item->score[MIDDLEGAME][side] += ISOLATED_PAWN;
            item->score[ENDGAME][side] += ISOLATED_PAWN;
        }

        /*
         * Look for passed pawns. Passed pawns are pawns
         * with no opposing pawns on the same file or on
         * adjacent files.
         */
        left = (file!=FILE_A)?
                        pos->bb_pieces[opp_side+PAWN]&file_mask[file-1]:0ULL;
        right = (file!=FILE_H)?
                        pos->bb_pieces[opp_side+PAWN]&file_mask[file+1]:0ULL;
        middle = pos->bb_pieces[opp_side+PAWN]&file_mask[file];
        blockers = (left|middle|right)&ranks_ahead_mask[rank][side];
        if (blockers == 0ULL) {
            item->score[MIDDLEGAME][side] +=
                                    passed_pawn_scores[side==WHITE?rank:7-rank];
            item->score[ENDGAME][side] +=
                                    passed_pawn_scores[side==WHITE?rank:7-rank];
            SETBIT(item->passers[side], sq);
        }
    }

    /*
     * Look for double pawns. Double pawns is when a
     * player has two or more pawns on the same file.
     */
    for (k=0;k<NFILES;k++) {
        pawns = pos->bb_pieces[side+PAWN]&file_mask[k];
        if (BITCOUNT(pawns) >= 2) {
            item->score[MIDDLEGAME][side] += DOUBLE_PAWNS;
            item->score[ENDGAME][side] += DOUBLE_PAWNS;
        }
    }

    /*
     * Calculate a pawnshield score. This score will be used
     * later when evaluating king safety.
     */
    evaluate_pawn_shield(pos, item, side);
}

/*
 * - mobility
 */
static void evaluate_knights(struct gamestate *pos, struct eval *eval, int side)
{
    uint64_t pieces;
    uint64_t moves;
    int      sq;

    /* Calculate mobility */
    pieces = pos->bb_pieces[KNIGHT+side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        moves = bb_knight_moves(sq)&(~pos->bb_sides[side]);
        eval->mobility[MIDDLEGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[KNIGHT+side]);
        eval->mobility[ENDGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[KNIGHT+side]);
    }
}

/*
 * - bishop pair
 * - mobility
 */
static void evaluate_bishops(struct gamestate *pos, struct eval *eval, int side)
{
    uint64_t pieces;
    uint64_t moves;
    int      sq;

    /*
     * Check if both bishops are still on the board. To be correct
     * also check if the two (or more) bishops operate on different
     * color squares. The only case when a player can have
     * two bishops on the same color squares is if he underpromotes
     * to a bishop. This is so unlikely that it should be safe to assume
     * that the bishops operate on different color squares.
     */
    if (BITCOUNT(pos->bb_pieces[side+BISHOP]) >= 2) {
        eval->material_adj[MIDDLEGAME][side] += BISHOP_PAIR;
        eval->material_adj[ENDGAME][side] += BISHOP_PAIR;
    }

    /* Calculate mobility */
    pieces = pos->bb_pieces[BISHOP+side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        moves = bb_bishop_moves(pos->bb_all, sq)&(~pos->bb_sides[side]);
        eval->mobility[MIDDLEGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[BISHOP+side]);
        eval->mobility[ENDGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[BISHOP+side]);
    }
}

/*
 * - open and half-open files
 * - mobility
 */
static void evaluate_rooks(struct gamestate *pos, struct eval *eval, int side)
{
    const uint64_t rank7[NSIDES] = {rank_mask[RANK_7], rank_mask[RANK_2]};
    const uint64_t rank8[NSIDES] = {rank_mask[RANK_8], rank_mask[RANK_1]};
    uint64_t pieces;
    uint64_t all_pawns;
    uint64_t moves;
    int      sq;
    int      file;

    all_pawns = pos->bb_pieces[WHITE_PAWN]|pos->bb_pieces[BLACK_PAWN];
    pieces = pos->bb_pieces[ROOK+side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        file = FILENR(sq);

        /* Open and half-open files */
        if ((file_mask[file]&all_pawns) == 0ULL) {
            eval->positional[MIDDLEGAME][side] += ROOK_OPEN_FILE;
            eval->positional[ENDGAME][side] += ROOK_OPEN_FILE;
        } else if ((file_mask[file]&pos->bb_pieces[PAWN+side]) == 0ULL) {
            eval->positional[MIDDLEGAME][side] += ROOK_HALF_OPEN_FILE;
            eval->positional[ENDGAME][side] += ROOK_HALF_OPEN_FILE;
        }

        /* 7th rank */
        if (ISBITSET(rank7[side], sq)) {
            /*
             * Only give bonus if the enemy king is on the 8th rank
             * or if there are enenmy pawns on the 7th rank.
             */
            if ((pos->bb_pieces[KING+FLIP_COLOR(side)]&rank8[side]) ||
                (pos->bb_pieces[PAWN+FLIP_COLOR(side)]&rank7[side])) {
                eval->positional[MIDDLEGAME][side] += ROOK_ON_7TH_MG;
                eval->positional[ENDGAME][side] += ROOK_ON_7TH_EG;
            }
        }

        /* Mobility */
        moves = bb_rook_moves(pos->bb_all, sq)&(~pos->bb_sides[side]);
        eval->mobility[MIDDLEGAME][side] +=
                                    (BITCOUNT(moves)*mobility_table[ROOK+side]);
        eval->mobility[ENDGAME][side] +=
                                    (BITCOUNT(moves)*mobility_table[ROOK+side]);
    }
}

/*
 * - open and half-open files
 * - mobility
 */
static void evaluate_queens(struct gamestate *pos, struct eval *eval, int side)
{
    uint64_t pieces;
    uint64_t all_pawns;
    uint64_t moves;
    int      sq;
    int      file;

    all_pawns = pos->bb_pieces[WHITE_PAWN]|pos->bb_pieces[BLACK_PAWN];
    pieces = pos->bb_pieces[QUEEN+side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        file = FILENR(sq);

        /* Open and half-open files */
        if ((file_mask[file]&all_pawns) == 0ULL) {
            eval->positional[MIDDLEGAME][side] += QUEEN_OPEN_FILE;
            eval->positional[ENDGAME][side] += QUEEN_OPEN_FILE;
        } else if ((file_mask[file]&pos->bb_pieces[PAWN+side]) == 0ULL) {
            eval->positional[MIDDLEGAME][side] += QUEEN_HALF_OPEN_FILE;
            eval->positional[ENDGAME][side] += QUEEN_HALF_OPEN_FILE;
        }

        /* Mobility */
        moves = bb_queen_moves(pos->bb_all, sq)&(~pos->bb_sides[side]);
        eval->mobility[MIDDLEGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[QUEEN+side]);
        eval->mobility[ENDGAME][side] +=
                                (BITCOUNT(moves)*mobility_table[QUEEN+side]);
    }
}

/*
 * - pawn shield
 */
static void evaluate_king_safety(struct gamestate *pos, struct eval *eval,
                                 int side)
{
    const uint64_t      queenside[NSIDES] = {
                                        sq_mask[A1]|sq_mask[B1]|sq_mask[C1],
                                        sq_mask[A8]|sq_mask[B8]|sq_mask[C8]};
    const uint64_t      kingside[NSIDES] = {
                                        sq_mask[F1]|sq_mask[G1]|sq_mask[H1],
                                        sq_mask[F8]|sq_mask[G8]|sq_mask[H8]};
    struct pawntt_item  *item;

    /*
     * If the king has moved to the side then it is good to keep a
     * shield of pawns in front of it. The only exception is if
     * there is a rook between the king and the corner. In this case
     * keeping a pawn shield will get the rook trapped.
     */
    item = &eval->pawntt;
    if (queenside[side]&pos->bb_pieces[KING+side]) {
        if (!((pos->bb_pieces[ROOK+side]&queenside[side]) &&
              (LSB(pos->bb_pieces[ROOK+side]&queenside[side]) <
               LSB(pos->bb_pieces[KING+side])))) {
                  eval->king_safety[MIDDLEGAME][side] =
                                            item->pawn_shield[side][QUEENSIDE];
              }
    } else if (kingside[side]&pos->bb_pieces[KING+side]) {
        if (!((pos->bb_pieces[ROOK+side]&kingside[side]) &&
              (LSB(pos->bb_pieces[ROOK+side]&kingside[side]) >
               LSB(pos->bb_pieces[KING+side])))) {
                  eval->king_safety[MIDDLEGAME][side] =
                                            item->pawn_shield[side][KINGSIDE];
              }
    }

    /*
     * In the end game it's more important to have an active king so
     * don't try to hide it behind a pawn shield.
     */
    eval->king_safety[ENDGAME][side] = 0;
}

static void do_eval(struct gamestate *pos, struct eval *eval)
{
    int k;

    memset(eval, 0, sizeof(struct eval));

    /* Copy scores that were updated iteratively to the eval struct */
    for (k=0;k<NPHASES;k++) {
        eval->psq[k][WHITE] = pos->psq[k][WHITE];
        eval->psq[k][BLACK] = pos->psq[k][BLACK];
        eval->material[k][WHITE] = pos->material[WHITE];
        eval->material[k][BLACK] = pos->material[BLACK];
    }

    /* Check if the position is present in the pawn transposition table */
    eval->in_pawntt = hash_pawntt_lookup(pos, &eval->pawntt);

    /* Evaluate the position from each sides pov */
    if (!eval->in_pawntt) {
        hash_pawntt_init_item(&eval->pawntt);
        evaluate_pawn_structure(pos, eval, WHITE);
        evaluate_pawn_structure(pos, eval, BLACK);
    }
    evaluate_knights(pos, eval, WHITE);
    evaluate_knights(pos, eval, BLACK);
    evaluate_bishops(pos, eval, WHITE);
    evaluate_bishops(pos, eval, BLACK);
    evaluate_rooks(pos, eval, WHITE);
    evaluate_rooks(pos, eval, BLACK);
    evaluate_queens(pos, eval, WHITE);
    evaluate_queens(pos, eval, BLACK);
    evaluate_king_safety(pos, eval, WHITE);
    evaluate_king_safety(pos, eval, BLACK);

    /*
     * Update the evaluation scores with information from
     * the pawn tranposition table. If the position was not
     * found then the item field have been updated during the
     * evaluation and therefore contains the correct information.
     */
    for (k=0;k<NPHASES;k++) {
        eval->pawn_structure[k][WHITE] = eval->pawntt.score[k][WHITE];
        eval->pawn_structure[k][BLACK] = eval->pawntt.score[k][BLACK];
    }

    /* Update the pawn hash table */
    if (!eval->in_pawntt) {
        hash_pawntt_store(pos, &eval->pawntt);
    }

    /* Summarize each evaluation term from white's pov */
    for (k=0;k<NPHASES;k++) {
        eval->material[k][BOTH] =
                            eval->material[k][WHITE] - eval->material[k][BLACK];
        eval->material_adj[k][BOTH] =
                    eval->material_adj[k][WHITE] - eval->material_adj[k][BLACK];
        eval->psq[k][BOTH] = eval->psq[k][WHITE] - eval->psq[k][BLACK];
        eval->pawn_structure[k][BOTH] =
                eval->pawn_structure[k][WHITE] - eval->pawn_structure[k][BLACK];
        eval->king_safety[k][BOTH] =
                    eval->king_safety[k][WHITE] - eval->king_safety[k][BLACK];
        eval->positional[k][BOTH] =
                        eval->positional[k][WHITE] - eval->positional[k][BLACK];
        eval->mobility[k][BOTH] =
                            eval->mobility[k][WHITE] - eval->mobility[k][BLACK];
    }

    /* Summarize the evaluation terms for each side */
    for (k=0;k<NPHASES;k++) {
        eval->sum[k][WHITE] =
                    eval->material[k][WHITE] + eval->material_adj[k][WHITE] +
                    eval->psq[k][WHITE] + eval->pawn_structure[k][WHITE] +
                    eval->king_safety[k][WHITE] + eval->positional[k][WHITE] +
                    eval->mobility[k][WHITE];
        eval->sum[k][BLACK] =
                    eval->material[k][BLACK] + eval->material_adj[k][BLACK] +
                    eval->psq[k][BLACK] + eval->pawn_structure[k][BLACK] +
                    eval->king_safety[k][BLACK] + eval->positional[k][BLACK] +
                    eval->mobility[k][BLACK];
        eval->sum[k][BOTH] = eval->sum[k][WHITE] - eval->sum[k][BLACK];
    }
}

void eval_reset(void)
{
    /* Initialize the mobility table */
    mobility_table[WHITE_PAWN] = 0;
    mobility_table[BLACK_PAWN] = 0;
    mobility_table[WHITE_KNIGHT] = KNIGHT_MOBILITY;
    mobility_table[BLACK_KNIGHT] = KNIGHT_MOBILITY;
    mobility_table[WHITE_BISHOP] = BISHOP_MOBILITY;
    mobility_table[BLACK_BISHOP] = BISHOP_MOBILITY;
    mobility_table[WHITE_ROOK] = ROOK_MOBILITY;
    mobility_table[BLACK_ROOK] = ROOK_MOBILITY;
    mobility_table[WHITE_QUEEN] = QUEEN_MOBILITY;
    mobility_table[BLACK_QUEEN] = QUEEN_MOBILITY;
    mobility_table[WHITE_KING] = 0;
    mobility_table[BLACK_KING] = 0;

    /* Initialize the passed pawns table */
    passed_pawn_scores[RANK_1] = 0;
    passed_pawn_scores[RANK_2] = PASSED_PAWN_RANK2;
    passed_pawn_scores[RANK_3] = PASSED_PAWN_RANK3;
    passed_pawn_scores[RANK_4] = PASSED_PAWN_RANK4;
    passed_pawn_scores[RANK_5] = PASSED_PAWN_RANK5;
    passed_pawn_scores[RANK_6] = PASSED_PAWN_RANK6;
    passed_pawn_scores[RANK_7] = PASSED_PAWN_RANK7;
    passed_pawn_scores[RANK_8] = 0;
}

int eval_evaluate(struct gamestate *pos)
{
    struct eval eval;
    int         phase;
    int         score;
    int         score_mg;
    int         score_eg;

    assert(valid_board(pos));
    assert(pos->material[WHITE] == eval_material(pos, WHITE));
    assert(pos->material[BLACK] == eval_material(pos, BLACK));
    assert(pos->psq[MIDDLEGAME][WHITE] == eval_psq(pos, WHITE, false));
    assert(pos->psq[MIDDLEGAME][BLACK] == eval_psq(pos, BLACK, false));
    assert(pos->psq[ENDGAME][WHITE] == eval_psq(pos, WHITE, true));
    assert(pos->psq[ENDGAME][BLACK] == eval_psq(pos, BLACK, true));

    /*
     * If no player have enough material left
     * to checkmate then it's a draw.
     */
    if (eval_is_material_draw(pos)) {
        return 0;
    }

    /* Evaluate the position */
    do_eval(pos, &eval);

    /* Convert the score to be from side to move's point of view */
    score_mg = (pos->stm == WHITE)?
                        eval.sum[MIDDLEGAME][BOTH]:-eval.sum[MIDDLEGAME][BOTH];
    score_eg = (pos->stm == WHITE)?
                        eval.sum[ENDGAME][BOTH]:-eval.sum[ENDGAME][BOTH];

    /* Adjust score for game phase */
    phase = calculate_game_phase(pos);
    if (phase < 0) {
        char fenstr[256];
        fen_build_string(pos, fenstr);
        printf("%s\n", fenstr);
    }
    assert(phase >= 0 && phase <= 256);
    score = calculate_tapered_eval(phase, score_mg, score_eg);

    return score;
}

void eval_display(struct gamestate *pos)
{
    struct eval eval;
    int         phase;
    int         score;

    assert(valid_board(pos));
    assert(pos->material[WHITE] == eval_material(pos, WHITE));
    assert(pos->material[BLACK] == eval_material(pos, BLACK));
    assert(pos->psq[MIDDLEGAME][WHITE] == eval_psq(pos, WHITE, false));
    assert(pos->psq[MIDDLEGAME][BLACK] == eval_psq(pos, BLACK, false));
    assert(pos->psq[ENDGAME][WHITE] == eval_psq(pos, WHITE, true));
    assert(pos->psq[ENDGAME][BLACK] == eval_psq(pos, BLACK, true));

    /*
     * If no player have enough material left
     * to checkmate then it's a draw.
     */
    if (eval_is_material_draw(pos)) {
        printf("Draw by insufficient material\n");
        printf("Score: 0\n");
        return;
    }

    /* Evaluate the position */
    do_eval(pos, &eval);

    /* Adjust score for game phase */
    phase = calculate_game_phase(pos);
    assert(phase >= 0 && phase <= 256);
    score = calculate_tapered_eval(phase, eval.sum[MIDDLEGAME][BOTH],
                                   eval.sum[ENDGAME][BOTH]);

    /* Print the evaluation */
    printf("  Evaluation Term       White        Black         Total\n");
    printf("                      MG     EG    MG     EG     MG     EG\n");
    printf("-------------------------------------------------------------\n");
    printf("Material                                      %5d   %5d\n",
           eval.material[MIDDLEGAME][BOTH], eval.material[ENDGAME][BOTH]);
    printf("Material adjustment %5d  %5d %5d  %5d %5d   %5d\n",
           eval.material_adj[MIDDLEGAME][WHITE],
           eval.material_adj[ENDGAME][WHITE],
           eval.material_adj[MIDDLEGAME][BLACK],
           eval.material_adj[ENDGAME][BLACK],
           eval.material_adj[MIDDLEGAME][BOTH],
           eval.material_adj[ENDGAME][BOTH]);
    printf("Piece/square tables %5d  %5d %5d  %5d %5d   %5d\n",
           eval.psq[MIDDLEGAME][WHITE], eval.psq[ENDGAME][WHITE],
           eval.psq[MIDDLEGAME][BLACK], eval.psq[ENDGAME][BLACK],
           eval.psq[MIDDLEGAME][BOTH], eval.psq[ENDGAME][BOTH]);
    printf("Pawn structure      %5d  %5d %5d  %5d %5d   %5d\n",
           eval.pawn_structure[MIDDLEGAME][WHITE],
           eval.pawn_structure[ENDGAME][WHITE],
           eval.pawn_structure[MIDDLEGAME][BLACK],
           eval.pawn_structure[ENDGAME][BLACK],
           eval.pawn_structure[MIDDLEGAME][BOTH],
           eval.pawn_structure[ENDGAME][BOTH]);
    printf("King safety         %5d  %5d %5d  %5d %5d   %5d\n",
           eval.king_safety[MIDDLEGAME][WHITE],
           eval.king_safety[ENDGAME][WHITE],
           eval.king_safety[MIDDLEGAME][BLACK],
           eval.king_safety[ENDGAME][BLACK],
           eval.king_safety[MIDDLEGAME][BOTH],
           eval.king_safety[ENDGAME][BOTH]);
    printf("Positional themes   %5d  %5d %5d  %5d %5d   %5d\n",
           eval.positional[MIDDLEGAME][WHITE], eval.positional[ENDGAME][WHITE],
           eval.positional[MIDDLEGAME][BLACK], eval.positional[ENDGAME][BLACK],
           eval.positional[MIDDLEGAME][BOTH], eval.positional[ENDGAME][BOTH]);
    printf("Mobility            %5d  %5d %5d  %5d %5d   %5d\n",
           eval.mobility[MIDDLEGAME][WHITE], eval.mobility[ENDGAME][WHITE],
           eval.mobility[MIDDLEGAME][BLACK], eval.mobility[ENDGAME][BLACK],
           eval.mobility[MIDDLEGAME][BOTH], eval.mobility[ENDGAME][BOTH]);
    printf("-------------------------------------------------------------\n");
    printf("Total                                         %5d   %5d\n",
           eval.sum[MIDDLEGAME][BOTH], eval.sum[ENDGAME][BOTH]);
    printf("\n");
    printf("Game phase: %d [0, 256]\n", phase);
    printf("Score:      %d (for white)\n", score);
}

int eval_material(struct gamestate *pos, int side)
{
    int score;
    int piece;

    assert(valid_board(pos));
    assert(valid_side(side));

    score = 0;
    for (piece=side;piece<NPIECES;piece+=2) {
        score += BITCOUNT(pos->bb_pieces[piece])*material_values[piece];
    }

    return score;
}

int eval_psq(struct gamestate *pos, int side, bool endgame)
{
    int      score;
    uint64_t pieces;
    int      sq;
    int      piece;

    assert(valid_board(pos));
    assert(valid_side(side));

    score = 0;
    pieces = pos->bb_sides[side];
    while (pieces != 0ULL) {
        sq = POPBIT(&pieces);
        piece = pos->pieces[sq];
        switch (piece) {
        case WHITE_PAWN:
            score += PSQ_TABLE_PAWN[sq];
            break;
        case BLACK_PAWN:
            score += PSQ_TABLE_PAWN[MIRROR(sq)];
            break;
        case WHITE_KNIGHT:
            score += PSQ_TABLE_KNIGHT[sq];
            break;
        case BLACK_KNIGHT:
            score += PSQ_TABLE_KNIGHT[MIRROR(sq)];
            break;
        case WHITE_BISHOP:
            score += PSQ_TABLE_BISHOP[sq];
            break;
        case BLACK_BISHOP:
            score += PSQ_TABLE_BISHOP[MIRROR(sq)];
            break;
        case WHITE_KING:
            score += endgame?PSQ_TABLE_KING_EG[sq]:PSQ_TABLE_KING_MG[sq];
            break;
        case BLACK_KING:
            score += endgame?
                PSQ_TABLE_KING_EG[MIRROR(sq)]:PSQ_TABLE_KING_MG[MIRROR(sq)];
            break;
        case WHITE_ROOK:
            score += PSQ_TABLE_ROOK[sq];
            break;
        case BLACK_ROOK:
            score += PSQ_TABLE_ROOK[MIRROR(sq)];
            break;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            break;
        default:
                assert(false);
                break;
        }
    }

    return score;
}

void eval_update_psq_score(struct gamestate *pos, int add, int piece, int sq)
{
    int delta;
    int color;

    assert(valid_board(pos));
    assert(valid_piece(piece));
    assert(valid_square(sq));

    delta = add?1:-1;
    sq = (COLOR(piece)==WHITE)?sq:MIRROR(sq);
    color = COLOR(piece);
    switch (piece) {
    case WHITE_PAWN:
    case BLACK_PAWN:
        pos->psq[MIDDLEGAME][color] += (delta*PSQ_TABLE_PAWN[sq]);
        pos->psq[ENDGAME][color] += (delta*PSQ_TABLE_PAWN[sq]);
        break;
    case WHITE_KNIGHT:
    case BLACK_KNIGHT:
        pos->psq[MIDDLEGAME][color] += (delta*PSQ_TABLE_KNIGHT[sq]);
        pos->psq[ENDGAME][color] += (delta*PSQ_TABLE_KNIGHT[sq]);
        break;
    case WHITE_BISHOP:
    case BLACK_BISHOP:
        pos->psq[MIDDLEGAME][color] += (delta*PSQ_TABLE_BISHOP[sq]);
        pos->psq[ENDGAME][color] += (delta*PSQ_TABLE_BISHOP[sq]);
        break;
    case WHITE_KING:
    case BLACK_KING:
        pos->psq[MIDDLEGAME][color] += (delta*PSQ_TABLE_KING_MG[sq]);
        pos->psq[ENDGAME][color] += (delta*PSQ_TABLE_KING_EG[sq]);
        break;
    case WHITE_ROOK:
    case BLACK_ROOK:
        pos->psq[MIDDLEGAME][color] += (delta*PSQ_TABLE_ROOK[sq]);
        pos->psq[ENDGAME][color] += (delta*PSQ_TABLE_ROOK[sq]);
        break;
    case WHITE_QUEEN:
    case BLACK_QUEEN:
        break;
    default:
        assert(false);
        break;
    }
}

/*
 * The following combination of pieces can never lead to chekmate:
 * - King vs King
 * - King+Knight vs King
 * - King+Bishops vs King (if the bishops operate on the same color squares)
 */
bool eval_is_material_draw(struct gamestate *pos)
{
    int wb;
    int wn;
    int bb;
    int bn;

    if ((pos->bb_pieces[WHITE_PAWN] != 0ULL) ||
        (pos->bb_pieces[BLACK_PAWN] != 0ULL) ||
        (pos->bb_pieces[WHITE_ROOK] != 0ULL) ||
        (pos->bb_pieces[BLACK_ROOK] != 0ULL) ||
        (pos->bb_pieces[WHITE_QUEEN] != 0ULL) ||
        (pos->bb_pieces[BLACK_QUEEN] != 0ULL)) {
        return false;
    }

    wn = BITCOUNT(pos->bb_pieces[WHITE_KNIGHT]);
    bn = BITCOUNT(pos->bb_pieces[BLACK_KNIGHT]);
    wb = BITCOUNT(pos->bb_pieces[WHITE_BISHOP]);
    bb = BITCOUNT(pos->bb_pieces[BLACK_BISHOP]);

    /* King vs King */
    if ((wn == 0) && (bn == 0) && (wb == 0) && (bb == 0)) {
        return true;
    }
    /* Knight+King vs King */
    if ((wn == 1) && (bn == 0) && (wb == 0) && (bb == 0)) {
        return true;
    }
    /* King vs King+Knight */
    if ((wn == 0) && (bn == 1) && (wb == 0) && (bb == 0)) {
        return true;
    }
    /* King+Bishops vs King */
    if ((wn == 0) && (bn == 0) && (wb > 0) && (bb == 0)) {
        return !(((pos->bb_pieces[WHITE_BISHOP]&white_square_mask) != 0ULL) &&
                    ((pos->bb_pieces[WHITE_BISHOP]&black_square_mask) != 0ULL));
    }
    /* King vs King+Bishops */
    if ((wn == 0) && (bn == 0) && (wb == 0) && (bb > 0)) {
        return !(((pos->bb_pieces[BLACK_BISHOP]&white_square_mask) != 0ULL) &&
                    ((pos->bb_pieces[BLACK_BISHOP]&black_square_mask) != 0ULL));
    }

    return false;
}
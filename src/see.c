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
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "see.h"
#include "eval.h"
#include "bitboard.h"
#include "validation.h"
#include "fen.h"

int see_material[NPIECES];

static uint64_t find_xray_attackers(struct position *pos, uint64_t occ,
                                    int target, uint64_t last_attacker)
{
    int      sq;
    int      piece;
    int      piece_value;
    uint64_t xrays;
    int      fdelta;
    int      rdelta;
    uint64_t mask;
    uint64_t attackers;

    /*
     * First check if there can actually be an xray attacker
     * hidden behind the last attacker.
     */
    sq = LSB(last_attacker);
    piece = pos->pieces[sq];
    piece_value = VALUE(piece);
    if (piece_value == KNIGHT) {
        return 0ULL;
    }

    /* Find the normalized direction of potential xray attackers */
    fdelta = FILENR(sq) - FILENR(target);
    if (fdelta != 0) {
        fdelta /= abs(fdelta);
    }
    rdelta = RANKNR(sq) - RANKNR(target);
    if (rdelta != 0) {
        rdelta /= abs(rdelta);
    }

    /* Generate slider moves in the xray direction */
    xrays = bb_slider_moves(occ, sq, fdelta, rdelta);

    /*
     * If the xrays set contains a piece that can move in the given
     * direction then that piece can attack the target square and
     * should be added as an attacker.
     */
    attackers = 0ULL;
    mask = 0ULL;
    if ((fdelta == 0) || (rdelta == 0)) {
        mask = pos->bb_pieces[WHITE_ROOK] | pos->bb_pieces[WHITE_QUEEN] |
                    pos->bb_pieces[BLACK_ROOK] | pos->bb_pieces[BLACK_QUEEN];
    } else {
        mask = pos->bb_pieces[WHITE_BISHOP] | pos->bb_pieces[WHITE_QUEEN] |
                    pos->bb_pieces[BLACK_BISHOP] | pos->bb_pieces[BLACK_QUEEN];
    }
    attackers = xrays & mask;

    return attackers;
}

static uint64_t find_next_attacker(struct position *pos, uint64_t attackers,
                                   int side, int *piece)
{
    uint64_t bb;

    for (*piece=PAWN+side;*piece<NPIECES;*piece+=2) {
        bb = attackers & pos->bb_pieces[*piece];
        if (bb != 0ULL) {
            return ISOLATE(bb);
        }
    }
    return 0ULL;
}

static int move_score(int sq, int piece, int stm, int victim)
{
    int score;

    score = see_material[victim];
    if ((piece == PAWN) &&
        ((rank_mask[RANK_1]|rank_mask[RANK_8])&sq_mask[sq]) != 0ULL) {
        score += see_material[QUEEN+stm];
        score -= see_material[PAWN+stm];
    }

    return score;
}

void see_init(void)
{
    int k;

    for (k=0;k<NPIECES;k++) {
        see_material[k] = (material_values_mg[k] + material_values_eg[k])/2;
    }
}

/*
 * The algorithm used is based on the swap algorithm from the
 * chessprogramming wiki:
 * https://chessprogramming.wikispaces.com/SEE+-+The+Swap+Algorithm
 */
int see_calculate_score(struct position *pos, uint32_t move)
{
    int      score[32];
    int      depth;
    int      to;
    int      from;
    int      piece;
    int      side;
    uint64_t attackers;
    uint64_t attacker;
    uint64_t occ;

    assert(valid_position(pos));
    assert(valid_move(move));
    assert(ISCAPTURE(move));
    assert(!ISENPASSANT(move));

    to = TO(move);
    from = FROM(move);
    piece = pos->pieces[from];
    attacker = sq_mask[from];
    side = COLOR(piece);
    depth = 0;
    score[0] = see_material[pos->pieces[to]];
    occ = pos->bb_all;

    /* Find all pieces that attacks the target square */
    attackers = bb_attacks_to(pos, pos->bb_all, to, WHITE) |
                                bb_attacks_to(pos, pos->bb_all, to, BLACK);

    /* Iterate until there are no more attackers */
    do {
        /* Resolve capture and calculate score at this depth */
        depth++;
        score[depth] = see_material[piece] - score[depth-1];

        /* Remove the attacker from the set of attackers */
        attackers &= ~attacker;
        occ &= ~attacker;

        /* Find xray attackers and add them to the set of attackers */
        attackers |= find_xray_attackers(pos, occ, to, attacker);

        /* Find the next attacker to consider */
        side = FLIP_COLOR(side);
        attacker = find_next_attacker(pos, attackers, side, &piece);
    } while (attacker != 0ULL);

    /* Min-max the score array to find the final score */
    depth--;
    while (depth > 0) {
        if (score[depth] > -score[depth-1]) {
            score[depth-1] = -score[depth];
        }
        depth--;
    }

    return score[0];
}

bool see_ge(struct position *pos, uint32_t move, int threshold)
{
    int      see_score;
    int      sq;
    int      maximizer;
    int      stm;
    int      piece;
    int      victim;
    uint64_t attackers;
    uint64_t attacker;
    uint64_t occ;

    assert(valid_position(pos));
    assert(valid_move(move));

    /*
     * In order for the castling to be legal the destination
     * square of the rook cannot be attacked so the see score
     * is always zero.
     */
    if (ISKINGSIDECASTLE(move) || ISQUEENSIDECASTLE(move)) {
       return threshold < 0;
    }

    /* Find the score of the move */
    maximizer = pos->stm;
    stm = maximizer;
    sq = TO(move);
    piece = pos->pieces[FROM(move)];
    if (ISENPASSANT(move)) {
        see_score = see_material[PAWN+FLIP_COLOR(stm)];
    } else if (ISCAPTURE(move)) {
        see_score = see_material[pos->pieces[sq]];
    } else {
        see_score = 0;
    }
    if (ISPROMOTION(move)) {
        see_score += see_material[PROMOTION(move)+stm];
        see_score -= see_material[PAWN+stm];
    }

    /* Apply the move */
    occ = pos->bb_all&(~sq_mask[FROM(move)]);
    if (ISENPASSANT(move)) {
        occ &= ~sq_mask[(pos->stm==WHITE)?sq-8:sq+8];
    }
    victim = ISPROMOTION(move)?PROMOTION(move)+stm:piece;
    stm = FLIP_COLOR(stm);

    /* Find all pieces that attacks the target square */
    attackers = bb_attacks_to(pos, occ, sq, WHITE) |
                                            bb_attacks_to(pos, occ, sq, BLACK);
    attackers &= ~sq_mask[FROM(move)];

    /* Iterate until there are no more attackers */
    while (!ISEMPTY(attackers)) {
        /*
         * Before recapturing compare the current score against the
         * threshold. If the side to move is the initial side to move
         * and the score is alreeady above the threshold then there is
         * no need to continue, same thing if it is the oppenents turn
         * to move and the score is below the threshold.
         */
        if ((stm == maximizer) && (see_score >= threshold)) {
            break;
        } else if ((stm != maximizer) && (see_score < threshold)) {
            break;
        }

        /* Find the next attacker to consider */
        attacker = find_next_attacker(pos, attackers, stm, &piece);
        if (attacker == 0ULL) {
            break;
        }

        /* Update the score based on the move */
        if (stm == maximizer) {
            see_score += move_score(sq, piece, stm, victim);
        } else {
            see_score -= move_score(sq, piece, stm, victim);
        }

        /*
         * Apply the move. The current piece becomes the next victim
         * unless it's a promotion in which case the next victim is
         * a queen.
         */
        attackers &= ~attacker;
        occ &= ~attacker;
        victim = ((VALUE(piece) == PAWN) && ISPROMOTION(move))?QUEEN+stm:piece;
        stm = FLIP_COLOR(stm);

        /* Find xray attackers and add them to the set of attackers */
        attackers |= find_xray_attackers(pos, occ, sq, attacker);
    }

    return see_score >= threshold;
}

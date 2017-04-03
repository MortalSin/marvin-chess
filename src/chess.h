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
#ifndef CHESS_H
#define CHESS_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* The number of sides */
#define NSIDES 2

/* The different piece/square colors */
enum {
    WHITE,
    BLACK,
    NO_SIDE
};
#define BOTH NO_SIDE

/* The different halves of the board */
enum {
    KINGSIDE,
    QUEENSIDE
};

/* The different game phases */
enum {
    MIDDLEGAME,
    ENDGAME
};
#define NPHASES 2

/* The number of different pieces */
#define NPIECES 12

/* The different piece types */
enum {
    PAWN = 0,
    KNIGHT = 2,
    BISHOP = 4,
    ROOK = 6,
    QUEEN = 8,
    KING = 10
};

/* The different pieces */
enum {
    WHITE_PAWN,
    BLACK_PAWN,
    WHITE_KNIGHT,
    BLACK_KNIGHT,
    WHITE_BISHOP,
    BLACK_BISHOP,
    WHITE_ROOK,
    BLACK_ROOK,
    WHITE_QUEEN,
    BLACK_QUEEN,
    WHITE_KING,
    BLACK_KING,
    NO_PIECE
};

/* The color of a piece */
#define COLOR(p) ((p)&BLACK)

/* The value of a piece */
#define VALUE(p) ((p)&(~BLACK))

/* Change WHITE to BLACK and vive versa. */
#define FLIP_COLOR(c) ((c)^BLACK)

/* Constants for the number of different squares/ranks/files */
#define NSQUARES 64
#define NFILES 8
#define NRANKS 8
#define NDIAGONALS 15

/* The different files */
enum {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H
};

/* The different ranks */
enum {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8
};

/* The different squares */
enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

/* Calculate the square number from rank and file */
#define SQUARE(f, r) (((r)<<3)+(f))

/*
 * Calculate the mirrored version of a square. For instance the
 * mirrored version of A1 is A8.
 */
#define MIRROR(sq) mirror_table[(sq)]

/* The rank of the square */
#define RANKNR(sq) ((sq)>>3)

/* The file of the square */
#define FILENR(sq) ((sq)&7)

/* Check if a square is outside of the board */
#define SQUAREOFFBOARD(f, r) (((f) < FILE_A) || \
                              ((f) > FILE_H) || \
                              ((r) < RANK_1) || \
                              ((r) > RANK_8))

/* Masks for the different diagonals */
#define A1H8(sq) a1h8_masks[sq2diag_a1h8[(sq)]]
#define A8H1(sq) a8h1_masks[sq2diag_a8h1[(sq)]]

/* Flags indicating castling availability */
enum {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8
};

/* Flags for different move types */
enum {
    NORMAL  =  0,
    CAPTURE  = 1,
    PROMOTION = 2,
    EN_PASSANT = 4,
    KINGSIDE_CASTLE = 8,
    QUEENSIDE_CASTLE = 16,
    NULL_MOVE = 32
};

/*
 * Chess moves are represented using an unsigned 32-bit integer. The bits
 * are assigned as follows:
 *
 * bit 0-5: from square (0-63)
 * bit 6-11: to square (0-63)
 * bit 12-15: promoted piece (see pieces enum)
 * bit 16-21: move type flags (see move types enum)
 */
#define MOVE(f, t, p, l)        ((uint32_t)(((f)) | \
                                 ((t)<<6) | \
                                 ((p)<<12) | \
                                 ((l)<<16)))
#define NULLMOVE                MOVE(0, 0, NO_PIECE, NULL_MOVE)
#define FROM(m)                 ((int)((m)&0x0000003F))
#define TO(m)                   ((int)(((m)>>6)&0x0000003F))
#define PROMOTION(m)            ((int)(((m)>>12)&0x0000000F))
#define TYPE(m)                 ((int)(((m)>>16)&0x0000003F))
#define ISNORMAL(m)             (TYPE((m)) == 0)
#define ISCAPTURE(m)            ((TYPE((m))&CAPTURE) != 0)
#define ISPROMOTION(m)          ((TYPE((m))&PROMOTION) != 0)
#define ISENPASSANT(m)          ((TYPE((m))&EN_PASSANT) != 0)
#define ISKINGSIDECASTLE(m)     ((TYPE((m))&KINGSIDE_CASTLE) != 0)
#define ISQUEENSIDECASTLE(m)    ((TYPE((m))&QUEENSIDE_CASTLE) != 0)
#define ISNULLMOVE(m)           ((TYPE((m))&NULL_MOVE) != 0)
#define NOMOVE                  0

/* The maximum number of chess moves that can occur in a game */
#define MAX_MOVES 250
#define MAX_CAPTURES 16

/* The maximum size of the game history */
#define MAX_HISTORY_SIZE 2048

/* The maximum supported search depth */
#define MAX_SEARCH_DEPTH 100

/* The maximum possible quiescence search depth */
#define MAX_QUIESCENCE_DEPTH 32

/* The maximum number of possible plies in the search tree */
#define MAX_PLY MAX_SEARCH_DEPTH+MAX_QUIESCENCE_DEPTH

/* Data structure used to keep track of the principle variation */
struct pv {
    /* The moves that makes up the principle variation */
    uint32_t moves[MAX_PLY];
    /* The length of the principle variation */
    int length;
};

/* List of moves */
struct movelist {
    /* The list of moves */
    uint32_t moves[MAX_MOVES];
    /* The number of moves in the list */
    int nmoves;
};

/* Move with extended information */
struct moveinfo {
    /* The move */
    uint32_t move;
    /* SEE score for captures, 0 for non-captures */
    int see;
    /* Move ordering score */
    int score;
};

/*
 * Move selector struct. Holds information for finding the
 * next move to search for a specific position.
 */
struct moveselect {
    /* The current depth. For quiescence nodes the depth is <= 0. */
    int depth;
    /* Indicates if the position is a quiscence node */
    bool qnode;
    /* Indicates if the position is the root node */
    bool root;
    /* Move fetched from the transposition table for this position */
    uint32_t ttmove;
    /* Additional information for the availables moves */
    struct moveinfo moveinfo[MAX_MOVES];
    /* The number of moves */
    int nmoves;
    /* Array for storing bad captures */
    struct moveinfo badcapinfo[MAX_CAPTURES];
    /* The number of bad captures */
    int nbadcaps;
    /* Index of the move currently being searched */
    int idx;
    /* The current move generation phase */
    int phase;
};

/* Struct for unmaking a move */
struct unmake {
    /* The move to unmake */
    uint32_t move;
    /* The captured piece, or NO_PIECE if the move is not a capture */
    int capture;
    /* Castling permissions before the move was made */
    int castle;
    /* En-passant target square before the move was made */
    int ep_sq;
    /* Fifty-move-draw counter before the move was made */
    int fifty;
    /* The unique position key before the move was made */
    uint64_t key;
    /* The unique pawn key before the move was made */
    uint64_t pawnkey;
    /* Indicates if the side to move before the move was made was in check */
    bool in_check;
};

/* The different supported time controls */
enum timectl_type {
    TC_SUDDEN_DEATH,
    TC_FISCHER,
    TC_INFINITE,
    TC_TOURNAMENT,
    TC_FIXED_TIME
};

/* An opening book entry */
struct book_entry {
    uint32_t move;
    uint16_t weight;
};

/*
 * An item in the main transposition table,
 * which represents a single position.
 */
struct tt_item {
    /* The key of this position */
    uint64_t key;
    /* The best move found */
    uint32_t move;
    /*
     * The score for the position. The type of
     * the score is determined by the flags parameter.
     */
    int score;
    /* The depth to which the position was searched */
    uint8_t depth;
    /*
     * The type of the score parameter. Possible
     * values are: EXACT, ALPHA, BETA and PV.
     */
    uint8_t type;
    /* The time when the position was stored */
    uint8_t date;
    /*
     * Dummy field added to make sure that the struct has the
     * same size for both 32-bit and 64-bit builds.
     */
    uint32_t padding;
};

/* The number of items stored in each transposition table bucket */
#define TT_BUCKET_SIZE 3

/* Transposition table bucket */
struct tt_bucket {
    /* Items stored in this bucket */
    struct tt_item items[TT_BUCKET_SIZE];
};

/* An item in the pawn transposition table */
struct pawntt_item {
    /* The pawn key */
    uint64_t pawnkey;
    /* Bitboards of all passed pawns */
    uint64_t passers[NSIDES];
    /* The score of pawn related terms for each side */
    int pawn_shield[NSIDES][2];
    int score[NPHASES][NSIDES];
    /* Indicates if the item is being used */
    bool used;
    /*
     * Dummy field added to make sure that the struct has the
     * same size for both 32-bit and 64-bit builds.
     */
    uint32_t padding;
};

/* Enum for different chess protocols */
enum protocol {
    PROTOCOL_UNSET,
    PROTOCOL_UCI,
    PROTOCOL_XBOARD
};

/* Data structure holding the state of an ongoing game */
struct gamestate {
    /* Engine protocol mode */
    enum protocol protocol;

    /*
     * Location of each piece on the board. An
     * empty square is identified NO_PIECE.
     */
    int pieces[NSQUARES];
    /* Bitboards for the different pieces */
    uint64_t bb_pieces[NPIECES];
    /* Bitboards for the pieces of the different sides */
    uint64_t bb_sides[NSIDES];
    /* Bitboard for all pieces */
    uint64_t bb_all;
    /* Key that uniquely identifies the current position */
    uint64_t key;
    /*
     * Key that uniquely identifies the location of
     * the pawns in the current position.
     */
    uint64_t pawnkey;

    /* The en-passant target square */
    int ep_sq;
    /* Castling availability for both sides */
    int castle;
    /* The side to move */
    int stm;
    /* Halfmove counter */
    int ply;
    /* Halfmove counter relative to the root of the search tree */
    int sply;
    /* Counter used for the fifty-move-draw rule */
    int fifty;
    /* Fullmove counter */
    int fullmove;

    /* Game history used for undoing moves */
    struct unmake history[MAX_HISTORY_SIZE];
    /* Move selector struct for each ply in the search tree */
    struct moveselect ppms[MAX_PLY];
    /* List of moves to search */
    struct movelist root_moves;
    /*
     * Parameter used during the search to keep track of the current
     * principle variation at a certain depth. After the search the
     * complete variation can be found in pv_table[0].
     */
    struct pv pv_table[MAX_PLY];
    /* Tables used for killer heuristics */
    uint32_t killer_table[MAX_PLY][2];
    /* Tables used for history heuristics */
    int history_table[NSIDES][NSQUARES][NSQUARES];
    /* The best move found so far */
    uint32_t best_move;
    /* The move the engine would like to ponder on */
    uint32_t ponder_move;

    /* Default size of the main transposition table (in MB) */
    int default_hash_size;
    /* Main transposition table */
    struct tt_bucket *tt_table;
    /* The number of entries in the transposition table */
    int tt_size;
    /* The current "date" of the transposition table */
    uint8_t date;

    /* Pawn transposition table */
    struct pawntt_item *pawntt_table;
    /* The number of entries in the pawn transposition table */
    int pawntt_size;

    /* Flag indicating if it's time to abort the search */
    bool abort;
    /*
     * Indicates if it is ok for the engine to abort a
     * search if it detects a mate.
     */
    bool exit_on_mate;
    /* The maximum depth the engine should search to */
    int sd;
    /* Flag used to suppress output during search */
    bool silent;
    /* Flag indicating if the engine should use it's own opening book */
    bool use_own_book;
    /*
     * Flag indicating if the engine is currently following
     * the opening book.
     */
    bool in_book;
    /* Flag indicating if pondering is enabled */
    bool ponder;
	/*
	 * Flag indicating if the engine is currently
	 * searching in pondering mode.
	 */
	bool pondering;

    /* The time control type to use for the search */
    enum timectl_type tc_type;
    /* The number of milliseconds left on the clock */
    int time_left;
    /* The time increment (in milliseconds) */
    int increment;
    /* The number of moves to the next time control */
    int movestogo;
    /* The time when the current search was started */
    time_t search_start;
    /*
     * Limit on how long the engine is allowed to search. In
     * some special circumstances it can be ok to exceed
     * this limit.
     */
    time_t soft_time_limit;
    /* A hard time limit that may not be exceeded */
    time_t hard_time_limit;
    /* Indicates if the engine is resolving a fail-low/fail-high at the root */
    bool resolving_root_fail;

    /*
     * The material score and piece/square table score for each
     * player. These value is incrementally updated during the search.
     */
    int material[NSIDES];
    int psq[NPHASES][NSIDES];

    /* The current search depth in plies */
    int depth;
    /* The current selective search depth in plies */
    int seldepth;
    /* The move currently being searched */
    uint32_t currmove;
    /* The number of the move currently being searched (one-based) */
    int currmovenumber;
    /* The number of nodes searched so far */
    uint32_t nodes;
    /* The number of quiscence nodes searched so far */
    uint32_t qnodes;
    /* The number of entries used in the main transposition table */
    uint32_t tt_used;
};

/* Bitboard mask for each square */
extern uint64_t sq_mask[NSQUARES];

/* Bitboard mask for white squares */
extern uint64_t white_square_mask;

/* Bitboard mask for black squares */
extern uint64_t black_square_mask;

/* Masks for all ranks */
extern uint64_t rank_mask[NRANKS];

/* Masks for all files */
extern uint64_t file_mask[NFILES];

/* Masks for all diagonals in the a1h8 direction */
extern uint64_t a1h8_masks[NDIAGONALS];

/* Masks for all diagonals in the a8h1 direction */
extern uint64_t a8h1_masks[NDIAGONALS];

/*
 * Masks for all ranks in front of a specified rank. For instance
 * the ranks_ahead_mask[6][WHITE] mask includes rank 7.
 */
extern uint64_t ranks_ahead_mask[NRANKS][NSIDES];

/* Character representation for each piece */
extern char piece2char[NPIECES+1];

/* Table mapping a square to an A1H8 diagonal index */
extern int sq2diag_a1h8[NSQUARES];

/* Table mapping a square to an A8H1 diagonal index */
extern int sq2diag_a8h1[NSQUARES];

/* Table used for mirroring squares */
extern int mirror_table[NSQUARES];

/* Table containing the square color for all squares on the chess table */
extern int sq_color[NSQUARES];

/* Initialize chess data */
void chess_data_init(void);

/*
 * Create a new game state object.
 *
 * @param hash_size The size of the main transposition table.
 * @return Returns the new object.
 */
struct gamestate* create_game_state(int hash_size);

/*
 * Destroy a game state object.
 *
 * @param pos The object to destroy.
 */
void destroy_game_state(struct gamestate *pos);

/*
 * Reset a game state object.
 *
 * @param pos The object to destroy.
 */
void reset_game_state(struct gamestate *pos);

/*
 * Convert a move into a string representation.
 *
 * @param move The move.
 * @param str Pointer to store the string representation at.
 */
void move2str(uint32_t move, char *str);

/*
 * Convert a move in algebraic notation to the internal move format.
 *
 * @param str A move in algebraic notation.
 * @param pos The current chess position.
 * @return Returns the move in our internal representation.
 */
uint32_t str2move(char *str, struct gamestate *pos);

#endif
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
/*
 * Note: This file is autogenerated. Do not edit.
 */

#include "evalparams.h"

int DOUBLE_PAWNS_MG = -19;
int DOUBLE_PAWNS_EG = -22;
int ISOLATED_PAWN_MG = -14;
int ISOLATED_PAWN_EG = -14;
int ROOK_OPEN_FILE_MG = 41;
int ROOK_OPEN_FILE_EG = 5;
int ROOK_HALF_OPEN_FILE_MG = 18;
int ROOK_HALF_OPEN_FILE_EG = 18;
int QUEEN_OPEN_FILE_MG = 0;
int QUEEN_OPEN_FILE_EG = 12;
int QUEEN_HALF_OPEN_FILE_MG = 13;
int QUEEN_HALF_OPEN_FILE_EG = 8;
int ROOK_ON_7TH_MG = 0;
int ROOK_ON_7TH_EG = 22;
int BISHOP_PAIR_MG = 30;
int BISHOP_PAIR_EG = 68;
int PAWN_SHIELD_RANK1 = 32;
int PAWN_SHIELD_RANK2 = 28;
int PAWN_SHIELD_HOLE = -6;
int PASSED_PAWN_RANK2_MG = 0;
int PASSED_PAWN_RANK3_MG = 0;
int PASSED_PAWN_RANK4_MG = 0;
int PASSED_PAWN_RANK5_MG = 26;
int PASSED_PAWN_RANK6_MG = 64;
int PASSED_PAWN_RANK7_MG = 172;
int PASSED_PAWN_RANK2_EG = 15;
int PASSED_PAWN_RANK3_EG = 14;
int PASSED_PAWN_RANK4_EG = 39;
int PASSED_PAWN_RANK5_EG = 72;
int PASSED_PAWN_RANK6_EG = 118;
int PASSED_PAWN_RANK7_EG = 186;
int KNIGHT_MOBILITY_MG = 6;
int BISHOP_MOBILITY_MG = 7;
int ROOK_MOBILITY_MG = 4;
int QUEEN_MOBILITY_MG = 4;
int KNIGHT_MOBILITY_EG = 1;
int BISHOP_MOBILITY_EG = 5;
int ROOK_MOBILITY_EG = 4;
int QUEEN_MOBILITY_EG = 6;
int PSQ_TABLE_PAWN_MG[NSQUARES] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    -23, -23, -21, -21, -15, 10, 13, -35,
    -16, -21, -16, -7, 2, -1, 16, -8,
    -11, -14, -5, 11, 7, 16, 1, -17,
    -14, -1, 11, 20, 25, 42, -1, 5,
    28, 23, 48, 14, 72, 127, 59, 54,
    58, 62, 55, 40, 11, -91, -131, -87,
    0, 0, 0, 0, 0, 0, 0, 0
};
int PSQ_TABLE_KNIGHT_MG[NSQUARES] = {
    -131, -40, -44, -17, -19, -6, -55, 18,
    -50, -25, -27, 7, 8, 0, -18, 2,
    -30, -19, -4, 19, 27, 6, 16, -13,
    -8, 2, 13, 16, 32, 24, 54, 7,
    20, 5, 32, 58, 24, 54, 19, 50,
    -34, 22, 60, 53, 144, 144, 18, 38,
    -58, -30, 4, 38, 8, 111, 6, -35,
    -136, -150, 18, -7, 30, -97, -74, -74
};
int PSQ_TABLE_BISHOP_MG[NSQUARES] = {
    -17, -2, -6, -20, -11, -9, 48, -22,
    19, 7, 17, -3, 18, 13, 30, -29,
    -6, 14, 13, 5, 8, 27, 1, 18,
    -20, -5, 13, 29, 22, -21, 8, 5,
    -13, 3, 24, 48, 19, 21, -12, 10,
    1, 1, -15, 16, -41, 101, 41, 4,
    -26, -9, -53, -10, -31, -46, -45, -46,
    -75, -43, -30, -9, -4, -1, -26, -67
};
int PSQ_TABLE_ROOK_MG[NSQUARES] = {
    -15, 3, 11, 12, 15, 19, 28, -1,
    -37, -16, -7, 7, -3, 14, 43, -14,
    -47, -30, -16, -14, -8, -8, 43, 19,
    -35, -38, -22, -18, -19, -22, 33, -1,
    -8, 3, 2, 12, 17, 7, 27, 20,
    -6, 21, 32, 29, 90, 88, 91, 17,
    -7, -12, 46, 23, 46, 57, 48, 134,
    47, 56, 46, 56, 29, 48, 61, 68
};
int PSQ_TABLE_QUEEN_MG[NSQUARES] = {
    18, 10, 31, 17, 43, -17, 14, -14,
    24, 4, 11, 19, 19, 60, 12, 6,
    -9, 1, 6, -2, 9, 22, 22, 16,
    -9, -35, -11, 10, 2, 14, 6, 24,
    -8, -24, -13, 2, 22, -11, 14, 35,
    -10, 20, -17, -4, 6, 125, 106, 66,
    6, -68, -28, -56, 2, 49, -35, 64,
    -20, -5, 40, 34, 20, 102, 113, 97
};
int PSQ_TABLE_KING_MG[NSQUARES] = {
    -19, -16, -57, -81, 25, -66, 3, -5,
    8, -27, -18, -46, -19, -32, 27, 27,
    -9, -1, -11, -44, -38, -36, -42, -71,
    -13, 13, -65, -78, -84, -65, -43, -72,
    -65, -59, -130, -140, -166, -72, 21, -108,
    66, 24, -4, -90, -75, -4, 29, 74,
    141, 62, 30, 61, 56, 65, 49, 159,
    -36, 192, 200, 141, 99, 200, 200, 40
};
int PSQ_TABLE_PAWN_EG[NSQUARES] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    27, 27, 11, -12, 24, 16, 9, 14,
    25, 29, 12, 18, 14, 22, 6, 10,
    36, 40, 22, 6, 15, 21, 25, 21,
    61, 48, 24, 7, 15, 16, 34, 34,
    82, 73, 50, 8, -13, 14, 30, 51,
    45, 23, 22, -42, -15, 29, 49, 40,
    0, 0, 0, 0, 0, 0, 0, 0
};
int PSQ_TABLE_KNIGHT_EG[NSQUARES] = {
    -20, -40, -19, 0, 4, -25, -37, -108,
    -50, 6, 2, 0, -8, -5, 15, -34,
    -38, 17, 18, 30, 22, -1, -13, -42,
    -5, 31, 45, 27, 38, 29, 11, -11,
    21, 34, 40, 38, 40, 38, 31, 4,
    14, 32, 35, 35, -9, 4, 24, 2,
    -1, 19, 24, 39, 33, 0, 0, -11,
    -32, 43, 23, 26, 13, 45, 43, -69
};
int PSQ_TABLE_BISHOP_EG[NSQUARES] = {
    1, -12, -3, 11, -4, 14, -22, -25,
    2, -7, -3, 8, 0, 8, -3, -8,
    11, 11, 10, 10, 24, -4, 14, 24,
    23, 6, 12, 12, 6, 26, -1, 9,
    19, 8, 8, 11, 12, 8, 19, -3,
    3, 28, 22, 7, 43, 0, 20, 24,
    12, 0, 40, 47, 23, 34, 23, 8,
    18, 32, 24, 28, 13, 17, 15, 18
};
int PSQ_TABLE_ROOK_EG[NSQUARES] = {
    25, 16, 19, 16, -1, 15, 2, 1,
    15, 26, 21, 8, 11, -3, -22, -1,
    31, 27, 24, 12, 2, 11, -23, -22,
    41, 52, 41, 35, 29, 43, 12, 15,
    44, 49, 51, 44, 35, 46, 28, 26,
    51, 48, 52, 37, 14, 26, 21, 30,
    39, 46, 29, 45, 25, 24, 26, 4,
    30, 36, 39, 35, 48, 47, 47, 31
};
int PSQ_TABLE_QUEEN_EG[NSQUARES] = {
    -68, -73, -82, -21, -70, -38, -52, -12,
    -34, -31, -10, -31, -10, -102, -75, -44,
    -5, -4, 0, -13, 4, 13, 7, -32,
    39, 43, 4, 15, 24, 44, 40, 56,
    2, 54, 38, 42, 39, 103, 104, 90,
    8, 17, 68, 82, 109, 45, 74, 77,
    7, 71, 80, 129, 112, 60, 91, 57,
    13, 16, 37, 36, 58, 76, 3, 4
};
int PSQ_TABLE_KING_EG[NSQUARES] = {
    -95, -62, -29, -23, -55, -29, -69, -81,
    -18, -11, -3, 10, 3, -1, -19, -33,
    -48, 5, 5, 19, 25, 13, 8, -2,
    -12, 12, 31, 40, 39, 41, 26, 2,
    26, 60, 65, 60, 70, 64, 57, 56,
    29, 59, 68, 69, 67, 77, 93, 41,
    -34, 72, 76, 58, 68, 97, 118, 33,
    -100, -35, 0, 31, 36, 26, 26, -80
};
int KNIGHT_MATERIAL_VALUE_MG = 344;
int BISHOP_MATERIAL_VALUE_MG = 348;
int ROOK_MATERIAL_VALUE_MG = 488;
int QUEEN_MATERIAL_VALUE_MG = 1184;
int KNIGHT_MATERIAL_VALUE_EG = 339;
int BISHOP_MATERIAL_VALUE_EG = 339;
int ROOK_MATERIAL_VALUE_EG = 600;
int QUEEN_MATERIAL_VALUE_EG = 1124;
int KING_ATTACK_SCALE_MG = 27;
int KING_ATTACK_SCALE_EG = 1;

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
#ifndef UCI_H
#define UCI_H

#include <stdbool.h>

#include "chess.h"

/*
 * Handle an UCI command.
 *
 * @param state The game state object.
 * @param cmd The command to handle.
 * @param stop Flag set if the engine should stop after this command.
 * @return Returns true if the command was handled, false otherwise.
 */
bool uci_handle_command(struct gamestate *state, char *cmd, bool *stop);

/*
 * Function called during search to check if input has arrived.
 *
 * @param worker The worker.
 * @return Returns true if the current search should be stopped.
 */
bool uci_check_input(struct search_worker *worker);

/*
 * Send information about the principle variation.
 *
 * @param worker The worker
 * @param pv The pv.
 * @param depth The depth.
 * @param seldepth The selctive depth.
 * @param score The PV score.
 * @param nodes The number of searched nodes.
 */
void uci_send_pv_info(struct search_worker *worker, struct pv *pv, int depth,
                      int seldepth, int score, uint32_t nodes);

/*
 * Send information about the move currently being searched.
 *
 * @param worker The worker.
 */
void uci_send_move_info(struct search_worker *worker);

#endif

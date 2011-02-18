/*
 *
 * Tic-Tac-Toe with a "minimax" implementation.
 *
 * This is actually pretty close to a "generic game player". 
 * Chess could be a possibility (forget about having all states, tho).
 *
 * But could be used for generic board games ;)
 *
 *
 * libttt.h: "Core" definitions for the game library.
 *
 *
 * NOTES:
 *
 *		1.- Best viewed with tabstop=3.
 *		2.- This is a nice thing to write in C++ or Java ;)
 *
 *
 * TODO:
 *		1.- Maybe wrap-around players. Could be better.
 *
 *
 * Author:
 *			marcelog@gmail.com
 *
 *
 * "Talk sense to a fool and he calls you foolish."
 *			-- Euripides
 *
 */
#ifndef	_LIBTTT_H_
#define	_LIBTTT_H_

#define	DEBUG					0

/* This game has 2 players. */
typedef enum
{
	PLAYER_1,
	PLAYER_2
} player_t;

/* This game can be either won, lost, or tied. */
typedef enum
{
	STATUS_TIE,
	STATUS_WIN,
	STATUS_LOSE,
	STATUS_UNKNOWN
} status_t;

static const char *STATUS_STR[STATUS_UNKNOWN + 1] = {
	"DRAW", "WIN", "LOSE", "UNKNOWN"
};

/* This game has '9' cells */
#define	CELL_NUMBER		9
typedef	char	cell_t;

/* Each 'cell' can be either 'empty' or used by one of the player's 'tips' */
typedef enum
{
	TIP_NONE,
	TIP_P1,
	TIP_P2
} tip_t;

/*
 * Each 'state' of this game contains:
 * a) the board (9 cells).
 * b) the status relative to the starting player can be one of:
 *		b.1) WIN, LOSE, TIE (if this is a terminal state)
 *		b.2) UNKNOWN (not a terminal state).
 * c) an array of child(ren) state(s) (if not a terminal state).
 * d) who moves.
 * e) an id for easy access.
  */
typedef struct state_t
{
	long id;
	status_t status;
	cell_t cells[CELL_NUMBER];
	long children;
	struct state_t *childFirst;
	player_t moves;
} state_t;

/* External declarations (these go in the .so). */
extern status_t solveState(state_t *s);
extern void dumpState(state_t *s);
extern status_t checkState(state_t *s);
extern int makeMove(state_t *s, long cell);
extern void freeState(state_t *s);
extern int stateCompare(state_t *s1, state_t *s2);

#endif

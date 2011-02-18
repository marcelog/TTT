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
 * libttt.c: "Core" code for game library.
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
 *			dronkit@kernelfusion.com.ar
 *
 * Thanks to drakorg (drakorg@gmail.com) for 
 * helping me out with solveState() ;) (you, prick!)
 *
 *
 * "The trouble with doing something right the first time is that nobody
 *  appreciates how difficult it was."
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
#include	"libttt.h"

#define		ret		return

/* Forward declarations. */
void dumpState(state_t *s);
void freeState(state_t *s);
status_t checkState(state_t *s);
int stateCompare(state_t *s1, state_t *s2);
static long countMoves(state_t *s);
static long getFreeCell(state_t *s, int n);
int makeMove(state_t *s, long cell);
static int makeFirstMove(state_t *s);
static long possibleMoves(state_t *s);
status_t solveState(state_t *s);
static void log(const char *fmt, ...);

/*
 * Global variables.
 */
static long id = 1;

/*
 * Code starts here.
 */

/* Dumps a state to console. */
void
dumpState(state_t *s)
{
	register long i = 1;
	fprintf(stdout, "State: #%ld (%s)\n", s->id, STATUS_STR[s->status]);
	for(; i <= CELL_NUMBER; i++)
	{
		if(s->cells[i-1] == TIP_NONE)
			fprintf(stdout, "     |");
		else if(s->cells[i-1] == TIP_P1)
			fprintf(stdout, "  X  |");
		else if(s->cells[i-1] == TIP_P2)
			fprintf(stdout, "  O  |");
		if(!(i % 3))
		{
			fprintf(stdout, "\n-----+-----+-----\n");
		}
	}

}

/* Frees all memory used by a state and all its descendants. */
void
freeState(state_t *s)
{
	register long i;

	/* Check arguments. */
	if(s == NULL)
		ret;

	if(!s->children)
		ret;

	/* Free each child. */	
	for(i = 0; i < s->children; i++)
		freeState((state_t *)&s->childFirst[i]);

	/* Free memory used by children array. */
	free(s->childFirst);

	/* Done. */
	ret;
}

/*
 * Evaluates a given state. As a result, a state can be either:
 *		a) WON / LOST / STATUS_TIED (relative to the starting player).
 *		b) STATUS_UNKNOWN (not a terminal state).
 *		c) -1 on error ;)
 */
status_t
checkState(state_t *s)
{
	/* Assume not terminal state for now. */
	status_t r = STATUS_UNKNOWN;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	/* If 0 possible moves, assume STATUS_TIE for now. */
	if(!countMoves(s))
		r = STATUS_TIE;
	
	/* Check if one of the players won. */
	if(s->cells[0] != TIP_NONE)
	{
		if((s->cells[0] == s->cells[1]) && (s->cells[1] == s->cells[2]))
		{
			if(s->cells[0] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
		if((s->cells[0] == s->cells[3]) && (s->cells[3] == s->cells[6]))
		{
			if(s->cells[0] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
		if((s->cells[0] == s->cells[4]) && (s->cells[4] == s->cells[8]))
		{
			if(s->cells[0] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
	}
	if(s->cells[1] != TIP_NONE)
	{
		if((s->cells[1] == s->cells[4]) && (s->cells[4] == s->cells[7]))
		{
			if(s->cells[1] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
	}
	if(s->cells[2] != TIP_NONE)
	{
		if((s->cells[2] == s->cells[5]) && (s->cells[5] == s->cells[8]))
		{
			if(s->cells[2] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
		if((s->cells[2] == s->cells[4]) && (s->cells[4] == s->cells[6]))
		{
			if(s->cells[2] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
	}
	if(s->cells[3] != TIP_NONE)
	{
		if((s->cells[3] == s->cells[4]) && (s->cells[4] == s->cells[5]))
		{
			if(s->cells[3] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
	}
	if(s->cells[6] != TIP_NONE)
	{
		if((s->cells[6] == s->cells[7]) && (s->cells[7] == s->cells[8]))
		{
			if(s->cells[6] == TIP_P1)
				ret STATUS_WIN;
			else
				ret STATUS_LOSE;
		}
	}

	/* Done. */
	ret r;
}

/*
 * Compare two states.
 * Two states are considered equal if:
 *		a) the board disposition is the same.
 *				AND
 * 	b) the player who moves is the same.
 *
 * 1 if states are equal.
 * 0 if not.
 */
int
stateCompare(state_t *s1, state_t *s2)
{
	register long i = 0;

	/* Check arguments. */
	if(s1 == NULL || s2 == NULL)
		ret -1;

	for(; i < CELL_NUMBER; i++)
		if(s1->cells[i] != s2->cells[i])
			ret 0;
	ret s1->moves != s2->moves ? 0 : 1;
}


/*
 * Counts possible moves in a given 'state'. It will actully count
 * "free" or "empty" 'cells'.
 * -1 on error.
 */
static long
countMoves(state_t *s)
{
	register long i = 0;
	register long j = 0;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	/* Traverse board and look for possible moves. */
	for(; i < CELL_NUMBER; i++)
		if(s->cells[i] == TIP_NONE)
			j++;

	/* Done. */
	ret j;
}

/*
 * Returns the n'th free cell (a number between 1-CELL_NUMBER.
 * n should be 1<= n <= CELL_NUMBER
 * 0 = no free cells.
 * -1 = error.
 */
static long
getFreeCell(state_t *s, int n)
{
	register long i = 0;
	long j = 1;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	if(!n || n > CELL_NUMBER)
		ret -1;
	
	/* Traverse board. */
	for(; i < CELL_NUMBER; i++)
		if(s->cells[i] == TIP_NONE)
		{
			if(j == n)
				ret i + 1;
			else
				j++;
		}

	/* If got here, no free cells were found. */
	ret 0;
}

/*
 * Makes a move for a given player.
 * -1 on error.
 * 0 on success.
 */
int
makeMove(state_t *s, long cell)
{
	player_t p;

	/* Check arguments. */
	if(s == NULL || cell > CELL_NUMBER)
		ret -1;

	/* Move. */
	p = s->moves;
	if(s->moves == PLAYER_1)
	{
		s->cells[cell - 1] = TIP_P1;
		s->moves = PLAYER_2;
	} else {
		s->cells[cell - 1] = TIP_P2;
		s->moves = PLAYER_1;
	}
	if(DEBUG)
		log("%s:%d: \t\t\tMoving player: %ld, next: %ld \n", 
			__FILE__, __LINE__, p + 1, s->moves + 1);

	/* Done. */
	ret 0;
}

/*
 * Makes the first available move for a given player.
 * 1 success.
 * 0 no possible move.
 * -1 on error.
 */
static int
makeFirstMove(state_t *s)
{
	register long i = 0;
	register long k = 0;
	long j;
	long freechild;
	int done = 0;
	state_t foostate;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	/* Search children for an available slot. */
	memset((void *)&foostate, 0, sizeof(state_t));
	
	for(k = 0; k < s->children; k++)
		if(stateCompare(&foostate, (state_t *)&s->childFirst[k]))
		{
			freechild = k;
			break;
		}

	/* All children are already computed. */
	if(k == s->children)
		ret 0;

	if(DEBUG)
		log("%s:%d: \t\tAvailable slot found at %ld\n",
			__FILE__, __LINE__, freechild);

	/* Try a move. */
	j = getFreeCell(s, freechild + 1);

	if(DEBUG)
		log("%s:%d: \t\tAvailable cell found %ld\n", __FILE__, __LINE__, j);

	/* No possible moves? */
	if(!j)
		ret 0;

	/* Do the first move. */
	memcpy((void *)&foostate, (void *)s, sizeof(state_t));
	makeMove(&foostate, j);
	id++;
	foostate.id = id;
	foostate.children = 0;
	foostate.childFirst = NULL;

	/* Copy this new move to the list. */
	memcpy((void *)&s->childFirst[freechild], (void *)&foostate, sizeof(state_t));

	/* Done. */
	if(DEBUG)
	{
		log("%s:%d: \t\tMove for player %ld: s=%ld at %ld at c=%ld\n", 
			__FILE__, __LINE__, s->moves + 1, id, j, freechild);
		dumpState(&foostate);
	}
	ret 1;
}

/*
 * Generates all child moves for a given state.
 */
static long
possibleMoves(state_t *s)
{
	long r;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	/* See how many possible moves we have (children). */
	s->children = countMoves(s);

	if(DEBUG)
		log("%s: %d: \tThis state has %ld potential moves\n",
			__FILE__, __LINE__, s->children);

	r = sizeof(state_t) * s->children;

	/* Allocate memory for pointer state array. */
	s->childFirst = (state_t *)malloc(r);
	if(s->childFirst == NULL)
		ret -1;
	memset((void *)s->childFirst, 0, r);

	if(DEBUG)
		log("%s:%d: \tGenerating %ld moves...\n",
			__FILE__, __LINE__, s->children);

	/* Effectively make the moves. */
	while(makeFirstMove(s));// if(DEBUG) getchar();

	if(DEBUG)
		log("%s:%d: \tAll moves done.\n", __FILE__, __LINE__);

	/* Done. */
	ret 0;
}

/*
 * This funcion will "map" a game from a given state.
 * It is basically a "state" generator.
 */
status_t
solveState(state_t *s)
{
	state_t *c;
	status_t status;
	register long i = 0;

	/* Check arguments. */
	if(s == NULL)
		ret -1;

	/* See if this is a terminal state already. */
	s->status = checkState(s);
	if(DEBUG)
	{
		log("%s:%d: Solving state %ld (%s) player %d moves\n",
			__FILE__, __LINE__, s->id, STATUS_STR[s->status], s->moves + 1);
		dumpState(s);
	}

	if(s->status != STATUS_UNKNOWN)
		ret s->status;
	
	/* It's not. Generate "child" moves then. */
	possibleMoves(s);

	/* Solve children */
	for(i = 0; i < s->children; i++)
	{
		c = (state_t *)&s->childFirst[i];
		status = solveState(c);
		/* Propagate the status for this child to the parent. */
		if(s->moves == PLAYER_1)
		{
			switch(s->status)
			{
				case STATUS_WIN:
					s->status = STATUS_WIN;
					break;
				case STATUS_TIE:
					if(status == STATUS_WIN)
						s->status = STATUS_WIN;
					else
						s->status = STATUS_TIE;
					break;
				case STATUS_LOSE:
				default:
					s->status = status;
					break;
			}
		} else {
			switch(s->status)
			{
				case STATUS_LOSE:
					s->status = STATUS_LOSE;
					break;
				case STATUS_TIE:
					if(status == STATUS_LOSE)
						s->status = STATUS_LOSE;
					else
						s->status = STATUS_TIE;
					break;
				case STATUS_WIN:
				default:
					s->status = status;
					break;
			}
		}
	}

	/* Done. */
	ret s->status;
}

/* This function just logs with a format. */
static void
log(const char *fmt, ...)
{
	char buffer[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);
	fprintf(stdout, "%s", buffer);
	ret;
}

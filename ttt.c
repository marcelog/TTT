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
 * ttt.c: console implementation for it ;)
 *
 *
 * NOTES:
 *
 *		1.- Best viewed with tabstop=3.
 *
 *
 * Author:
 *			marcelog@gmail.com
 *
 *
 *  "The two most common things in the universe are hydrogen and
 *  stupidity."
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	"./libttt.h"

/*
 * Types and constant definitions.
 */
#define	ret			return
#define	MSG_IWIN		"I WIN... big surprise ;)"
#define	MSG_DRAW		"DRAW. You got lucky."

/*
 * Code starts here.
 */

/* This function outputs all children recursively for a given state. */
static void
traceState(state_t *s)
{
	register long i = 0;
	fprintf(stdout, "---- State #%ld (%s) p%d moves.---- \n", s->id, STATUS_STR[s->status], s->moves + 1);
	dumpState(s);
	for(i = 0; i < s->children; i++)
		traceState((state_t *)&s->childFirst[i]);
	ret;
}

/* This function selects the best move available for me. */
static state_t *
mymove(state_t *cs, status_t s)
{
	register long i = 0;

	/* Find best alternatives (WIN). */
	for(i = 0; i < cs->children; i++)
		if(((state_t *)(&cs->childFirst[i]))->status == s)
			ret(state_t *)&cs->childFirst[i];

	/* Find second best alternatives (DRAW). */
	for(i = 0; i < cs->children; i++)
		if(((state_t *)(&cs->childFirst[i]))->status == STATUS_TIE)
			ret (state_t *)&cs->childFirst[i];
	
	/* Unreached. */
	ret NULL;
}

/* This function makes the move for the 'inferior' human player ;) */
static state_t *
humanmove(state_t *cs)
{
	register long i = 0;
	int c = 0;
	state_t foostate;
	
	/* Get move from human. */
	do
	{
		fprintf(stdout, "\rEnter your move(1-9): ");
		c = getchar() - 48;
		if(c > 0)
			if(c <= 9)
				if(cs->cells[c-1] == TIP_NONE)
					break;
	} while(1);

	/* Make the move in our virtual state. */
	memcpy((void *)&foostate, (void *)cs, sizeof(state_t));
	makeMove(&foostate, c);

	/* Search for this particular state in current state's children. */
	for(; i < cs->children; i++)
		if(stateCompare(&foostate, (state_t *)&(cs->childFirst[i])))
			ret (state_t *)&(cs->childFirst[i]);

	/* Unreached. */
	ret NULL;
}

/*
 * Main Entry point.
 */
int
main(int argc, char *argv[])
{
	register long i = 0;
	state_t start_state;
	state_t *cs;
	status_t sstatus;
	int ans;

	/* Initial start state. */
	start_state.id = 1;
	start_state.status = STATUS_UNKNOWN;
	start_state.childFirst = NULL;
	start_state.moves = PLAYER_1;
	for(; i < CELL_NUMBER; i++)
		start_state.cells[i] = TIP_NONE;

	/* Generate game-map. */
	fprintf(stdout, "Generating all states...\n");

	/* Solve our little game here ... ;) */
	solveState(&start_state);
	cs = &start_state;

	if(DEBUG)
		traceState(cs);

	/* Main loop. */
	do
	{
		do
		{
			fprintf(stdout, "\rWould you like to go first? (y/n): ");
			ans = tolower(getchar());
		} while(ans != 'y' && ans != 'n');

		/* If we start, we'll look for 'WIN' situations. 'LOSE' if not. */
		if(ans == 'y')
		{
			i = 1;
			sstatus = STATUS_LOSE;
		} else {
			i = 0;
			sstatus = STATUS_WIN;
		}
		/* Play! */
		cs = &start_state;
		while(checkState(cs) == STATUS_UNKNOWN)
		{
			/* Turn players. */
			if(i)
			{
				/* Redraw board. */
				dumpState(cs);
				i = 0;
				cs = humanmove(cs);
			} else {
				i = 1;
				cs = mymove(cs, sstatus);
			}
		}

		/* This is it. */
		dumpState(cs);
		if(cs->status == STATUS_TIE)
			fprintf(stdout, "%s\n", MSG_DRAW);
		else
			fprintf(stdout, "%s\n", MSG_IWIN);

		/* Rematch?? ;) */
		do
		{
			fprintf(stdout, "\rWant to play again? (y/n): ");
			ans = tolower(getchar());
		} while(ans != 'y' && ans != 'n');
	} while(ans == 'y');

	/* Free memory. */
	freeState(&start_state);

	/* Get out. Anti "return-in-libc" technique ;) */
	exit(EXIT_SUCCESS);

	/* Not reachable. Some stupid compilers may complain if not present. */
	ret EXIT_SUCCESS;
}

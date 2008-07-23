/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Process scheduler.
 */

#include "tinsel/sched.h"

#include "common/util.h"

namespace Tinsel {


/** list of all processes */
static PROCESS *processList = 0;

/** active process list - also saves scheduler state */
static PROCESS active;

/** pointer to free process list */
static PROCESS *pFreeProcesses = 0;

/** the currently active process */
static PROCESS *pCurrent = 0;

#ifdef DEBUG
// diagnostic process counters
static int numProcs = 0;
static int maxProcs = 0;
#endif

/**
 * Called from ProcessKill() to enable other resources
 * a process may be allocated to be released.
 */
static VFPTRPP pRCfunction = 0;


/**
 * Kills all processes and places them on the free list.
 */
void InitScheduler(void) {

#ifdef DEBUG
	// clear number of process in use
	numProcs = 0;
#endif

	if (processList == NULL) {
		// first time - allocate memory for process list
		processList = (PROCESS *)calloc(NUM_PROCESS, sizeof(PROCESS));

		// make sure memory allocated
		if (processList == NULL) {
			error("Cannot allocate memory for process data");
		}

		// fill with garbage
		memset(processList, 'S', NUM_PROCESS * sizeof(PROCESS));
	}

	// no active processes
	pCurrent = active.pNext = NULL;

	// place first process on free list
	pFreeProcesses = processList;

	// link all other processes after first
	for (int i = 1; i < NUM_PROCESS; i++) {
		processList[i - 1].pNext = processList + i;
	}

	// null the last process
	processList[NUM_PROCESS - 1].pNext = NULL;
}

void FreeProcessList(void) {
	if (processList) {
		free(processList);
		processList = NULL;
	}
}


#ifdef	DEBUG
/**
 * Shows the maximum number of process used at once.
 */
void ProcessStats(void) {
	printf("%i process of %i used.\n", maxProcs, NUM_PROCESS);
}
#endif


/**
 * Give all active processes a chance to run
 */
void Scheduler(void) {
	// start dispatching active process list
	PROCESS *pPrevProc = &active;
	PROCESS *pProc = active.pNext;
	while (pProc != NULL) {
		if (--pProc->sleepTime <= 0) {
			// process is ready for dispatch, activate it
			pCurrent = pProc;
			pProc->coroAddr(pProc->state);
			pCurrent = NULL;
			if (!pProc->state || pProc->state->_sleep <= 0) {
				// Coroutine finished
				ProcessKill(pProc);
				pProc = pPrevProc;
			} else {
				pProc->sleepTime = pProc->state->_sleep;
			}
		}
		pPrevProc = pProc;
		pProc = pProc->pNext;
	}
}


/**
 * Creates a new process.
 *
 * @param pid	process identifier
 * @param CORO_ADDR	coroutine start address
 * @param pParam	process specific info
 * @param sizeParam	size of process specific info
 */
PROCESS *CoroutineInstall(int pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam) {
	PROCESS *pProc;

	// get a free process
	pProc = pFreeProcesses;

	// trap no free process
	assert(pProc != NULL); // Out of processes

#ifdef DEBUG
	// one more process in use
	if (++numProcs > maxProcs)
		maxProcs = numProcs;
#endif

	// get link to next free process
	pFreeProcesses = pProc->pNext;

	if (pCurrent != NULL) {
		// place new process before the next active process
		pProc->pNext = pCurrent->pNext;

		// make this new process the next active process
		pCurrent->pNext = pProc;
	} else {	// no active processes, place process at head of list
		pProc->pNext = active.pNext;
		active.pNext = pProc;
	}

	// set coroutine entry point
	pProc->coroAddr = coroAddr;

	// clear coroutine state
	pProc->state = 0;

	// wake process up as soon as possible
	pProc->sleepTime = 1;

	// set new process id
	pProc->pid = pid;

	// set new process specific info
	if (sizeParam) {
		assert(sizeParam > 0 && sizeParam <= PARAM_SIZE);

		// set new process specific info
		memcpy(pProc->param, pParam, sizeParam);
	}


	// return created process
	return pProc;
}

/**
 * Kills the specified process.
 *
 * @param pKillProc	which process to kill
 */
void ProcessKill(PROCESS *pKillProc) {
	PROCESS *pProc, *pPrev;	// process list pointers

	// make sure a valid process pointer
	assert(pKillProc >= processList && pKillProc <= processList + NUM_PROCESS - 1);
	
	// can not kill the current process using ProcessKill !
	assert(pCurrent != pKillProc);

#ifdef DEBUG
	// one less process in use
	--numProcs;
	assert(numProcs >= 0);
#endif

	// search the active list for the process
	for (pProc = active.pNext, pPrev = &active; pProc != NULL; pPrev = pProc, pProc = pProc->pNext) {
		if (pProc == pKillProc) {
			// found process in active list

			// Free process' resources
			if (pRCfunction != NULL)
				(pRCfunction)(pProc);

			delete pProc->state;

			// make prev point to next to unlink pProc
			pPrev->pNext = pProc->pNext;

			// link first free process after pProc
			pProc->pNext = pFreeProcesses;

			// make pProc the first free process
			pFreeProcesses = pProc;

			return;
		}
	}

	// process not found in active list if we get to here
	error("ProcessKill(): tried to kill a process not in the list of active processes");
}



/**
 * Returns a pointer to the currently running process.
 */
PROCESS *CurrentProcess(void) {
	return pCurrent;
}

char *ProcessGetParamsSelf() {
	PROCESS *pProc = pCurrent;

	// make sure a valid process pointer
	assert(pProc >= processList && pProc <= processList + NUM_PROCESS - 1);

	return pProc->param;
}

/**
 * Returns the process identifier of the specified process.
 *
 * @param pProc	which process
 */
int ProcessGetPID(PROCESS *pProc) {
	// make sure a valid process pointer
	assert(pProc >= processList && pProc <= processList + NUM_PROCESS - 1);

	// return processes PID
	return pProc->pid;
}

/**
 * Kills any process matching the specified PID. The current
 * process cannot be killed.
 *
 * @param pidKill	process identifier of process to kill
 * @param pidMask	mask to apply to process identifiers before comparison
 * @return The number of processes killed is returned.
 */
int KillMatchingProcess(int pidKill, int pidMask) {
	int numKilled = 0;
	PROCESS *pProc, *pPrev;	// process list pointers

	for (pProc = active.pNext, pPrev = &active; pProc != NULL; pPrev = pProc, pProc = pProc->pNext) {
		if ((pProc->pid & pidMask) == pidKill) {
			// found a matching process

			// dont kill the current process
			if (pProc != pCurrent) {
				// kill this process
				numKilled++;

				// make prev point to next to unlink pProc
				pPrev->pNext = pProc->pNext;

				// link first free process after pProc
				pProc->pNext = pFreeProcesses;

				// make pProc the first free process
				pFreeProcesses = pProc;

				// set to a process on the active list
				pProc = pPrev;
			}
		}
	}

#ifdef DEBUG
	// adjust process in use
	numProcs -= numKilled;
	assert(numProcs >= 0);
#endif

	// return number of processes killed
	return numKilled;
}



/**
 * Set pointer to a function to be called by ProcessKill().
 * 
 * May be called by a resource allocator, the function supplied is
 * called by ProcessKill() to allow the resource allocator to free
 * resources allocated to the dying process.
 *
 * @param pFunc	Function to be called by ProcessKill()
 */
void SetResourceCallback(VFPTRPP pFunc) {
	pRCfunction = pFunc;
}

} // end of namespace Tinsel

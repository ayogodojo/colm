/*
 *  Copyright 2001-2012 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _FSMCODEGEN_H
#define _FSMCODEGEN_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "keyops.h"
#include "compiler.h"
#include "redfsm.h"

using std::string;
using std::ostream;

/* Integer array line length. */
#define IALL 8

/* Forwards. */
struct RedFsm;
struct RedState;
struct GenAction;
struct NameInst;
struct RedAction;
struct LongestMatch;
struct TokenInstance;
struct InlineList;
struct InlineItem;
struct NameInst;
struct FsmCodeGen;

typedef unsigned long ulong;
typedef unsigned char uchar;


/*
 * The interface to the parser
 */

std::ostream *openOutput( char *inputFile );

inline string itoa( int i )
{
	char buf[16];
	sprintf( buf, "%i", i );
	return buf;
}

/*
 * class FsmCodeGen
 */
class FsmCodeGen
{
public:
	FsmCodeGen( ostream &out, RedFsm *redFsm, fsm_tables *fsmTables );

protected:

	string FSM_NAME();
	string START_STATE_ID();
	ostream &ACTIONS_ARRAY();
	string GET_WIDE_KEY();
	string GET_WIDE_KEY( RedState *state );
	string TABS( int level );
	string KEY( Key key );
	string LDIR_PATH( char *path );
	void ACTION( ostream &ret, GenAction *action, int targState, bool inFinish );
	void CONDITION( ostream &ret, GenAction *condition );
	string ALPH_TYPE();
	string WIDE_ALPH_TYPE();
	string ARRAY_TYPE( unsigned long maxVal );

	string ARR_OFF( string ptr, string offset );
	string CAST( string type );
	string UINT();
	string GET_KEY();

	string ACCESS() { return "pdaRun->"; }

	string P() { return ACCESS() + "p"; }
	string PE() { return ACCESS() + "pe"; }
	string DATA_EOF() { return ACCESS() + "eof"; }

	string CS();
	string TOP() { return ACCESS() + "top"; }
	string TOKSTART() { return ACCESS() + "tokstart"; }
	string TOKEND() { return ACCESS() + "tokend"; }
	string BLOCK_START() { return ACCESS() + "start"; }
	string TOKLEN() { return ACCESS() + "toklen"; }
	string ACT() { return ACCESS() + "act"; }
	string MATCHED_TOKEN() { return ACCESS() + "matched_token"; }

	string DATA_PREFIX();

	string START() { return DATA_PREFIX() + "start"; }
	string ERROR() { return DATA_PREFIX() + "error"; }
	string FIRST_FINAL() { return DATA_PREFIX() + "first_final"; }

	string ENTRY_BY_REGION() { return DATA_PREFIX() + "entry_by_region"; }


	void INLINE_LIST( ostream &ret, InlineList *inlineList, 
		int targState, bool inFinish );
	void EXEC_TOKEND( ostream &ret, InlineItem *item, int targState, int inFinish );
	void EXECTE( ostream &ret, InlineItem *item, int targState, int inFinish );
	void LM_SWITCH( ostream &ret, InlineItem *item, int targState, int inFinish );
	void SET_ACT( ostream &ret, InlineItem *item );
	void INIT_TOKSTART( ostream &ret, InlineItem *item );
	void INIT_ACT( ostream &ret, InlineItem *item );
	void SET_TOKSTART( ostream &ret, InlineItem *item );
	void SET_TOKEND( ostream &ret, InlineItem *item );
	void GET_TOKEND( ostream &ret, InlineItem *item );
	void SUB_ACTION( ostream &ret, InlineItem *item, int targState, bool inFinish );
	void LM_ON_LAST( ostream &ret, InlineItem *item );
	void LM_ON_NEXT( ostream &ret, InlineItem *item );
	void LM_ON_LAG_BEHIND( ostream &ret, InlineItem *item );
	void EXEC_TOKEND( ostream &ret );
	void EMIT_TOKEN( ostream &ret, LangEl *token );

	string ERROR_STATE();
	string FIRST_FINAL_STATE();

	string PTR_CONST();
	ostream &OPEN_ARRAY( string type, string name );
	ostream &CLOSE_ARRAY();
	ostream &STATIC_VAR( string type, string name );

	string CTRL_FLOW();

	unsigned int arrayTypeSize( unsigned long maxVal );

public:
	ostream &out;
	RedFsm *redFsm;
	fsm_tables *fsmTables;
	int codeGenErrCount;

	/* Write options. */
	bool dataPrefix;
	bool writeFirstFinal;
	bool writeErr;
	bool skipTokenLabelNeeded;

	std::ostream &TO_STATE_ACTION_SWITCH();
	std::ostream &FROM_STATE_ACTION_SWITCH();
	std::ostream &ACTION_SWITCH();
	std::ostream &STATE_GOTOS();
	std::ostream &TRANSITIONS();
	std::ostream &EXEC_FUNCS();

	unsigned int TO_STATE_ACTION( RedState *state );
	unsigned int FROM_STATE_ACTION( RedState *state );

	std::ostream &TO_STATE_ACTIONS();
	std::ostream &FROM_STATE_ACTIONS();

	void emitCondBSearch( RedState *state, int level, int low, int high );
	void STATE_CONDS( RedState *state, bool genDefault ); 

	void emitSingleSwitch( RedState *state );
	void emitRangeBSearch( RedState *state, int level, int low, int high );

	std::ostream &EXIT_STATES();
	std::ostream &TRANS_GOTO( RedTrans *trans, int level );
	std::ostream &FINISH_CASES();

	void writeIncludes();
	void writeData();
	void writeInit();
	void writeExec();
	void writeCode();
	void writeMain( long activeRealm );

protected:
	bool useAgainLabel();

	/* Called from GotoCodeGen::STATE_GOTOS just before writing the gotos for
	 * each state. */
	bool IN_TRANS_ACTIONS( RedState *state );
	void GOTO_HEADER( RedState *state );
	void STATE_GOTO_ERROR();

	/* Set up labelNeeded flag for each state. */
	void setLabelsNeeded();
};

#endif /* _FSMCODEGEN_H */

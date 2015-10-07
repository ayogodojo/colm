/*
 *  Copyright 2015 Adrian Thurston <thurston@complang.org>
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

#include <iostream>
#include <iomanip>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <sstream>

#include "global.h"
#include "avltree.h"
#include "compiler.h"
#include "parser.h"
#include "parsetree.h"
#include "mergesort.h"
#include "redbuild.h"
#include "pdacodegen.h"
#include "fsmcodegen.h"
#include "pdarun.h"
#include "colm.h"
#include "pool.h"
#include "struct.h"

void Compiler::writeCommitStub()
{
	*outStream <<
		"void commit_forward_recurse( program_t *prg, tree_t **root,\n"
		"		struct pda_run *pda_run, parse_tree_t *pt )\n"
		"{\n"
		"	commit_clear_parse_tree( prg, root, pda_run, pt->child );\n"
		"}\n"
		"\n"
		"long commit_union_sz() { return 0; }\n"
		"\n";
	;
}

void Compiler::writeHostItemList( LangEl *prodName, int prodNum,
		const ReduceTextItemList &list )
{
	for ( ReduceTextItemList::Iter i = list; i.lte(); i++ ) {
		switch ( i->type ) {
			case ReduceTextItem::RhsRef: {
				String name( i->txt.data + 1, i->txt.length() - 1 );

				/* Find the field in the rhsVal using capture field. */
				ObjectField *field = prodName->objectDef->rootScope->findField( name );
				if ( field != 0 ) {
					for ( Vector<RhsVal>::Iter r = field->rhsVal;
							r.lte(); r++ )
					{
						if ( r->prodEl->production->prodNum == prodNum ) {
							if ( r->prodEl->langEl->type == LangEl::Term ) {
								*outStream << "(get_rhs_el_kid( prg, kid->tree, "
										<< r->prodEl->pos << ")->tree->tokdata)";
							}
							else {
								*outStream << "(&((commit_reduce_union*)"
										"(get_rhs_parse_tree( prg, lel, " <<
										r->prodEl->pos << ")+1))->" <<
										r->prodEl->langEl->fullName << ")";
							}
						}
					}
				}
					
				break;
			}
			case ReduceTextItem::LhsRef:
				*outStream << "(&((commit_reduce_union*)(lel+1))->" <<
						prodName->fullName << ")";
				break;
					
			case ReduceTextItem::Txt:
				*outStream << i->txt;
				break;
		}
	}
}

void Compiler::writeCommit()
{
	*outStream <<
		"#include <colm/pdarun.h>\n"
		"#include <colm/debug.h>\n"
		"#include <colm/bytecode.h>\n"
		"#include <colm/config.h>\n"
		"#include <colm/defs.h>\n"
		"#include <colm/input.h>\n"
		"#include <colm/tree.h>\n"
		"#include <colm/program.h>\n"
		"#include <colm/colm.h>\n"
		"\n"
		"#include <stdio.h>\n"
		"#include <stdlib.h>\n"
		"#include <string.h>\n"
		"#include <assert.h>\n"
		"\n"
		"#include <iostream>\n";

	for ( ReductionVect::Iter r = rootNamespace->reductions; r.lte(); r++ ) {
		for ( ReduceNonTermList::Iter rdi = (*r)->reduceNonTerms; rdi.lte(); rdi++ ) {
			*outStream <<
				"struct lel_" << rdi->nonTerm->uniqueType->langEl->fullName << "\n"
				"{";

			writeHostItemList( 0, 0, rdi->itemList );

			*outStream <<
				"};\n";
		}
	}

	*outStream << 
		"union commit_reduce_union\n"
		"{\n";

	for ( ReductionVect::Iter r = rootNamespace->reductions; r.lte(); r++ ) {
		for ( ReduceNonTermList::Iter rdi = (*r)->reduceNonTerms; rdi.lte(); rdi++ ) {
			LangEl *langEl = rdi->nonTerm->uniqueType->langEl;
			*outStream <<
				"	lel_" << langEl->fullName << " " << langEl->fullName << ";\n";
		}
	}

	*outStream <<
		"};\n"
		"\n";

	*outStream <<
		"long commit_union_sz() { return sizeof( commit_reduce_union ); }\n";

	*outStream <<
		"\n"
		"void commit_forward_recurse( program_t *prg, tree_t **root,\n"
		"		struct pda_run *pda_run, parse_tree_t *pt )\n"
		"{\n"
		"	tree_t **sp = root;\n"
		"\n"
		"	parse_tree_t *lel = pt;\n"
		"	kid_t *kid = pt->shadow;\n"
		"\n"
		"recurse:\n"
		"\n"
		"	if ( lel->child != 0 ) {\n"
		"		/* There are children. Must process all children first. */\n"
		"		vm_push_ptree( lel );\n"
		"		vm_push_kid( kid );\n"
		"\n"
		"		lel = lel->child;\n"
		"		kid = kid->tree->child;\n"
		"		while ( lel != 0 ) {\n"
		"			goto recurse;\n"
		"			resume:\n"
		"			lel = lel->next;\n"
		"			kid = kid->next;\n"
		"		}\n"
		"\n"
		"		kid = vm_pop_kid();\n"
		"		lel = vm_pop_ptree();\n"
		"	}\n"
		"\n"
		"	if ( !( lel->flags & PF_COMMITTED ) ) {\n"
		"		/* Now can execute the reduction action. */\n"
		"		switch ( kid->tree->id ) {\n";

	for ( ReductionVect::Iter r = rootNamespace->reductions; r.lte(); r++ ) {
		for ( ReduceActionList::Iter rdi = (*r)->reduceActions; rdi.lte(); rdi++ ) {
			int lelId = rdi->production->prodName->id;
			int prodNum = rdi->production->prodNum;

			*outStream <<
				"		case " << lelId << ": {\n"
				"			if ( kid->tree->prod_num == " << prodNum << " ) {\n";

			writeHostItemList( rdi->production->prodName, prodNum, rdi->itemList );

			*outStream <<
				"			}\n"
				"			break;\n"
				"		}\n";
		}
	}

	*outStream <<
		"		}\n"
		"	}\n"
		"\n"
		"	commit_clear_parse_tree( prg, sp, pda_run, lel->child );\n"
		"	lel->child = 0;\n"
		"\n"
		"	if ( sp != root )\n"
		"		goto resume;\n"
		"	pt->flags |= PF_COMMITTED;\n"
		"}\n"
		"\n";
}




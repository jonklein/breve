/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include "kernel.h"

#define BRMENUENTRYPOINTER(p) ((brMenuEntry*)BRPOINTER(p))

/*! \addtogroup InternalFunctions */
/*@{*/

/*!
	\brief Creates a new menu item for a given object.

	brMenuItem pointer menuItemNew(object owner, string method, string name)

	The method parameter is the method to be called when the menu
	item is activated.  The name parameter is the name attached to
	the menu item.
*/

int brIMenuItemNew( brEval args[], brEval *target, brInstance *i ) {
	brInstance *m;

	if ( !BRINSTANCE( &args[0] ) ) {
		slMessage( DEBUG_ALL, "addMenuItem called with uninitialized object\n" );
		return EC_ERROR;
	}

	m = BRINSTANCE( &args[0] );

	target->set( brAddMenuItem( m, BRSTRING( &args[1] ), BRSTRING( &args[2] ) ) );

	if ( i->engine->updateMenu )
		i->engine->updateMenu( m );

	return EC_OK;
}

/*!
	\brief Adds or removes a check for a menu item.

	void menuItemSetCheck(brMenuEntry pointer, int).

	The exact effect of adding a check mark to a menu item depends on
	the frontend implementation.
*/

int brIMenuItemSetCheck( brEval args[], brEval *target, brInstance *i ) {
	brMenuEntry *item = BRMENUENTRYPOINTER( &args[0] );

	if ( !item ) {
		slMessage( DEBUG_ALL, "menuItemSetCheck called with uninitialized menu item\n" );
		return EC_OK;
	}

	item->checked = BRINT( &args[1] );

	if ( item->instance && i->engine->updateMenu )
		i->engine->updateMenu( item->instance );

	return EC_OK;
}

/*!
	\brief Enables or disables a menu item.

	void menuItemSetEnabled(brMenuEntry pointer, int).

	The exact effect of enabled or disabling a menu item depends on the
	frontend implementation.  Some implementations may gray out the menu
	item, while others remove it from the menu completely.
*/

int brIMenuItemSetEnabled( brEval args[], brEval *target, brInstance *i ) {
	brMenuEntry *item = BRMENUENTRYPOINTER( &args[0] );

	if ( !item ) {
		slMessage( DEBUG_ALL, "enableMenuItem called with uninitialized menu item\n" );
		return EC_OK;
	}

	item->enabled = BRINT( &args[1] );

	if ( item->instance && i->engine->updateMenu )
		i->engine->updateMenu( item->instance );

	return EC_OK;
}

/*@}*/

void breveInitMenuFunctions( brNamespace *n ) {
	brNewBreveCall( n, "menuItemNew", brIMenuItemNew, AT_POINTER, AT_INSTANCE, AT_STRING, AT_STRING, 0 );
	brNewBreveCall( n, "menuItemSetEnabled", brIMenuItemSetEnabled, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "menuItemSetCheck", brIMenuItemSetCheck, AT_NULL, AT_POINTER, AT_INT, 0 );
}

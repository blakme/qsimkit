/**
 * QSimKit - MSP430 simulator
 * Copyright (C) 2013 Jan "HanzZ" Kaluza (hanzz.k@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#pragma once

#include <QString>
#include <QStringList>
#include <QChar>
#include <QRect>
#include <QList>
#include <stdint.h>

#include "QSimKit/MCU/MCU.h"

#include "DwarfExpression.h"

class DwarfLocationList;
class DwarfVariable;
class RegisterSet;
class Memory;

class DwarfVariable : public Variable {
	public:
		DwarfVariable(VariableType *type, const QString &name, DwarfLocationList *ll, DwarfExpression *expr);
		~DwarfVariable();

		QString getValue(RegisterSet *r, Memory *m, Subprogram *p, uint16_t pc);

	private:
		DwarfLocationList *m_ll;
		DwarfExpression *m_expr;
};



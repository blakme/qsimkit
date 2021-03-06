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

#include <stdint.h>
#include <string>

#include "CPU/Instructions/InstructionArgument.h"

namespace MSP430 {

class Memory;

class MemoryArgument : public InstructionArgument {
	public:
		MemoryArgument(Memory *mem, uint16_t address);
		virtual ~MemoryArgument();

		uint16_t get();
		uint16_t getBigEndian();
		void set(uint16_t value);
		void setBigEndian(uint16_t value);

		uint8_t getByte();
		void setByte(uint8_t value);

		void reinitialize(uint16_t address) { m_address = address; }

	private:
		Memory *m_mem;
		uint16_t m_address;
};

}

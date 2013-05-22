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

#include "Timer.h"
#include "CPU/Variants/Variant.h"
#include "CPU/Memory/Memory.h"
#include "CPU/Interrupts/InterruptManager.h"
#include <iostream>

#include "ACLK.h"
#include "SMCLK.h"

#define TIMER_STOPPED 0
#define TIMER_UP 1
#define TIMER_CONTINUOUS 2
#define TIMER_UPDOWN 3

namespace MCU {

Timer::Timer(InterruptManager *intManager, Memory *mem, Variant *variant,
			 ACLK *aclk, SMCLK *smclk, uint16_t tactl, uint16_t tar,
			 uint16_t taiv) :
m_intManager(intManager), m_mem(mem), m_variant(variant), m_source(0),
m_divider(1), m_aclk(aclk), m_smclk(smclk), m_up(true), m_tactl(tactl),
m_tar(tar), m_taiv(taiv) {

	m_mem->addWatcher(tactl, this);
	m_mem->addWatcher(taiv, this, true);

	m_intManager->addWatcher(m_variant->getTIMERA0_VECTOR(), this);
	m_intManager->addWatcher(m_variant->getTIMERA1_VECTOR(), this);

	reset();
}

Timer::~Timer() {

}

void Timer::addCCR(uint16_t tacctl, uint16_t taccr) {
	CCR ccr;
	ccr.tacctl = tacctl;
	ccr.taccr = taccr;
	m_ccr.push_back(ccr);
}

void Timer::checkCCRInterrupts(uint16_t tar) {
	for (int i = 1; i < m_ccr.size(); ++i) {
		uint16_t ccr = m_mem->getBigEndian(m_ccr[i].taccr);
		bool interrupt_enabled = m_mem->isBitSet(m_ccr[i].tacctl, 16);
		if (interrupt_enabled && ccr == tar) {
			m_mem->setBit(m_ccr[i].tacctl, 1, true);
			m_intManager->queueInterrupt(m_variant->getTIMERA1_VECTOR());
		}
	}
}

void Timer::changeTAR(uint8_t mode) {
	uint16_t tar = m_mem->getBigEndian(m_tar);
	uint16_t ccr0 = m_mem->getBigEndian(m_ccr[0].taccr);
	bool ccr0_interrupt_enabled = m_mem->isBitSet(m_ccr[0].tacctl, 16);
	bool taifg_interrupt_enabled = m_mem->isBitSet(m_tactl, 2);

	switch (mode) {
		case TIMER_STOPPED:
			break;
		case TIMER_UP:
			if (ccr0 == 0) {
				break;
			}
			if (tar == ccr0) {
				// set TAIFG
				m_mem->setBigEndian(m_tar, 0);
				if (taifg_interrupt_enabled) {
					m_mem->setBit(m_tactl, 1, true);
					m_intManager->queueInterrupt(m_variant->getTIMERA1_VECTOR());
				}
			}
			else {
				tar += 1;
				if (ccr0_interrupt_enabled && tar == ccr0) {
					// set TACCR0 CCIFG
					m_mem->setBit(m_ccr[0].tacctl, 1, true);
					m_intManager->queueInterrupt(m_variant->getTIMERA0_VECTOR());
				}
				m_mem->setBigEndian(m_tar, tar);
			}
			checkCCRInterrupts(tar);
			break;
		case TIMER_CONTINUOUS:
			m_mem->setBigEndian(m_tar, tar + 1);
			break;
		case TIMER_UPDOWN:
			if (m_up) {
				m_mem->setBigEndian(m_tar, tar + 1);
			}
			else {
				m_mem->setBigEndian(m_tar, tar - 1);
			}
			break;
		default:
			break;
	}
}

void Timer::tick() {
// 	static int i;
// 	i++;
// 	std::cout << i << "\n";
	uint8_t mode = (m_mem->getByte(m_tactl) >> 4) & 3;
	changeTAR(mode);
}

unsigned long Timer::getFrequency() {
	return m_source->getFrequency() / m_divider;
}

double Timer::getStep() {
	return m_source->getStep() * m_divider;
}

void Timer::reset() {
	m_source = m_aclk;
}


void Timer::handleMemoryChanged(Memory *memory, uint16_t address) {
	uint16_t val = memory->getBigEndian(address);

	if (address == m_tactl) {
		// TACLR
		if (val & 4) {
			memory->setBit(address, 4, false);
			memory->setBigEndian(m_tar, 0);
			m_up = true;
			m_divider = 1;
		}

		// Choose divider
		switch((val >> 6) & 3) {
			case 0: m_divider = 1; break;
			case 1: m_divider = 2; break;
			case 2: m_divider = 4; break;
			case 3: m_divider = 8; break;
			default: break;
		}

		// Choose source
		switch((val >> 8) & 3) {
			case 0: break;
			case 1: m_source = m_aclk; break;
			case 2: m_source = m_smclk; break;
			case 3: break;
			default: break;
		}
	}
}

void Timer::handleInterruptFinished(InterruptManager *intManager, int vector) {
	if (vector == m_variant->getTIMERA0_VECTOR()) {
		// CCR0 CCIFG is reset after interrupt routine
		m_mem->setBit(m_ccr[0].tacctl, 1, false);
	}
	else {
		// Check if we have more interrupts queued and if any, queue them
		for (int i = 1; i < m_ccr.size(); ++i) {
			bool interrupt_enabled = m_mem->isBitSet(m_ccr[i].tacctl, 16);
			if (m_mem->isBitSet(m_ccr[i].tacctl, 1)) {
				m_intManager->queueInterrupt(m_variant->getTIMERA1_VECTOR());
				return;
			}
		}

		bool taifg_interrupt_enabled = m_mem->isBitSet(m_tactl, 2);
		if (taifg_interrupt_enabled && m_mem->isBitSet(m_tactl, 1)) {
			m_intManager->queueInterrupt(m_variant->getTIMERA1_VECTOR());
		}
	}
}

void Timer::handleMemoryRead(Memory *memory, uint16_t address, uint16_t &value) {
	// Check what interrupts we have queued and set TAIV to the one with highest
	// priority.
	for (int i = 1; i < m_ccr.size(); ++i) {
		bool interrupt_enabled = m_mem->isBitSet(m_ccr[i].tacctl, 16);
		if (m_mem->isBitSet(m_ccr[i].tacctl, 1)) {
			value = 2 * i;
			m_mem->setBit(m_ccr[i].tacctl, 1, false);
			return;
		}
	}

	bool taifg_interrupt_enabled = m_mem->isBitSet(m_tactl, 2);
	if (taifg_interrupt_enabled && m_mem->isBitSet(m_tactl, 1)) {
		m_mem->setBit(m_tactl, 1, false);
		value = 10;
	}
}

}
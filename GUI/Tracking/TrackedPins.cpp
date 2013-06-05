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

#include "TrackedPins.h"

#include "ui/QSimKit.h"
#include "ui/Screen.h"
#include "ui/ScreenObject.h"
#include "Peripherals/SimulationObject.h"

#include <QWidget>
#include <QTime>
#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QIcon>
#include <QTreeWidgetItem>
#include <QDebug>

TrackedPins::TrackedPins(QSimKit *simkit, QWidget *parent) :
QDockWidget(parent), m_simkit(simkit) {
	setupUi(this);
	setMinimumHeight(100);

	connect(m_simkit, SIGNAL(onSimulationStep(double)), this, SLOT(handleSimulationStep(double)));
	connect(m_simkit, SIGNAL(onSimulationStarted(bool)), this, SLOT(handleSimulationStarted(bool)));
}

void TrackedPins::handleSimulationStarted(bool wasPaused) {
	std::map<ScreenObject *, SimulationObjectWrapper *> &wrappers = m_simkit->getScreen()->getWrappers();
	for (std::map<ScreenObject *, SimulationObjectWrapper *>::iterator it = wrappers.begin(); it != wrappers.end(); ++it) {
		QList<PinHistory *> &history = (*it).second->getPinHistory();
		for (int i = 0; i < history.size(); ++i) {
			if (history[i]) {
				plot->showPinHistory(history[i]);
			}
		}
	}
	plot->repaint();
}

void TrackedPins::handleSimulationStep(double t) {
	plot->setMaximumX(t);
}


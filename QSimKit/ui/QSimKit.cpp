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

#include "QSimKit.h"

#include "ProjectConfiguration.h"

#include "MCU/MCUManager.h"
#include "MCU/MCU.h"
#include "MCU/RegisterSet.h"
#include "MCU/Register.h"
#include "Peripherals/PeripheralManager.h"
#include "Peripherals/SimulationModel.h"

#include "DockWidgets/Disassembler/Disassembler.h"
#include "DockWidgets/Peripherals/Peripherals.h"
#include "Breakpoints/BreakpointManager.h"

#include "Tracking/TrackedPins.h"

#include <QWidget>
#include <QTime>
#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QIcon>
#include <QIODevice>
#include <QDebug>
#include <QDomDocument>
#include <QSettings>
#include <QMessageBox>

// #include "valgrind/callgrind.h"

QSimKit::QSimKit(QWidget *parent) : QMainWindow(parent),
m_dig(0), m_sim(0), m_logicalSteps(0), m_instPerCycle(2500), m_stopped(true) {
	setupUi(this);

	m_mcuManager = new MCUManager();
	m_mcuManager->loadMCUs();
	m_peripherals = new PeripheralManager();
	m_peripherals->loadPeripherals();

	screen->setPeripheralManager(m_peripherals);
	screen->setMCUManager(m_mcuManager);

	m_breakpointManager = new BreakpointManager();

	connect(actionNew_project, SIGNAL(triggered()), this, SLOT(newProject()) );
	connect(actionSave_project, SIGNAL(triggered()), this, SLOT(saveProject()) );
	connect(actionLoad_project, SIGNAL(triggered()), this, SLOT(loadProject()) );
	connect(actionProject_options, SIGNAL(triggered()), this, SLOT(projectOptions()) );
	connect(actionTracked_pins, SIGNAL(triggered(bool)), this, SLOT(showTrackedPins(bool)) );
	connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));

	populateToolBar();

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(simulationStep()));

	m_peripheralsWidget = new Peripherals(this);
	m_disassembler = new Disassembler(this);

	addDockWidget(m_disassembler, Qt::RightDockWidgetArea);	
	addDockWidget(m_peripheralsWidget, Qt::LeftDockWidgetArea);

	connect(screen, SIGNAL(onPeripheralAdded(QObject *)), m_peripheralsWidget, SLOT(addPeripheral(QObject *)));
	connect(screen, SIGNAL(onPeripheralRemoved(QObject *)), m_peripheralsWidget, SLOT(removePeripheral(QObject *)));

	m_trackedPins = new TrackedPins(this, this);
	QMainWindow::addDockWidget(Qt::BottomDockWidgetArea, m_trackedPins);

	setDockWidgetsEnabled(false);

	readSettings();
}

void QSimKit::setStepMode(StepMode mode) {
	int index = m_stepModeCombo->findData((int) mode);
	if (index == -1) {
		return;
	}

	m_stepModeCombo->setCurrentIndex(index);
}

StepMode QSimKit::getStepMode() {
	return (StepMode) m_stepModeCombo->itemData(m_stepModeCombo->currentIndex()).toInt();
}

void QSimKit::populateToolBar() {
	QAction *action = toolbar->addAction(QIcon(":/icons/22x22/actions/media-playback-start.png"), tr("Start &simulation"));
	connect(action, SIGNAL(triggered()), this, SLOT(startSimulation()));

	action = toolbar->addAction(QIcon(":/icons/22x22/actions/media-playback-pause.png"), tr("P&ause simulation"));
	action->setCheckable(true);
	action->setEnabled(false);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(pauseSimulation(bool)));
	m_pauseAction = action;

	action = toolbar->addAction(QIcon(":/icons/22x22/actions/media-playback-stop.png"), tr("Sto&p simulation"));
	connect(action, SIGNAL(triggered()), this, SLOT(stopSimulation()));

	action = toolbar->addAction(QIcon(":/icons/22x22/actions/media-skip-forward.png"), tr("Single step"));
	connect(action, SIGNAL(triggered()), this, SLOT(singleStep()));

	toolbar->addWidget(new QLabel("Single step mode:"));

	m_stepModeCombo = new QComboBox();
// 	m_stepModeCombo->setMaximumWidth(100);
	m_stepModeCombo->addItem("Simulation event", (int) SimulationStep);
	m_stepModeCombo->addItem("Assembler instr.", (int) AssemblerStep);
	m_stepModeCombo->addItem("C line", (int) CStep);
	toolbar->addWidget(m_stepModeCombo);

	setStepMode(AssemblerStep);

	toolbar->addSeparator();

	toolbar->addWidget(new QLabel("Run until:"));

	m_runUntil = new QLineEdit();
	m_runUntil->setMaximumWidth(100);
	m_runUntil->setValidator(new QDoubleValidator());
	m_runUntil->setText("1.0");
	connect(m_runUntil, SIGNAL(returnPressed()), this, SLOT(startSimulation()));
	toolbar->addWidget(m_runUntil);


}

void QSimKit::pointToInstruction(int pc) {
	m_disassembler->pointToInstruction(pc);
}

Screen *QSimKit::getScreen() {
	return screen;
}

void QSimKit::showTrackedPins(bool value) {
	m_trackedPins->setVisible(value);
}

void QSimKit::addDockWidget(DockWidget *widget, Qt::DockWidgetArea area) {
	QMainWindow::addDockWidget(area, widget);
	m_dockWidgets.append(widget);
}

void QSimKit::refreshDockWidgets() {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->refresh();
	}

	statusbar->showMessage(QString::number(m_sim->nextEventTime()));
}

void QSimKit::setDockWidgetsEnabled(bool enabled) {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->setEnabled(enabled);
	}

	toolbar->setEnabled(enabled);
}

void QSimKit::setDockWidgetsMCU(MCU *mcu) {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->setMCU(mcu);
	}

	setDockWidgetsEnabled(true);
}

void QSimKit::doSingleAssemblerStep() {
	uint16_t pc = screen->getMCU()->getRegisterSet()->get(0)->getBigEndian();
	double start_t = m_sim->nextEventTime();
	do {
		double t = m_sim->nextEventTime();

		do {
			m_sim->execNextEvent();
		}
		while (t == m_sim->nextEventTime());

		if (t - start_t > 0.01) {
			QMessageBox::warning(this, tr("Instruction takes too long"),
						tr("Single instruction takes more than 10ms. Possible loop detected."));
			break;
		}
	}
	while(pc == screen->getMCU()->getRegisterSet()->get(0)->getBigEndian());
}

void QSimKit::doSingleCStep() {
	uint16_t pc = screen->getMCU()->getRegisterSet()->get(0)->getBigEndian();
	double start_t = m_sim->nextEventTime();
	do {
		do {
			double t = m_sim->nextEventTime();

			do {
				m_sim->execNextEvent();
			}
			while (t == m_sim->nextEventTime());
			if (t - start_t > 0.01) {
				pc = 0; // to get from outer loop
				QMessageBox::warning(this, tr("C command takes too long"),
							tr("Single C command takes more than 10ms. Possible loop detected."));
				break;
			}
		}
		while(!m_disassembler->isDifferentCLine(screen->getMCU()->getRegisterSet()->get(0)->getBigEndian()));
	}
	while(pc == screen->getMCU()->getRegisterSet()->get(0)->getBigEndian());
}

void QSimKit::singleStep() {
	if (!m_dig || m_stopped) {
		resetSimulation();
		onSimulationStarted(false);
		m_stopped = false;
	}

	switch(getStepMode()) {
		case SimulationStep:
			m_sim->execNextEvent();
			break;
		case AssemblerStep:
			doSingleAssemblerStep();
			break;
		case CStep:
			doSingleCStep();
			break;
		default:
			break;
	}

	refreshDockWidgets();

	onSimulationStep(m_sim->nextEventTime());
}

void QSimKit::simulationStep() {
	QTime perf;
	perf.start();
	double until = m_runUntil->text().toDouble();
	for (int i = 0; i < m_instPerCycle; ++i) {
		m_sim->execNextEvent();
		if (m_breakpointManager->shouldBreak()) {
			m_instCounter += m_instPerCycle;
			onSimulationStep(m_sim->nextEventTime());
			m_pauseAction->setChecked(true);
			pauseSimulation(true);
			return;
		}

		if (m_sim->nextEventTime() >= until) {
			m_instCounter += m_instPerCycle;
			refreshDockWidgets();
			onSimulationStep(m_sim->nextEventTime());
			pauseSimulation(true);
			return;
		}
	}
	m_instCounter += m_instPerCycle;
	m_logicalSteps++;
	if (m_logicalSteps == 2) {
		m_logicalSteps = 0;
		statusbar->showMessage(QString("Simulation Time: ") + QString::number(m_sim->nextEventTime()) + ", " + QString::number(m_instPerCycle * 20) + " simulation events per second");
		onSimulationStep(m_sim->nextEventTime());
	}

	
	// Keep 60% CPU usage. This method is called every 50ms, so we should spend
	// 30ms here. If we are spending less or more, change the instPerCycle
	// value.
	if (perf.elapsed() < 28 || perf.elapsed() > 32) {
		m_instPerCycle = m_instPerCycle * (30.0 / perf.elapsed());
	}
}

void QSimKit::resetSimulation() {
	m_timer->stop();
	m_pauseAction->setChecked(false);

	delete m_dig;
	delete m_sim;

	m_dig = new SimulationModel();
	screen->prepareSimulation(m_dig);
	m_sim = new adevs::Simulator<SimulationEvent>(m_dig);
	screen->setSimulator(m_sim);
	
}

void QSimKit::startSimulation() {
	if (m_pauseAction->isChecked()) {
		m_pauseAction->setChecked(false);
		onSimulationStarted(true);
	}
	else {
		resetSimulation();
		onSimulationStarted(false);
	}
	m_stopped = false;
	m_instCounter = 0;
	m_pauseAction->setEnabled(true);
	m_timer->start(50);
	m_simStart.start();
	// 	CALLGRIND_ZERO_STATS;
}

void QSimKit::stopSimulation() {
	m_pauseAction->setChecked(false);
	m_timer->stop();
	m_pauseAction->setEnabled(false);
	onSimulationStopped();
	m_stopped = true;
}

void QSimKit::pauseSimulation(bool checked) {
	if (checked) {
		qDebug() << "Simulation paused. Simulation lasted" << m_simStart.elapsed() << "ms.";
		qDebug() << "Executed" << m_instCounter << "simulation events.";
		m_timer->stop();
		refreshDockWidgets();
		onSimulationPaused();
	}
	else {
		m_timer->start(50);
	}
}

void QSimKit::newProject() {
	ProjectConfiguration dialog(this, m_mcuManager);
	if (dialog.exec() == QDialog::Accepted) {
		m_filename = "";
		screen->clear();
		screen->setMCU(dialog.getMCU());
		setDockWidgetsMCU(screen->getMCU());

		m_breakpointManager->setMCU(screen->getMCU());
	}
}

void QSimKit::saveProject() {
	if (m_filename.isEmpty()) {
		m_filename = QFileDialog::getSaveFileName(this, tr("Save QSimKit project"),
												  QString(), tr("QSimKit project (*.qsp)"));
		if (m_filename.isEmpty()) {
			return;
		}
	}

	QFile file(m_filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
		return;

	QTextStream stream(&file);
	stream << "<qsimkit_project>\n";
	screen->save(stream);
	m_breakpointManager->save(stream);
	stream << "</qsimkit_project>\n";
}

bool QSimKit::loadProject(const QString &file) {
    int errorLine, errorColumn;
    QString errorMsg;

	QFile modelFile(file);
	QDomDocument document;
	if (!document.setContent(&modelFile, &errorMsg, &errorLine, &errorColumn))
	{
			QString error("Syntax error line %1, column %2:\n%3");
			error = error
					.arg(errorLine)
					.arg(errorColumn)
					.arg(errorMsg);
			qDebug() << error;
			return false;
	}

	if (!screen->load(document)) {
		screen->clear();
		return false;
	}

	m_filename = file;
	setDockWidgetsMCU(screen->getMCU());
	m_breakpointManager->setMCU(screen->getMCU());

	m_breakpointManager->load(document);

	return true;
}

void QSimKit::loadProject() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open QSimKit project"),
													QString(), tr("QSimKit project (*.qsp)"));
	if (filename.isEmpty()) {
		return;
	}

	loadProject(filename);
}

void QSimKit::projectOptions() {
	ProjectConfiguration dialog(this, m_mcuManager, screen->getMCU());
	if (dialog.exec() == QDialog::Accepted) {

	}
}

void QSimKit::readSettings() {
	QSettings settings("QSimKit", "QSimKit");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void QSimKit::closeEvent(QCloseEvent *event) {
	QSettings settings("QSimKit", "QSimKit");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	QMainWindow::closeEvent(event);
}


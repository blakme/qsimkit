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

#include "AddPeripheral.h"

#include "Peripherals/Peripheral.h"
#include "Peripherals/PeripheralManager.h"

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

AddPeripheral::AddPeripheral(PeripheralManager *p, QWidget *parent) :
QDialog(parent), m_peripherals(p) {
	setupUi(this);

	foreach (const PeripheralInfo &peripheral, m_peripherals->getPeripherals()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(peripherals);
		item->setText(0, peripheral.getName());
		item->setData(0, Qt::UserRole, peripheral.getLibrary());
	}

	connect(peripherals, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(handleItemActivated(QTreeWidgetItem *, int)));
}

QString AddPeripheral::getPeripheral() {
	QTreeWidgetItem *item = peripherals->currentItem();
	if (!item) {
		return "";
	}

	return item->data(0, Qt::UserRole).toString();
}

void AddPeripheral::handleItemActivated(QTreeWidgetItem *item, int colum) {
	Peripheral *p = m_peripherals->getPeripheral(item->data(0, Qt::UserRole).toString()).create();
	preview->setObject(p);
}


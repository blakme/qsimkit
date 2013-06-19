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

#include "CodeUtil.h"

#include <QFile>
#include <QProcess>

namespace CodeUtil {
	
static void parseCode(DisassembledFiles &df, QString &code) {
	QString file;
	DisassembledLine pendingSection;
	int num = 0;

	QStringList lines = code.split("\n", QString::SkipEmptyParts);
	for (int i = 0; i < lines.size(); ++i) {
		QString &line = lines[i];
		if (line[0] == ' ') {
			if (line.startsWith("   ") && line[8] == ':') {
				QString l = line.simplified();

				QString addr = l.left(l.indexOf(':'));
				int x;
				int c = 0;
				for (x = l.indexOf(':') + 1; x < l.size(); ++x) {
					if (!l[x].isNumber() && !l[x].isSpace() || (l[x].isSpace() && c > 2)) {
						if (l[x] >= 'a' && l[x] <= 'f') {
							c++;
							continue;
						}
						if (!l[x - 1].isSpace() && !l[x - 1].isNumber()) {
							x -= c;
						}
						break;
					}
					c = 0;
				}
				QString inst = l.mid(x);

				if (inst.isEmpty()) {
					continue;
				}

				df[file].append(DisassembledLine(addr.toInt(0, 16), num, DisassembledLine::Instruction, inst));
			}
			else {
				df[file].append(DisassembledLine(0, num, DisassembledLine::Code, line));
			}
		}
		else if (line[0] == '0') {
			QString addr = line.left(8).right(4);
			pendingSection = DisassembledLine(addr.toInt(0, 16), num, DisassembledLine::Section, line.mid(9));
		}
		else if (line.startsWith("+<")) {
			file = line.mid(6, line.indexOf(':') - 6);
			num = line.mid(line.indexOf(':') + 1).toInt();
			if (pendingSection.getAddr()) {
				df[file].append(pendingSection);
				pendingSection = DisassembledLine();
			}
		}
// 		else if (i > 2) {
// 			df[file].append(DisassembledLine(0, 0, DisassembledLine::Code, line));
// 		}
	}
}

DisassembledFiles disassemble(const QByteArray &elf, const QString &a43) {
	DisassembledFiles df;

	bool hasELF = true;
	QByteArray code = elf;
	if (code.isEmpty()) {
		hasELF = false;
		code = a43.toAscii();
	}

	QFile file("test.dump");
	if (!file.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
		return df;

	file.write(code);
	file.close();

	QProcess objdump;
	if (hasELF) {
		objdump.start("msp430-objdump", QStringList() << "-dSl" << "--prefix=+<FILE" << "test.dump");
	}
	else {
		objdump.start("msp430-objdump", QStringList() << "-D" << "-m" << "msp430:430" << "test.dump");
	}
	
	if (!objdump.waitForStarted()) {
		return df;
	}

	if (!objdump.waitForFinished())
		return df;

	QString result = QString(objdump.readAll());
	parseCode(df, result);

	return df;
}

QString ELFToA43(const QByteArray &elf) {
	QFile file("test.dump");
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return "";

	file.write(elf);
	file.close();

	QProcess objdump;
	objdump.start("msp430-objcopy", QStringList() << "-O" << "ihex" << "test.dump" << "test.a43");
	if (!objdump.waitForStarted()) {
		return "";
 	}
 
	if (!objdump.waitForFinished())
		return "";

	QFile file2("test.a43");
	if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
		return "";

	return file2.readAll();
}

}

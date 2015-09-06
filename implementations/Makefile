#!/bin/sh
#
# Makefile for building a suffix array test program.
#
# Copyright (C) 2015	Josh Marshall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

##PASS OFF BUILDS TO RESPECTIVE IMPLEMENTATIONS########################

ARCHON = archon3

BPR2 = bpr2_auto

CPM03_VAR = CPM03

DUMMY = dummyProgram

DIVUSORT = libdivsufsort2

LIBSUFFIXARRAY = libsuffixarray

QSUFSORT = qsufsort

SAIS = sais-lite-2.4.1

all : ARCHON BPR2 CPM03_VAR DUMMY DIVSUFSORT LIBSUFFIXARRAY QSUFSORT SAIS

ARCHON :
	cd $(ARCHON) \
	make \
	mv archon3 ..
	 cd ../
	

#Original build is broken
BPR2 :
	cd $(BPR2) \
	./bootstrap.sh \
	./configure \
	make \
	cd ../

#Original build is broken
CPM03_VAR :
	cd $(CPM03_VAR) ; make ; cd ../

DUMMY :
	cd $(DUMMY) ; make ; cd ../

#Original build is broken
DIVSUFSORT :
	cd $(DIVSUFSORT) ; cmake CMakeLists.txt ; make ; cd ../


LIBSUFFIXARRAY :
	cd $(LIBSUFFIXARRAY) ; make ; cd ../

QSUFSORT :
	cd $(QSUFSORT) ; make ; cd ../


SAIS :
	cd $(SAIS) \
	make \
	mv suftest .. \
	cd ../


clean :
	for i in $$(ls -d */ ) ; do cd $$i ; make clean ; cd .. ; done
	cd libdivsufsort ; make clean ; git checkout -- . ; git clean -f
	

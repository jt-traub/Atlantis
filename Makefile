# This is the makefile for Atlantis 5.0
#
# Copyright 1998 by Geoff Dunbar
# MODIFICATIONS
# Date        Person         Comments
# ----        ------         --------
# 2000/MAR/14 Davis Kulis    Added the template code.
# 2004/MAR/29 Jan Rietema    Added/modified the gamesets

GAME ?= standard

CPLUS = g++
CC = gcc
CFLAGS = -g -I. -I.. -Wall -std=c++20

RULESET_OBJECTS = extra.o map.o monsters.o rules.o world.o 

ENGINE_OBJECTS = alist.o aregion.o army.o astring.o battle.o economy.o \
  edit.o faction.o fileio.o game.o gamedata.o gamedefs.o gameio.o \
  genrules.o i_rand.o items.o main.o market.o modify.o monthorders.o \
  npc.o object.o orders.o parseorders.o production.o quests.o runorders.o \
  shields.o skills.o skillshows.o specials.o spells.o template.o unit.o \
  events.o events-battle.o events-assassination.o mapgen.o simplex.o namegen.o

OBJECTS = $(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)) \
  $(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)) 

TESTSUITE_OBJECTS = astring.o

TEST_OBJECTS = $(patsubst obj/main.o,,$(OBJECTS)) \
  $(patsubst %.o,unit/obj/%.o,$(TESTSUITE_OBJECTS))

$(GAME)-m: objdir $(OBJECTS)
	echo $(TEST_OBJECTS)
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME) $(OBJECTS)

test: objdir $(TEST_OBJECTS)
	echo "LINKING test"
	$(CPLUS) $(CFLAGS) -o $(GAME)/$(GAME)-test $(TEST_OBJECTS) -lgtest -lgtest_main
	$(GAME)/$(GAME)-test

all: basic standard fracas kingdoms havilah neworigins

arcadia: FORCE
	$(MAKE) GAME=arcadia

basic: FORCE
	$(MAKE) GAME=basic

standard: FORCE
	$(MAKE) GAME=standard
	
kingdoms: FORCE
	$(MAKE) GAME=kingdoms

fracas: FORCE
	$(MAKE) GAME=fracas

havilah: FORCE
	$(MAKE) GAME=havilah

neworigins: FORCE
	$(MAKE) GAME=neworigins

$(GAME)/$(GAME): FORCE
	$(MAKE) GAME=$(GAME)

standard-test: FORCE
	$(MAKE) GAME=standard test

all-clean: basic-clean standard-clean fracas-clean kingdoms-clean \
	havilah-clean neworigins-clean

arcadia-clean:
	$(MAKE) GAME=arcadia clean

basic-clean:
	$(MAKE) GAME=basic clean

standard-clean:
	$(MAKE) GAME=standard clean

fracas-clean:
	$(MAKE) GAME=fracas clean
	
kingdoms-clean:
	$(MAKE) GAME=kingdoms clean

havilah-clean:
	$(MAKE) GAME=havilah clean

neworigins-clean:
	$(MAKE) GAME=neworigins clean

clean:
	rm -f $(OBJECTS)
	rm -f $(TEST_OBJECTS)
	if [ -d obj ]; then rmdir obj; fi
	if [ -d unit/obj ]; then rmdir unit/obj; fi
	if [ -d $(GAME)/obj ]; then rmdir $(GAME)/obj; fi
	rm -f $(GAME)/html/$(GAME).html
	rm -f $(GAME)/$(GAME)
	rm -f $(GAME)/$(GAME)-test


all-rules: basic-rules standard-rules fracas-rules kingdoms-rules \
	havilah-rules neworigins-rules

arcadia-rules:
	$(MAKE) GAME=arcadia rules

basic-rules:
	$(MAKE) GAME=basic rules

standard-rules:
	$(MAKE) GAME=standard rules

fracas-rules:
	$(MAKE) GAME=fracas rules
	
kingdoms-rules:
	$(MAKE) GAME=kingdoms rules

havilah-rules:
	$(MAKE) GAME=havilah rules

neworigins-rules:
	$(MAKE) GAME=neworigins rules

rules: $(GAME)/$(GAME)
	(cd $(GAME); \
	 ./$(GAME) genrules $(GAME)_intro.html $(GAME).css html/$(GAME).html \
	)

FORCE:

objdir:
	if [ ! -d obj ]; then mkdir obj; fi
	if [ ! -d unit/obj ]; then mkdir unit/obj; fi
	if [ ! -d $(GAME)/obj ]; then mkdir $(GAME)/obj; fi


$(patsubst %.o,$(GAME)/obj/%.o,$(RULESET_OBJECTS)): $(GAME)/obj/%.o: $(GAME)/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,obj/%.o,$(ENGINE_OBJECTS)): obj/%.o: %.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

$(patsubst %.o,unit/obj/%.o,$(TESTSUITE_OBJECTS)): unit/obj/%.o: unit/%.cpp
	$(CPLUS) $(CFLAGS) -c -o $@ $<

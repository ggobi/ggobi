include ../local.config

SRC=$(wildcard *.java)
CLASSES=$(SRC:%.java=%.class)
classes: $(CLASSES)

JAVA_GNOME=${HOME}/java-gnome-0.7

CLASSPATH=$(JAVA_GNOME)/lib/gtk.jar:$(JAVA_GNOME)/lib/gnome.jar

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<

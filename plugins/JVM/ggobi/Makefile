include ../local.config

JAVA_SRC=$(wildcard *.java)

classes: $(JAVA_SRC:%.java=%.class)


%.class: %.java
	$(JAVAC) -classpath .. $<


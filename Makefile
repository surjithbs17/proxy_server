CXXFLAGS =	-O2 -g -Wall -lpthread -fmessage-length=0

OBJS =		webproxy.o

LIBS = -lpthread

TARGET =	webproxy

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

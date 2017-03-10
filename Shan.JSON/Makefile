TARGET = jt
OBJECTS = json_test.o main.o

CXX = g++
CXXFLAGS = -Wall -std=c++11
INC = -Ishan

$(TARGET) : $(OBJECTS)
        $(CXX) -o $(TARGET) $(OBJECTS)

%.o : Shan.JSON/%.cpp
        $(CXX) $(INC) $(CXXFLAGS) -c -o $@ $<

clean :
        $(RM) jt $(OBJECTS)

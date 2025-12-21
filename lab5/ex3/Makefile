CC= gcc

FLAGS= -Wall -g

C_SRC=string_manipulation.c
AS_SRC=ask3.s

OBJ_C=string_manipulation.o
OBJ_AS=ask3.o
OBJ=string_manipulation.o ask3.o

TARGET=string_manipulation.out 


all: $(TARGET)


$(OBJ_C): $(C_SRC)
        $(CC) $(FLAGS) -c $(C_SRC) -o $(OBJ_C)


$(OBJ_AS): $(AS_SRC)
        $(CC) $(FLAGS) -c $(AS_SRC) -o $(OBJ_AS)

$(TARGET): $(OBJ)
        $(CC) $(OBJ) -o $(TARGET)

clean:
        rm -f $(OBJ) $(TARGET)

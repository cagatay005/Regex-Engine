# Derleyici ve bayrak (flag) ayarları
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include

# Dosya yolları
SRC_DIR = src
TEST_DIR = tests
INCLUDE_DIR = include

# Kaynak dosyaları
SRCS = $(SRC_DIR)/lexer.c $(SRC_DIR)/nfa.c $(SRC_DIR)/matcher.c
MAIN_SRC = $(SRC_DIR)/main.c
TEST_SRC = $(TEST_DIR)/testRunner.c

# Çıktı dosyaları
TARGET = regex_engine
TEST_TARGET = test_runner

# Varsayılan hedef (İnteraktif uygulamayı derler)
all: $(TARGET)

# İnteraktif uygulamayı derleme kuralı
$(TARGET): $(SRCS) $(MAIN_SRC)
	$(CC) $(CFLAGS) $(SRCS) $(MAIN_SRC) -o $(TARGET)

# Projeyi test dosyasıyla derleyip testleri çalıştıran kural
test: $(SRCS) $(TEST_SRC)
	$(CC) $(CFLAGS) $(SRCS) $(TEST_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

# Derlenen dosyaları temizleyen kural
clean:
	rm -f $(TARGET) $(TEST_TARGET)
	rm -f *.exe
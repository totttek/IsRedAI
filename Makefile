CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g
LDLIBS  = -lm

GEN_DIR = generator_src
TRN_DIR = trainer_src
PRD_DIR = predictor_src
LIB_DIR = libmodel

GEN     = generator
TRN     = trainer
PRD     = predictor
LIB     = $(LIB_DIR)/libmodel.a

GEN_OBJ = $(GEN_DIR)/generator.o
TRN_OBJ = $(TRN_DIR)/trainer.o $(TRN_DIR)/dataset_utils.o
PRD_OBJ = $(PRD_DIR)/predictor.o

all: $(GEN) $(TRN) $(PRD)

$(GEN): $(GEN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(TRN): $(TRN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(PRD): $(PRD_OBJ) $(LIB)
	$(CC) $(CFLAGS) $(PRD_OBJ) -L$(LIB_DIR) -lmodel -o $@ $(LDLIBS)

# The library has its own Makefile; let it decide what needs rebuilding.
$(LIB): FORCE
	$(MAKE) -C $(LIB_DIR)

FORCE:

$(GEN_DIR)/%.o: $(GEN_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TRN_DIR)/%.o: $(TRN_DIR)/%.c $(TRN_DIR)/trainer.h
	$(CC) $(CFLAGS) -I$(TRN_DIR) -c $< -o $@

$(PRD_DIR)/%.o: $(PRD_DIR)/%.c $(LIB_DIR)/model.h
	$(CC) $(CFLAGS) -I$(LIB_DIR) -c $< -o $@

# Regenerates the datasets with fixed seeds (always identical output).
# Note: this overwrites test.csv too.
data: $(GEN)
	./$(GEN) 1000 2000 test.csv 0
	./$(GEN) 1 5000 train_uniform.csv 0
	./$(GEN) 2 5000 train_50.csv 0 50
	./$(GEN) 3 5000 train_uniform_boundry.csv 30

clean:
	$(MAKE) -C $(LIB_DIR) clean
	rm -rf $(GEN_OBJ) $(TRN_OBJ) $(PRD_OBJ)

fclean: clean
	$(MAKE) -C $(LIB_DIR) fclean
	rm -rf $(GEN) $(TRN) $(PRD)

dataclean:
	rm -rf *.csv

re: fclean all

.PHONY: all data clean fclean dataclean re FORCE

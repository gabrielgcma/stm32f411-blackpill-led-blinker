# Sistemas Embarcados I - 2023/3 - Universidade Federal de Uberlândia
# Aluno: Gabriel Carneiro Marques Amado
# Matrícula: 12111ECP002

PROGNAME := Blinky
LD = arm-none-eabi-gcc
LFLAGS = -nostdlib -T ./src/linker.ld

CC = arm-none-eabi-gcc
CFLAGS = -g -mcpu=cortex-m4 -mthumb -O0 -Wall 

CP = arm-none-eabi-objcopy

OBJDIR = build

DEPDIR = .deps
DEPFLAGS = -MMD -MP -MF $(DEPDIR)/$*.d

# Pega todos os .c do ./src/ 
SRCS = $(basename $(notdir $(wildcard ./src/*.c)))
# $(info $(SRCS))

OBJS = $(patsubst %, $(OBJDIR)/%.o, $(SRCS))
# $(info $(OBJS))
$(shell mkdir -p $(dir $(OBJS)) > /dev/null)

DEPS = $(patsubst %, $(DEPDIR)/%.d, $(SRCS))
# $(info $(DEPS))
$(shell mkdir -p $(dir $(DEPS)) > /dev/null)

all: $(OBJDIR)/$(PROGNAME).elf $(OBJDIR)/$(PROGNAME).bin

# $< = primeiro elemento da lista de pré requisitos
# $@ = alvo da regra

$(OBJDIR)/%.o: ./src/%.c $(DEPDIR)/%.d
	$(CC) -c $(CFLAGS) $(DEPFLAGS) $< -o $@

$(OBJDIR)/$(PROGNAME).elf: $(OBJS)
	$(LD) $(LFLAGS) $^ -o $@ 

$(OBJDIR)/$(PROGNAME).bin: $(OBJDIR)/$(PROGNAME).elf
	$(CP) -O binary $< $@

$(DEPS):
-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(DEPDIR) $(OBJDIR)/$(PROGNAME).elf $(OBJDIR)/$(PROGNAME).bin 
flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c init -c "reset halt" -c "flash write_image erase ./build/Blinky.bin 0x08000000" -c 'reset init' -c 'resume' -c exit

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Dump Format:
 *
 *    ADDRESS  | 16 hex values  | 16 ascii/'.' values
 *
 *    Example:
 *      Address                hex values                         ascii/'.' values
 *      000000 | A0 B0 C0 D0 E0 F0 A1 B1 C1 D1 E1 F1 A2 B2 C2 D2 | ................
 *
 * Extra Information:
 *    If the dump exceeds a specific amount of memory, the address values will change colors:
 *      ↳ Yellow: Yellow means the address is getting big.
 *      ↳ Red: Red means the address is huge.
 *      ↳ Red Underlined: Address has exceeded 1MB/has reached 1MB.
 *    Other general colors:
 *      ↳ Green: When 2, or more, colors are the same simultaneously
 *      ↳ Blue: When a hex value represents a ascii value(ascii values are white, non-ascii values are red).
 *      ↳ Yellow
 */

#define TRUE    1
#define FALSE   0
typedef char    bool;

struct Information {
  /*
   * special_char:
   *    Will be the placeholder for non-acsii values.
   */
  char special_char;
  /*
   * COMMAND:
   *    --HW
   * USAGE:
   *    Use the --HW argument if you want all ascii values depicted from the hex values to be highlighted.
   */
  bool highlight_words;
  int chunk_size;
  bool sufficient_print;
  /*
   * COMMANDS:
   *    --stopAt [value_to_stop_at: char/hex/dec]
   *      ↳ Where [value_to_stop_at: char/hex/dec] can be a character(eg. 'a'), a hex value(eg. 0xFF) or a decimal value(eg. 15).
   *      ↳ [value_to_stop_at: char/hex/dec] is required for the argument --stopAt.
   * USAGE:
   *    stop_at_value_I
   *      ↳ Stores the decimal/hex value.
   *    stop_at_value_C
   *      ↳ Stores the character value(a-z, A-Z).
   *    
   *    --stopAt [value_to_stop_at: char/hex/dec] will stop the program after it reaches a specific value.
   *      ↳ 1. If the value is not found, the program will not stop.
   *      ↳ 2. If the value is found 𝘣𝘦𝘧𝘰𝘳𝘦 the end of the current line(see layout information above), the program will stop after the line is complete.
   *        ↳ Example:
   *        ↳   Lets say we're looking for value 't' in a dump, and the dump looks like this:
   *            000000 | FA BD 00 7C B8 C0 07 8E D8 8E C0 31 C0 8E D0 89 | ................
                000010 | EC FB E8 00 00 B8 DE 07 8E C0 31 DB B4 02 B0 03 | ................
                000020 | B5 00 B1 01 B6 00 B2 80 CD 13 72 0B 80 FC 00 75 | ..........r....u
                000030 | 06 3C 00 74 02 EB 08 B4 0E B0 45 CD 10 EB FE B0 | ...t

                The value 't' was found, but the program won't stop there. Instead, it will finish that line then stop:
                000000 | FA BD 00 7C B8 C0 07 8E D8 8E C0 31 C0 8E D0 89 | ................
                000010 | EC FB E8 00 00 B8 DE 07 8E C0 31 DB B4 02 B0 03 | ................
                000020 | B5 00 B1 01 B6 00 B2 80 CD 13 72 0B 80 FC 00 75 | ..........r....u
                000030 | 06 3C 00 74 02 EB 08 B4 0E B0 45 CD 10 EB FE B0 | ...t......E.....
                Stopped at value t
   */
  int stop_at_value_I;
  char stop_at_value_C;
  /*
   * USAGE:
   *    *BUFFER stores all values(ascii or not) from the file being dumped.
   */
  char *BUFFER;
  /*
   * COMMAND: 
   *  --stopAtEqual [optional_command: amount]
   *    ↳ where [optional_command: amount] is the amount of hex values you want to go through(that are the same/have the same letters in them)
   *    before exiting.
   * USAGE:
   *  Use the --stopAtEqual [optional_command: amount] argument if you want to stop at a hex value that has two have the same letters.
   *    ↳ If you apply the [optional_command: amount], the program will proceed to run until it finds X amount of hex values with the same letter.
   */
  bool stopAtEqual;
  /*
   * USAGE:
   *    If the user applies the [optional_command: amount] to the --stopAtEqual argument, the value will be stored here.
   */
  int times;
  int cur;
  int ind;
  char *INFO;
  char last_val;
  int total_found;
  char ascii_val[16];
};

static struct Information info = (struct Information) {
    .special_char = '.',
    .highlight_words = FALSE,
    .chunk_size = 0,
    .sufficient_print = FALSE,
    .stop_at_value_I = -1,
    .stop_at_value_C = -1,
    .stopAtEqual = FALSE,
    .times = -1,
    .cur = 0,
    .ind = 0,
    .last_val = '\0',
    .total_found = 0
  };

char get_next_current(int index)
{
  char current = info.INFO[index];
  if(!(current == info.last_val))
  {
    info.last_val = current;
    info.total_found = 1;
  }
  else info.total_found++;

  if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) {
    info.ascii_val[info.ind] = (char)current;
    info.ind++;
  } else {
    info.ascii_val[info.ind] = info.special_char;
    info.ind++;
  }

  return current;
}

void replace(const char *filename, int start, int end, const char *output_file)
{
  FILE* file = fopen(filename, "rb");
  FILE* rep = fopen(output_file, "w");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  size_t size;
  if(end == 0) end = file_size;
  if((start + end) > file_size && !(file_size == 0))
  {
    while(end >= file_size)
      end--;

    if((start + end) >= file_size)
    {
      if(start > 0)
        while(start >= file_size + 10)
          start--;
    }
    
    size = start + end;
  } else size = start + end;

  int count = 0;
  int index = 0;
  info.BUFFER = calloc(size, sizeof(*info.BUFFER));

  info.INFO = calloc(file_size, sizeof(*info.INFO));
  fread(info.INFO, file_size, 1, file);

  while(count < file_size)
  {
    if(count >= start && count <= end)
    {
      info.BUFFER[index] = info.INFO[count];
      index++;
    } else {
      fputc((int)info.INFO[count], rep);
    }
    count++;
  }

  unsigned char current;
  int mem_addr = 0;
  int total_found = 0;
  char mem_addr_padding[32];
  char *pad_val = "%06X";
  char stop = FALSE;
  char hex_info[3];
  int last_value = 0;
  int length = 0;

  if(file_size >= 0x100000)
  {
    pad_val = "%010X";
  }
  if(info.chunk_size == 0) info.chunk_size = file_size;

  int last_addr = 0;
  for(int i = 0; i < file_size; i++)
  {
    if(i == file_size - 1)
    {
      int leftover = (mem_addr % 16) / 4 - 1;
      for(int x = 0; x < leftover; x++)
      {
        if(x == leftover - 1) printf(" | ");
        else printf("xx");
      }

      for(int x = 0; x < 16; x++)
      {
        if(info.ascii_val[x] == '\0') printf("\033[1;91m.\033[0;37m");
        else {
          if(info.ascii_val[x] == info.special_char) printf("\033[1;91m%c\033[0;37m", info.ascii_val[x]);
          else printf("\033[1;97m%c", info.ascii_val[x]); 
        }
      }
      if(mem_addr / 1024 / 1024 >= 1) printf(" | [%fMB]\n", (float)mem_addr / 1024 / 1024);
      else printf("\n");
      break;
    }
    // if((int)info.INFO[i] < 0) info.INFO[i] *= -1;
    current = get_next_current(i);

    if(info.sufficient_print == TRUE)
    {
      redo:
      if(!(info.cur % 32 == 1)) goto again;
      else {
          info.cur = 0;
          last_addr += mem_addr;
          mem_addr = 0;
      }
      i++;
      if(i == file_size - 1) goto end;
      //if(mem_addr % 16 == 0)
      //{
        if(info.cur == 0)
        {
          // putchar('\n');
          // for(int d = 0; d < 80; d++) putchar('-');
          // putchar('\n');
          sprintf(mem_addr_padding, pad_val, mem_addr);
          length = strlen(mem_addr_padding)+2;
          printf("\n\033[1;33m%s\033[0;37m | ", mem_addr_padding);
          memset(mem_addr_padding, 0, strlen(mem_addr_padding));
        }
      //}

      again:
      current = get_next_current(i);

      if(info.ind >= 16)
      {
        memset(info.ascii_val, 0, info.ind);
        info.ind = 0;
      }

      if(info.cur / 32 == 1) {
        putchar('\n');
        for(int i = length; i > 0; i--)
          putchar(' ');

        info.cur = 0;
        goto redo;
      } else {
        printf("%02X ", current);
        info.cur++;
        mem_addr++;
        goto redo;
      }
    } else {
      if(i == size) printf("\n\n\t\t----CUT----\n\n");
      if(mem_addr % info.chunk_size == 0)printf("\n\t\t\t\t%d CHUNK\n\n", info.chunk_size);
      if(mem_addr % 16 == 0)
      {
        if(mem_addr >= 0x20000 && mem_addr < 0x30000)
        {
          sprintf(mem_addr_padding, pad_val, mem_addr);
          printf("\033[4;33m%s\033[0;37m | ", mem_addr_padding);
          memset(mem_addr_padding, 0, strlen(mem_addr_padding));
        }
        else if(mem_addr >= 0x30000)
        {
          sprintf(mem_addr_padding, pad_val, mem_addr);
          printf("\033[4;91m%s\033[0;37m | ", mem_addr_padding);
          memset(mem_addr_padding, 0, strlen(mem_addr_padding));
        }
        else if(mem_addr >= 0xF4240)
        {
          sprintf(mem_addr_padding, pad_val, mem_addr);
          printf("\033[0;101m%s\033[0;37m | ", mem_addr_padding);
          memset(mem_addr_padding, 0, strlen(mem_addr_padding));
        }
        else {
          sprintf(mem_addr_padding, pad_val, mem_addr);
          printf("%s | ", mem_addr_padding);
          memset(mem_addr_padding, 0, strlen(mem_addr_padding));
        }
        if(total_found > 0 && (info.INFO[i + 1] == info.last_val || info.INFO[i - 1] == info.last_val))
          printf("\033[1;32m%02X\033[0;37m ", current);
        else
        {
          if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m ", current);
          else printf("\033[1;93m%02X\033[0;37m ", current);
        }
      } else if(mem_addr % 16 == 15)
      {
        if(total_found > 0 && (info.INFO[i + 1] == info.last_val || info.INFO[i - 1] == info.last_val))
          printf("\033[1;32m%02X\033[0;37m | ", current);
        else
        {
          if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m | ", current);
          else printf("\033[1;93m%02X\033[0;37m | ", current);
        }
        for(int x = 0; x < 16; x++)
        {
          if(info.ascii_val[x] == info.special_char) printf("\033[1;91m%c\033[0;37m", info.ascii_val[x]);
          else {
            if(info.highlight_words == 1) printf("\033[46m\033[1;97m%c\033[0m", info.ascii_val[x]);
            else printf("\033[1;97m%c\033[0;37m", info.ascii_val[x]); 
          }
        }
        memset(info.ascii_val, 0, strlen(info.ascii_val));
        if(mem_addr / 1024 / 1024 >= 1) printf("\033[0;37m | [%.3fMB]\n", (float)mem_addr / 1024 / 1024);
        else printf("\n");
        info.ind = 0;
        if(stop == TRUE) {
          if(info.stop_at_value_C == -1 && !(info.stop_at_value_I == -1))
            printf("\n\nStopped at value %X\n", info.stop_at_value_I);
          else if(info.stop_at_value_I == -1 && !(info.stop_at_value_C == -1)) printf("\n\nStopped at value %c\n", info.stop_at_value_C);
          else printf("\n\nStopped at matching number: %d\n", last_value);
          goto end;
        }
      } else
      {
        if(info.stopAtEqual == TRUE)
        {  
          sprintf(hex_info, "%02X", current);
          if(hex_info[0] == hex_info[1] && atoi(hex_info) > 0)
          {
            last_value = atoi(hex_info);
            stop = TRUE;
          }
        }
        if(current == info.stop_at_value_I || current == info.stop_at_value_C) {
          stop = TRUE;
          //continue;
        }
        if(total_found > 0 && (info.INFO[i + 1] == info.last_val || info.INFO[i - 1] == info.last_val))
          printf("\033[1;32m%02X\033[0;37m ", current);
        else
        {
          if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m ", current);
          else printf("\033[1;93m%02X\033[0;37m ", current);
        }
      }
    }

    mem_addr++;
  }

  end:
  fclose(file);
  fclose(rep);
}

int see_next(int val, int total)
{
  return (total - val) > 0;
}

int main(int arg_c, char **argv)
{

  if(arg_c < 2)
  {
    fprintf(stderr, "Expected: file, start, end.");
    exit(EXIT_FAILURE);
  }

  int start = 0;
  int end = 0;
  char* of = "out.txt";

  for(int i = 0; i < arg_c; i++)
  {
    if(strcmp(argv[i], "--special") == 0)
    {
      if(see_next(i, arg_c) == 1) info.special_char = (char)argv[i + 1][0];
    }
    if(strcmp(argv[i], "--stopAt") == 0)
    {
      if(see_next(i, arg_c) == 1) {
        if(argv[i+1][1] == 'x')
          info.stop_at_value_I = strtol(argv[i+1], NULL, 16);
        else if((strlen(argv[i+1]) == 1 && (argv[i+1][0] >= 'a' && argv[i+1][0] <= 'z')) || (argv[i+1][0] >= 'A' && argv[i+1][0] <= 'Z'))
          info.stop_at_value_C = argv[i+1][0];
        else info.stop_at_value_I = atoi(argv[i+1]);
      }
    }
    if(strcmp(argv[i], "--stopAtEqual")==0) info.stopAtEqual = TRUE;
    if(strcmp(argv[i], "--HW") == 0) info.highlight_words = TRUE;
    if(strcmp(argv[i], "--S") == 0)
    {
	    if(see_next(i, arg_c) == 1) start = atoi(argv[i + 1]);
    }
    if(strcmp(argv[i], "--E") == 0)
    {
	    if(see_next(i, arg_c) == 1) end = atoi(argv[i + 1]);
    }
    if(strcmp(argv[i], "--OF") == 0)
    {
	    if(see_next(i, arg_c) == 1) of = argv[i + 1];
    }
    if(strcmp(argv[i], "--CS") == 0)
    {
      if(see_next(i, arg_c) == 1) info.chunk_size = atoi(argv[i + 1]);
    }
    if(strcmp(argv[i], "--SP") == 0)
    {
      info.sufficient_print = TRUE;
    }
  }

  replace(argv[1], start, end, of);
}
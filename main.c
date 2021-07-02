#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char special_char = '_';
static int highlight_words = 0;

void replace(const char *filename, int start, int end, const char *output_file)
{
  FILE* file = fopen(filename, "rb");
  FILE* rep = fopen(output_file, "w");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  size_t size;
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
  char c;
  char BUFFER[size];

  unsigned char INFO[file_size];
  fread(INFO, file_size, 1, file);

  while(count < file_size)
  {
    if(count >= start && count <= end)
    {
      BUFFER[index] = INFO[count];
      index++;
    } else {
      fputc((int)INFO[count], rep);
    }
    count++;
  }

  unsigned char current;
  int temp = 0;
  char HEX_VAL[size];
  int mem_addr = 0;
  int spacing = 1;
  char ascii_val[16];
  int ind = 0;
  unsigned char last_val = 0;
  int total_found = 0;
  char mem_addr_padding[32];
  char *pad_val = "%06X";

  if(file_size >= 0x100000)
  {
    pad_val = "%010X";
  }
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
        if(ascii_val[x] == special_char) printf("\033[1;91m%c\033[0;37m", ascii_val[x]);
        else printf("\033[1;97m%c", ascii_val[x]); 
      }
      if(mem_addr / 1024 / 1024 > 1) printf(" | [%f]", (float)mem_addr / 1024 / 1024);
      else printf("\n");
      break;
    }
    // if((int)INFO[i] < 0) INFO[i] *= -1;
    current = INFO[i];
    if(!(current == last_val))
    {
      last_val = current;
      total_found = 1;
    }
    else total_found++;

    if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) {
      ascii_val[ind] = (char)current;
      ind++;
    } else {
      ascii_val[ind] = special_char;
      ind++;
    }

    if(i == size)
    {
      printf("\n\n\t\t----CUT----\n\n");
    }
    if(mem_addr % 16 == 0)
    {
      if(mem_addr >= 0x1000 && mem_addr < 0x8000)
      {
        sprintf(mem_addr_padding, pad_val, mem_addr);
        printf("\033[1;33m%s\033[0;37m | ", mem_addr_padding);
        memset(mem_addr_padding, 0, strlen(mem_addr_padding));
      }
      else if(mem_addr >= 0x8000 && mem_addr < 0x30000)
      {
        sprintf(mem_addr_padding, pad_val, mem_addr);
        printf("\033[1;31m%s\033[0;37m | ", mem_addr_padding);
        memset(mem_addr_padding, 0, strlen(mem_addr_padding));
      }
      else if(mem_addr >= 0x30000)
      {
        sprintf(mem_addr_padding, pad_val, mem_addr);
        printf("\033[0;91m%s\033[0;37m | ", mem_addr_padding);
        memset(mem_addr_padding, 0, strlen(mem_addr_padding));
      }
      else {
        sprintf(mem_addr_padding, pad_val, mem_addr);
        printf("%s | ", mem_addr_padding);
        memset(mem_addr_padding, 0, strlen(mem_addr_padding));
      }
      if(total_found > 0 && (INFO[i + 1] == last_val || INFO[i - 1] == last_val))
        printf("\033[1;32m%02X\033[0;37m ", current);
      else
      {
        if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m ", current);
        else printf("\033[1;93m%02X\033[0;37m ", current);
      }
    } else if(mem_addr % 16 == 15)
    {
      if(total_found > 0 && (INFO[i + 1] == last_val || INFO[i - 1] == last_val))
        printf("\033[1;32m%02X\033[0;37m | ", current);
      else
      {
        if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m | ", current);
        else printf("\033[1;93m%02X\033[0;37m | ", current);
      }
      for(int x = 0; x < 16; x++)
      {
        if(ascii_val[x] == special_char) printf("\033[1;91m%c\033[0;37m", ascii_val[x]);
        else {
          if(highlight_words == 1) printf("\033[46m\033[1;97m%c\033[0m", ascii_val[x]);
          else printf("\033[1;97m%c\033[0;37m", ascii_val[x]); 
        }
      }
      if(mem_addr / 1024 / 1024 >= 1) printf("\033[0;37m | [%2fMB]\n", (float)mem_addr / 1024 / 1024);
      else printf("\n");
      ind = 0;
    } else
    {
      if(total_found > 0 && (INFO[i + 1] == last_val || INFO[i - 1] == last_val))
        printf("\033[1;32m%02X\033[0;37m ", current);
      else
      {
        if((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) printf("\033[1;94m%02X\033[0;37m ", current);
        else printf("\033[1;93m%02X\033[0;37m ", current);
      }
    }

    mem_addr++;
  }

  // BUFFER[index - 1] = '\0';
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

  for(int i = 0; i < arg_c; i++)
  {
    if(strcmp(argv[i], "--special") == 0)
    {
      if(see_next(i, arg_c) == 1) special_char = (char)argv[i + 1][0];
    }
    if(strcmp(argv[i], "--HW") == 0) highlight_words = 1;
  }

  replace(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4]);

  
}
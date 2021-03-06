#include <stdio.h>
// clang-format puts this above stdio.h
// this is an error. stdio.h should be included before readline
#ifndef nongnu
#include <readline/history.h>
#include <readline/readline.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifndef nongnu
#include <uuid/uuid.h>
#endif

#ifdef nongnu
static char *readline(char *s) {
  printf("\nNote: running without readline. The limit is 499 chars\n%s", s);
  char *a = malloc(500);
  (void)fgets(a, 500, stdin);
  return a;
}

static void swap(char *a, char *b);
static int random(unsigned long length);

// do nothing
static inline void rl_clear_history() {}
static inline void clear_history() {}
#endif

static char *getuuid(char *s);
int main(int argc, char **argv) {
  // just ignore the command line arguments
  (void)argc;
  (void)argv;
  // this variable holds our timestamp
  unsigned long a = (unsigned long)time(NULL);
  // name stores the name of the full zip. name1 stores the name of the
  // incremental zip. Link stores a base URI from where both files can be
  // acccessed. The code will always assume both files are in the same directory
  static char *name = NULL, *name1 = NULL, *link = NULL, *temp = NULL,
              *temp1 = NULL;
  // base download link where both full and incremental zip are stored
  // size is the size of the full OTA. size1 is the size of the incremental OTA
  static unsigned long long size, size1;
  // this is used to safely accept user input for the size variables
  // spaces are needed for fornatting and since this is needed often keep a
  // const variable for it
  static const char *__restrict five_spaces = "     ";

  char *update_id = malloc(37);

  // open the updates file for reading
  FILE *updates = fopen("updates.json", "a");

  // get information about full OTA
  name = readline("Enter file name for full OTA. must have no spaces\n");
  // strip trailing newline
  name[strcspn(name, "\n")] = 0;

  temp = readline(
      "Enter file size in bytes. use wc -c <update.zip to find this. values "
      "above 100 GB are not allowed\n");
  size = (unsigned long long)strtoll(temp, NULL, 10);

  name1 =
      readline("Enter file name for incremental OTA. must have no spaces\n");
  name1[strcspn(name1, "\n")] = 0;

  // get incremental OTA details
  temp1 = readline(
      "Enter file size in bytes. use wc -c <update.zip to find this. values "
      "above 100 GB are not allowed\n");
  size1 = (unsigned long long)strtoll(temp1, NULL, 10);

  link = readline("Enter base download link. The filename will be appended to "
                  "this. The trailing slash does not matter\n");
  link[strcspn(link, "\n")] = 0;

  // if size is 0 then user did not enter a valid number and if size1
  // (incremental size) > size (full size) something is wrong
  if (size == 0 || size1 == 0 || size1 > size) {
    fclose(updates);
    if (temp)
      printf("invalid size. exiting\n");
    else
      printf("Out of memory error\n");
    goto free;
  }

  if (!(name && name1 && link && temp && temp1 && update_id)) {
    printf("FAILED: out of memory?\n");
    goto free;
  }

  // handle trailing slash in the download link
  if (link[(strlen(link)) - 1] == '/')
    link[(strlen(link)) - 1] = 0;

  // write full OTA information to file
  fprintf(
      updates,
      "{\n  \"response\": [\n    {\n%s\"datetime\": %lu,\n%s\"filename\": "
      "\"%s\",\n%s\"id\": \"%s\",\n%s\"romtype\": \"unofficial\",\n%s\"size\": "
      "%lld,\n%s\"url\": \"%s/%s\",\n%s\"version\": \"17.1\"\n    },\n",
      five_spaces, a, five_spaces, name, five_spaces, getuuid(update_id),
      five_spaces, five_spaces, size, five_spaces, link, name, five_spaces);

  // write incremental OTA details
  fprintf(
      updates,
      "    {\n%s\"datetime\": %lu,\n%s\"filename\": "
      "\"%s\",\n%s\"id\": \"%s\",\n%s\"romtype\": \"unofficial\",\n%s\"size\": "
      "%lld,\n%s\"url\": \"%s/%s\",\n%s\"version\": \"17.1\"\n    }\n  ]\n}\n",
      five_spaces, a, five_spaces, name1, five_spaces, getuuid(update_id),
      five_spaces, five_spaces, size1, five_spaces, link, name1, five_spaces);
  // close the file
  fclose(updates);
  printf("check updates.json\n");
free:
  updates = NULL;
  if (name)
    free(name);
  if (name1)
    free(name1);
  if (link)
    free(link);
  if (temp)
    free(temp);
  if (temp1)
    free(temp1);
  if (update_id)
    free(update_id);
  name = name1 = link = temp = temp1 = update_id = NULL;
  rl_clear_history();
  clear_history();
}

#ifndef nongnu
static char *getuuid(char *update_id) {
  // the variable we use to store the uuid in the binary representation
  static uuid_t temp_update_id;
  // generate uuid
  uuid_generate_random(temp_update_id);
  // convert the generated uuid from binary to text (lowercase)
  uuid_unparse_lower(temp_update_id, update_id);
  char *a;
  // we need this to remove dashes from the uuid
  while (strstr(update_id, "-")) {
    a = strstr(update_id, "-");
    // randomly return a number from 0-8. why not 9? well i was bored, that's
    // why
    *a = 48 + (rand() % 9);
  }
  return update_id;
}
#else
// a simple trick to get a random ID. Walk through the entire list and randomly
// replace any element with any other element. Then get the first 36 bytes and
// copy that into update id. Note that numbers are given higher priority so they
// appear several times in the list
static char *getuuid(char *update_id) {
  char a[] =
      "012345678900987654321qwertyuiopasdfghjklzxcvbnm123456789009876543210";
  for (unsigned long i = 0; i < strlen(a); ++i)
    swap(&a[i], &a[random(strlen(a))]);
  a[36] = 0;
  strcpy(update_id, a);
  return update_id;
}

static void swap(char *a, char *b) {
  static char temp;
  temp = *a;
  *a = *b;
  *b = temp;
}

// This function only exists to silence a compiler warning
static int random(unsigned long length) {
  int a = rand();
  a %= length;
  return a;
}
#endif

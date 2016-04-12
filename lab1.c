#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

// prototype values until I allocate at a level 999 Monk status.
#define MAXLEN 100
#define MAXBODYLEN 1000

// callback used for slurp
typedef void (*callback)(FILE * f, char * n);

char ** alloc2D(int, int);
char *** alloc3D(int, int, int);
void dealloc2D(char **, int);
void dealloc3D(char ***, int, int);
int parseCommand(char **, char *);
FILE * openFileSafely(char *, char *);
void closeFileSafely(FILE *);
int slurp(char *, callback);
void spurt(char *, char[MAXBODYLEN]);
int parse(char *, char ***, int);

struct LinkNode {
  int index;
  char * name;
  char * address;
  char * phone;
  struct LinkNode * next;
  struct LinkNode * prev;
} * head;

struct File {
  char name[MAXLEN];
  char lines[MAXBODYLEN];
  int isLoaded;
} loadedFile = {.isLoaded = 0};


typedef struct LinkNode Record;

void restoreIndices() {
  if (head == NULL) return;
  Record * node;
  for (node = head->next; node != NULL; node = node->next)
    node->index = node->prev->index + 1;
}

void compose(char lines[MAXLEN]) {
  if (head != NULL) {
    snprintf(lines, 1000, "%s\t%s\t%s\n", head->name, head->address, head->phone);
    Record * node;
    for (node = head->next; node != NULL; node = node->next) {
      char to_append[MAXLEN];
      sprintf(to_append, "%s\t%s\t%s\n", node->name, node->address, node->phone);
      strcat(lines, to_append);
    }
    return;
  }
  snprintf(lines, 1000, "");
}

Record * createRecord(char * name, char * address, char * phone, Record * prev) {
  Record * node = (Record *)malloc(sizeof(Record));
  node->name = name;
  node->address = address;
  node->phone = phone;
  node->next = NULL;
  node->prev = prev;
  if (prev != NULL) {
    node->prev->next = node;
    node->index = node->prev->index + 1;
  } else
    node->index = 1;
  return node;
}

void printRecords() {
  Record * node;
  for (node = head; node != NULL; node = node->next)
    printf("%d\t%s\t%s\t%s\n", node->index, node->name, node->address, node->phone);
}

void deleteRecord(int index) {
  if (head == NULL) {
    //printf("There aren't any records to delete!\n");
    return;
  }
  //printf("deleting record at index %d...\n", index);
  if (index == 1) {
    if (head->next == NULL) {
      free(head);
      head = NULL;
    } else {
      Record ** temp = &((head)->next);
      free(head);
      head = NULL;
      head = *temp;
      head->index = 1;
    }
  } else {
    Record * node;
    for (node = head; node != NULL; node = node->next)
      if (node->index == index) {
        node->prev->next = node->next;
        if (node->next != NULL)
          node->next->prev = node->prev;
        Record * tempNode = node->prev;
        free(node);
        node = NULL;
        break;
      }
  }
  restoreIndices();
  compose(loadedFile.lines);
}


void clearRecords(Record ** rec) {
  if (*rec == NULL) return;
  if ((*rec)->next != NULL) {
    clearRecords(&((*rec)->next));
    (*rec)->next = NULL;
    if ((*rec)->prev != NULL) return;
  }
  free(*rec);
  *rec = NULL;
}

void slurpCB(FILE * file, char * name) {
  loadedFile.isLoaded = 1;
  char * c = malloc(MAXLEN*sizeof(char));
  while ((fgets(c, MAXLEN, file)))
	  strcat(loadedFile.lines, c);
  free(c);
  c = NULL;
  strcpy(loadedFile.name, name);
}

void loadRecords(char * filename) {
  if (slurp(filename, slurpCB) == 0) return;
  char *** parsedLines = alloc3D(MAXLEN, 3, MAXLEN);
  int lineCount = parse(loadedFile.lines, parsedLines, 4);
  Record * records[lineCount];
  int i;
  for (i = 0; i < lineCount; ++i) {
    char * name = parsedLines[i][0],
         * addr = parsedLines[i][1],
         * phne = parsedLines[i][2];
    Record * prev = NULL;
    if (i != 0) prev = records[i-1];
    records[i] = createRecord(name, addr, phne, prev);
  }
  //dealloc3D(parsedLines, MAXLEN, 3);
  if (lineCount > 0) head = records[0];
  //printf("%s is loaded\n", filename);
}

void writeRecords(char filename[100]) {
  if (strcmp(filename, "") == 0)
    strcpy(filename, loadedFile.name);
  spurt(filename, loadedFile.lines);
}

void handleCommand(char ** command) {
  /* file-independent commands */
  if (strcmp(command[0], "read") == 0)
    loadRecords(command[1]);
  /* file-dependent commands */
  else if (loadedFile.isLoaded != 0) {
    if (strcmp(command[0], "write") == 0)
      writeRecords(command[1]);
    else if (strcmp(command[0], "print") == 0)
      printRecords();
    else if (strcmp(command[0], "delete") == 0)
      deleteRecord(atoi(command[1]));
  }
}

int main() {
  /* note to jalbert1: 2 must match max number of options */
  const int NUMOPTS = 2;
  char ** parsedCommand = alloc2D(NUMOPTS, MAXLEN);
  printf("> ");
  while (parseCommand(parsedCommand, "quit") != 0) {
    handleCommand(parsedCommand);
    printf("> ");
    dealloc2D(parsedCommand, NUMOPTS);
    parsedCommand = alloc2D(NUMOPTS, MAXLEN);
  }
  dealloc2D(parsedCommand, NUMOPTS);
  clearRecords(&head);
  return 0;
}

char ** alloc2D(int x, int y) {
  /*
    alloc2D(
      @x: number of elements of first index
      @y: number of elements of second index
    )

    allocate memory for a 2D array

    returns the 2D array allocated.
  */
  char ** array2D = malloc(sizeof(char*)*x);
  int i;
  for (i = 0; i < x; ++i)
    array2D[i] = malloc(sizeof(char)*y);
  return array2D;
}

char *** alloc3D(int x, int y, int z) {
  /*
    alloc3D(
      @x: number of elements of first index
      @y: number of elements of second index
      @z: number of elements of third index
    )

    allocate memory for a 3D array

    returns the 3D array allocated.
  */
  char *** array3D = malloc(sizeof(char**)*x);
  int i;
  for (i = 0; i < x; ++i)
    array3D[i] = alloc2D(y, z);
  return array3D;
}

void dealloc2D(char ** array2D, int x) {
  /*
    dealloc2D(
      @array2D: 2D array to deallocate
      @x: number of elements of first index
    )

    deallocate memory for a 3D array
  */
  int i;
  for (i = 0; i < x; ++i) {
    free(array2D[i]);
    array2D[i] = NULL;
  }
  free(array2D);
  array2D = NULL;
}

void dealloc3D(char *** array3D, int x, int y) {
  /*
    dealloc3D(
      @array3D: 3D array to deallocate
      @x: number of elements of first index
      @y: number of elements of second index
    )

    deallocate memory for a 3D array
  */
  int i;
  for (i = 0; i < x; ++i) {
    dealloc2D(array3D[i], y);
  }
  free(array3D);
  array3D = NULL;
}

int parseCommand(char ** parsedCommand, char * failOn) {
  /* parseCommand (
      @parsedCommand: array to be populated with parsed contents\
    )

    parses a command string expecting the format `%s [%s...%s]`

    returns 1 if command was successfully parsed, 0 otherwise.
  */

  // get user input
  char command[MAXLEN];
  fgets(command, MAXLEN, stdin);
  fflush(stdin);
  int diff = 1;
  // split the user input
  int segments = 0;
  char * pch = malloc(sizeof(char)*MAXLEN);
  for (pch = strtok(command, "%[ \t]"); pch != NULL; pch = strtok(NULL, "%[ \t]")) {
    if (pch != NULL) {
		if (segments + 1 == 3) {
		  diff = 0;
		  break;
		}
      strcpy(parsedCommand[segments++], pch);
	  if (segments == 2) {
		  continue;
	  }
	}
  }
  pch = NULL;
  // get rid of natural newline char at end of command
  parsedCommand[segments-1][strlen(parsedCommand[segments-1])-diff] = '\0';
  // quit on specified command
  if (strcmp(parsedCommand[0], failOn) == 0)
    return 0;
  return 1;
}

FILE * openFileSafely(char * filename, char * permissions) {
  /*
    openFileSafely(
      @filename: name of file to open
      @permissions: permissions with which to open file
    )

    opens a file avoiding a segfault in the event of unsuccessful open

    returns pointer to opened file
  */
  FILE * lab1Data = fopen(filename, permissions);
  if (lab1Data == NULL)
    //printf("warning: %s\n", errno == 2 ? "file doesn't exist" : "had problems loading file");
  return lab1Data;
}

void closeFileSafely(FILE * filename) {
  /*
    closeFileSafely(
      @filename: name of file to open
    )

    closes a file avoiding a segfault in the event of unsuccessful close
  */
  if (filename != NULL)
    fclose(filename);
  //else printf("warning: file handle was null\n");
}

int slurp(char * filename, callback cb) {
  /*
    slurp(
      @filename: name of file to slurp
      @cb: callback function to call after slurp. Use this to perform
           custom operations of slurped data.
    )

    opens a file, passes file data to the specified callback, and closes the file.

    returns 1 for successful slurp, 0 otherwise.
  */
  FILE * lab1Data = openFileSafely(filename, "r+");
  if (lab1Data == NULL) {
    //printf("error: can't open %s because %s\n", filename, errno == 2 ? "it doesn't exist" : "unknown reasons");
    return 0;
  }
  cb(lab1Data, filename);
  closeFileSafely(lab1Data);
  return 1;
}

void spurt(char * filename, char lines[MAXBODYLEN]) {
  /*
    spurt(
      @filename: name of file to slurp
      @lines: cstring with which to overwrite file
    )

    opens a file, overwrites file with @lines, and closes the file.
  */
  FILE * fileToWrite = openFileSafely(filename, "w+");
  fwrite(lines, strlen(lines), 1, fileToWrite);
  closeFileSafely(fileToWrite);
}

int parse(char * lines, char *** parsed, int spaceThreshold) {
  /*
    parse(
      @lines: slurped file contents to parse
      @parsed: data structure to populate with parsed contents
      @spaceThreshold: number of spaces to signify column break
    )
    Parse a file based on number of spaces between columns.

    returns number of lines parsed.
  */
  int i = 0, ri = 0, ci = 0, li = 0, chars = 0;
  char c;
  for (c = lines[i]; c != '\0'; c = lines[++i]) {
    if (c == '\n') {
      parsed[ri][ci][li++] = '\0';
      ++ri;
      ci = li = 0;
    }
    else if (c == '\t') {
      if (li != 0) {
        parsed[ri][ci][li++] = '\0';
        li = 0;
        ++ci, ++chars;
      }
    }
    else {
      parsed[ri][ci][li++] = c;
      ++chars;
    }
    if (parsed[ri][ci][li-1] == '\n')
      parsed[ri][ci][li-1] = '\b';
  }
  return ri;
}

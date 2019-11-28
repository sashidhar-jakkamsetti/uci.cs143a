// nShell (nsh). A Shell immitating xv6 Shell.

// I included all three extra credits in the same file, 
// labeling wherever they are implemented.
// So all the folders will have this same file.

#include "types.h"
#include "user.h"
#include "fcntl.h"

// Supported commands and their enum representation.
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10
#define MAXCMDS 100


// Definitions

// struct cmd carries all the command related properties to leverage them when calling 
// the system calls.
struct cmd 
{
  int type;
  char *argv[MAXARGS];
  struct cmd *subcmd;
  char *f;
  int mode;
  int fd;
  struct cmd *left;
  struct cmd *right;
};

// I am maintaining a global array of commands inputted to nsh.
// For the same, I need to have a global command counter to make sure I am not overwriting on 
// previously filled slot.
// I assumed that max number of commands issued will be less than MAXCMDS (= 100)
int cmdcounter = 0;
struct cmd cmdmem[MAXCMDS];

// Cmd memory allocator. 
struct cmd *allocate_memory()
{
  struct cmd *mem = &cmdmem[cmdcounter];
  cmdcounter++;
  return mem;
};


// This is basically a universal constructor for any type of command.
struct cmd *prepcmd(int type, struct cmd *subcmd, 
    char *f, int mode, int fd, struct cmd *left, struct cmd *right)
{
  struct cmd *cmd = allocate_memory();
  cmd->type = type;
  memset(cmd->argv, 0, MAXARGS * sizeof(char *));
  cmd->f = f;
  cmd->mode = mode;
  cmd->fd = fd;
  cmd->subcmd = subcmd;
  cmd->left = left;
  cmd->right = right;
  return cmd;
}


// Parser

// Recursive descent parser to understand and convert shell commands into control flow graph.
// Lines 63 - 132 is the command scanner.
// Lines 133 - 267 is the recursive descent parser.
// cmdmem, the global variable I declared above is the Control flow graph here,
// because it has internal pointers in itself making it a directed graph.

// This parser looks very similar to xv6 one, 
// but it is a pruned and modified to reduce code duplication and increase modularity.

char space[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

// Error reporter.
void panic(char *s)
{
  printf(2, "error: %s\n", s);
  exit();
}

// Just consume space!
void eatspace(char **ps, char *es) 
{
  char *s = *ps;
  while(s < es && strchr(space, *s))
    s++;
  *ps = s;
}

// Gets the next token.
int gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  eatspace(ps, es);
  s = *ps;

  if(q)
    *q = s;
  ret = *s;

  switch(*s){
  case 0:
    break;
  // anything from symbols[] = "<|>&;()" should be just eaten, 
  // because we already used that information by calling istoken().
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
  case '>':
    s++;
    break;
  
  // arguments we are looking for.
  default:
    ret = 'a';
    while(s < es && !strchr(space, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  *ps = s;
  eatspace(ps, es);
  
  return ret;
}

// To check whats the next token is.
int istoken(char **ps, char *es, char *toks)
{
  eatspace(ps, es);
  return **ps && strchr(toks, **ps);
}

struct cmd *parseexec(char**, char*);

// Parser for pipe commands. Similar to xv6 one.
struct cmd *parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(istoken(ps, es, "|"))
  {
    gettoken(ps, es, 0, 0);
    cmd = prepcmd(PIPE, 0, 0, 0, 0, cmd, parsepipe(ps, es));
  }
  return cmd;
}

// Parser for redirection commands. Similar to xv6 one.
struct cmd* parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(istoken(ps, es, "<>"))
  {
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    
    *eq = 0;
    switch(tok)
    {
    case '<':
      cmd = prepcmd(REDIR, cmd, q, O_RDONLY, 0, 0, 0);
      break;
    case '>':
      cmd = prepcmd(REDIR, cmd, q, O_WRONLY|O_CREATE, 1, 0, 0);
      break;
    }
  }
  return cmd;
}

// EXTRA CREDIT 1 & 3:
// The high level command parser which understands background and list commands as edge cases.
// Similar to xv6 one.
struct cmd* parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);

  // EXTRA CREDIT 3
  while(istoken(ps, es, "&"))
  {
    gettoken(ps, es, 0, 0);
    cmd = prepcmd(BACK, cmd, 0, 0, 0, 0, 0);
  }

  // EXTRA CREDIT 1
  if(istoken(ps, es, ";"))
  {
    gettoken(ps, es, 0, 0);
    cmd = prepcmd(LIST, 0, 0, 0, 0, cmd, parseline(ps, es));
  }
  return cmd;
}

// EXTRA CREDIT 2: Parser for bracketed commands. Similar to parseblock of xv6. 
struct cmd* parsebracketsblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!istoken(ps, es, "("))
    panic("parsebracketsblock");
  
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);

  if(!istoken(ps, es, ")"))
    panic("syntax - missing )");
  
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

// The actual command and its argument parser. Similar to xv6 one.
struct cmd* parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct cmd *cmd;

  // EXTRA CREDIT 2
  if(istoken(ps, es, "("))
    return parsebracketsblock(ps, es);

  cmd = prepcmd(EXEC, 0, 0, 0, 0, 0, 0);

  argc = 0;
  cmd = parseredirs(cmd, ps, es);

  while(!istoken(ps, es, "|)&;"))
  {
    if((tok = gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    
    *eq = 0; // null termination to show the system call that argument ends here. 
    cmd->argv[argc] = q;
    argc++;

    if(argc >= MAXARGS)
      panic("too many args");
    
    cmd = parseredirs(cmd, ps, es);
  }

  cmd->argv[argc] = 0;
  return cmd;
}

// The enter point for the parser.
struct cmd* parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  eatspace(&s, es);
  if(s != es)
  {
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  return cmd;
}


// Command execution.

// fork wrapper.
int fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

// CFG is generated and stored in *cmd passed as an argument to runcmd.
// This is the main part of the shell which executes the commands.
// Almost same as xv6 one.
void runcmd(struct cmd *cmd)
{
  int p[2];

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    if(cmd->argv[0] == 0)
      exit();
    exec(cmd->argv[0], cmd->argv);
    printf(2, "exec %s failed\n", cmd->argv[0]);
    break;

  case REDIR:
    close(cmd->fd);
    if(open(cmd->f, cmd->mode) < 0){
      printf(2, "open %s failed\n", cmd->f);
      exit();
    }
    runcmd(cmd->subcmd);
    break;

  case PIPE:
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(cmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(cmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  // EXTRA CREDIT 1
  case LIST:
    if(fork1() == 0)
      runcmd(cmd->left);
    wait();
    runcmd(cmd->right);
    break;

  // EXTRA CREDIT 3
  case BACK:
    if(fork1() == 0)
      runcmd(cmd->subcmd);
    break;
  }
  exit();
}

// IO reader of the shell. Waits for the input and stores it in the data (BSS) section.
int getcmd(char *buf, int nbuf)
{
  printf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

// Driver for the shell. Same as xv6 one.
int main(void)
{
  static char buf[100];
  int fd;

  // fd = 0, 1, 2 are standard IO ports for the console, 
  // rest all are closed at the start of shell.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Initialize the counter and zero out the global command memory allocator.
  cmdcounter = 0;
  memset(cmdmem, 0, sizeof(struct cmd) * MAXCMDS);

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0)
  {
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ')
    {
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;
      if(chdir(buf+3) < 0)
        printf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0) 
    {
      // cmd here stores the CFG of the command issued.
      struct cmd *cmd = parsecmd(buf);
      runcmd(cmd);
    }
      
    wait();
  }
  exit();
}
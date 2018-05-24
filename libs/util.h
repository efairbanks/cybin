#ifndef UTIL_H
#define UTIL_H

#define DEBUG_MODE 0
#define DEBUG(...) if(DEBUG_MODE){fprintf(stderr,"Debug: ");fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n");}
#define ERROR(...) fprintf(stderr,"Error: ");fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n");
#define FATAL(...) fprintf(stderr,"Fatal: ");fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n");exit(1);

#endif

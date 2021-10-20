//
//  macros.h
//  myfs
//
//  Created by Oliver Waldhorst on 09.10.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef macros_h
#define macros_h

#define error(str)                \
do {                        \
fprintf(stderr, str "\n");\
exit(-1);\
} while(0)

#ifdef DEBUG
#define LOGF(fmt, ...) \
do { fprintf(this->logFile, "----" fmt "\n", __VA_ARGS__); } while (0)

#define LOG(text) \
do { fprintf(this->logFile, "\t" text "\n"); } while (0)

#define LOGE(text) \
do { fprintf(this->logFile, "ERROR:" text "\n"); } while (0)

#else
#define LOGF(fmt, ...)
#define LOG(text)
#define LOGE(text)
#endif

#ifdef DEBUG_METHODS
#define LOGM() \
do { fprintf(this->logFile, "Function:%s()\nFile:%s:%d\n",__func__, __FILE__, \
__LINE__); } while (0)
#else
#define LOGM()
#endif

#ifdef DEBUG_RETURN_VALUES
#define RETURN(ret) \
do {if (ret < 0) {      \
    switch(ret) {   \
        case -2:  fprintf(this->logFile, "No such file or directory\n"); break; \
        case -22:  fprintf(this->logFile, "Invalid argument\n"); break; \
        case -9:  fprintf(this->logFile, "File not open\n"); break; \
        case -28:  fprintf(this->logFile, "To many files in directory\n"); break; \
        case -18:  fprintf(this->logFile, "File already exists\n"); break;      \
        default: fprintf(this->logFile, "ERROR\n"); break;                \
    }               \
}        \
fprintf(this->logFile, "%s() returned %d\n\n------------------------\n", __func__, ret);} while(0); return ret;
#else
#define RETURN(ret) return ret;
#endif

// TODO: Implement your own macros here!

#endif /* macros_h */

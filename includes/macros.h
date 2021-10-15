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
#else
#define LOGF(fmt, ...)
#define LOG(text)
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
fprintf(this->logFile, "%s() returned %d\n\n------------------------\n", __func__, ret); return ret;
#else
#define RETURN(ret) return ret;
#endif

// TODO: Implement your own macros here!

#endif /* macros_h */

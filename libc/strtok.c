#include <stdlib.h>
#include <string.h>

char *strtok (char s[], char *ct) {
    static char *nextS = NULL;  // Holds modified delimiter.
    static int done = 1;        // Flag to indicate all done.

    // Initial case.
    if (s != NULL) {
        // Return NULL for empty string.
        if (*s == '\0') { done = 1; return NULL; }

        // Find next delimiter.
        nextS = s;
        while ((strchr(ct, *nextS)== NULL) && (*nextS != '\0')) nextS++;
        done = (*nextS == '\0');
        *nextS = '\0';
        //printf("%s\n", s);
        return s;
    }

    // Subsequent cases.
    if (done) return NULL;

    s = ++nextS;
    while ((strchr(ct, *nextS)== NULL) && (*nextS != '\0')) nextS++;
    done = (*nextS == '\0');
    *nextS = '\0';
    return s;
}
